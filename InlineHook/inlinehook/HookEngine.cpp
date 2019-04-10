#include "stdafx.h"
#include "HookEngine.h"

std::function<BOOL(PEXCEPTION_RECORD, CONTEXT *)>m_ptr_hookengine = nullptr;

HookEngine::HookEngine()
	: m_int3_init(false)
{
}


HookEngine::~HookEngine()
{
	m_hook_lock.lock();
	for (auto &item:m_hook_list)
	{
		write_mem(item.hookfunction, item.old_code.data(), item.old_code.size());
		item.old_code.clear();
		item.old_code.resize(1);
	}
	m_hook_lock.unlock();
	m_hook_list.clear();
	m_int3_handler_map.clear();
}


bool HookEngine::hook_function(PVOID function, PVOID new_function, PVOID * old_function, hook_type type)
{
	if (INT3_HOOK==type)
	{
		if (!m_int3_init)
		{
			init_int3_hook();
		}
		if (!m_int3_init)
		{
			return false;
		}
	}
	{
		hook_record hk_rcd;
		auto jmp_size = get_jmp_size(new_function, function);
		if (type==JMP_REG_HOOK)
		{
			jmp_size = get_jmp_reg_size(function, new_function);
		}
		if (type==INT3_HOOK)
		{
			jmp_size = 1;
		}
		printf("jmp_size = %d\r\n", jmp_size);
#ifdef _X86_
		auto old_code = VirtualAlloc(NULL, jmp_size * 3+0x10, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		auto exitp = std::experimental::make_scope_exit([&] {VirtualFree(old_code, jmp_size * 3 + 0x10, MEM_RELEASE); });
#else
		auto old_code = alloc_mem(function, jmp_size * 3 + 0x10);//64位hook在临近的位置申请内存方便处理RIP_RELATIVE的OPCODE
		auto exitp = std::experimental::make_scope_exit([&] {VirtualFree(old_code, jmp_size * 3 + 0x10, MEM_RELEASE); });
#endif
		auto target_code = reinterpret_cast<BYTE*>(old_code);


		_CodeInfo ci = { 0 };
		ci.code = reinterpret_cast<uint8_t*>(function);
		ci.codeOffset = (_OffsetType)function;
		ci.codeLen = jmp_size * 3+0x10;
#ifdef _X86_
		ci.dt = Decode32Bits;
#else
		ci.dt = Decode64Bits;
#endif
		ci.features = DF_NONE;
		unsigned int InstrSize = 0;
		do
		{
			_DInst dec[1];
			UINT decCnt = 0;
			distorm_decompose(&ci, dec, 1, &decCnt);
			if (dec->flags == FLAG_NOT_DECODABLE)
			{
				return false;
			}
			auto fc = META_GET_FC(dec->meta);
			if (fc == FC_UNC_BRANCH || fc == FC_CND_BRANCH || fc == FC_CALL || fc == FC_INT || fc == FC_RET || fc == FC_SYS)
			{
				//有改变RIP的指令，拒绝继续xx！
				//TODO::处理各类jcc/call的指令迁移
				return false;
			}
			{
				//non branching instruction
				memcpy(target_code, (void*)ci.codeOffset, dec->size);
				target_code += dec->size;
				printf("size = %d\r\n", dec->size);
			}
#ifdef _WIN64

			if (dec->flags & FLAG_RIP_RELATIVE)
			{
				auto p_targ = INSTRUCTION_GET_RIP_TARGET(dec);
				_DecodedInst inst;
				distorm_format(&ci, dec, &inst);
				printf("%s %s %s\r\n", inst.instructionHex.p, inst.mnemonic.p, inst.operands.p);
				printf("opxx = %p\r\n", PVOID(p_targ));
				ptrdiff_t diff;
				unsigned int immSize = 0;
				diff = ((ptrdiff_t)p_targ - (ptrdiff_t)(target_code-dec->size));
				for (auto i = 0; i < OPERANDS_NO; i++) {
					if (dec->ops[i].type == O_IMM) {
						immSize = dec->ops[i].size / 8;
						break;
					}
				}
				auto fix_code = target_code - immSize - 4;
				*(unsigned int*)fix_code = (unsigned int)diff;
				//return false;
			}
#endif	
			ci.codeOffset = ci.nextOffset;
			ci.code += dec->size;
			InstrSize += dec->size;
		} while (InstrSize < jmp_size);

		int OverridenSize = InstrSize;
		printf("backup size = %d\r\n", InstrSize);
		//Jump back to source
		{
		 auto size = make_jmp_code(PVOID((BYTE*)function + InstrSize), target_code);
		}
		*old_function = old_code;

		hk_rcd.hookfunction = function;
		hk_rcd.old_code.resize(OverridenSize);
		DWORD dwOld = 0;
		VirtualProtect(function, OverridenSize, PAGE_EXECUTE_READWRITE, &dwOld);
		auto a1 = std::experimental::make_scope_exit([&] {VirtualProtect(function, OverridenSize, dwOld, &dwOld); });
		RtlCopyMemory(hk_rcd.old_code.data(), function, OverridenSize);
		if (type == JMP_HOOK)
		{
			make_jmp_code(new_function, function);
		}
		if (type==JMP_REG_HOOK)
		{
			make_jmp_reg(new_function, function);
		}
		if (type==INT3_HOOK)
		{
			//写int3?
			m_int3_handler_map[function] = new_function;
			std::vector<BYTE> code(1);
			code[0] = 0xCC;
			write_mem(function, code.data(), 1);
		}
		m_hook_lock.lock();
		m_hook_list.push_back(hk_rcd);
		m_hook_lock.unlock();

		exitp.release();
		return true;
	}
	return false;
}


bool HookEngine::unhook_function(PVOID function)
{
	m_hook_lock.lock();
	auto exit = std::experimental::make_scope_exit([&] {m_hook_lock.unlock(); });
	auto p = std::find_if(m_hook_list.begin(), m_hook_list.end(), [&](auto item) { return item.hookfunction == function; });
	if (p!=m_hook_list.end())
	{
		auto item = *p;
		write_mem(item.hookfunction, item.old_code.data(), item.old_code.size());
		item.old_code.clear();
		item.old_code.resize(1);
		m_hook_list.erase(p);
		return true;
	}
	return false;
}


void HookEngine::write_mem(PVOID mem, PVOID buff, SIZE_T buff_size)
{
	DWORD dwOld = 0;
	VirtualProtect(mem, buff_size, PAGE_EXECUTE_READWRITE, &dwOld);
	RtlCopyMemory(mem, buff, buff_size);
	VirtualProtect(mem, buff_size, dwOld, &dwOld);
}


size_t HookEngine::make_jmp_code(PVOID target, PVOID addr)
{
	printf("%p\r\n", target);
	asmjit::JitRuntime runtime;
	asmjit::X86Assembler jit(&runtime);
	jit.jmp(asmjit::Ptr(target));
	DWORD_PTR const stubSize = jit.getCodeSize();
	std::vector<BYTE> code(stubSize * 3);
	auto real_size = jit.relocCode(code.data(), asmjit::Ptr(addr));
	std::for_each(code.begin(), code.begin()+real_size, [&](auto p) { printf("%x ", p); });
	write_mem(addr, code.data(), real_size);
	return real_size;
}


size_t HookEngine::get_jmp_size(PVOID target, PVOID addr)
{
	asmjit::JitRuntime runtime;
	asmjit::X86Assembler jit(&runtime);
	jit.jmp(asmjit::Ptr(target));
	DWORD_PTR const stubSize = jit.getCodeSize();
	std::vector<BYTE> code(stubSize * 3);
	auto real_size = jit.relocCode(code.data(), asmjit::Ptr(addr));
	//write_mem(addr, code.data(), real_size);
	return real_size;
}


size_t HookEngine::get_jmp_reg_size(PVOID addr, PVOID target)
{
	asmjit::JitRuntime runtime;
	asmjit::X86Assembler jit(&runtime);
#ifdef _WIN64
	jit.mov(asmjit::host::rax, asmjit::Ptr(target));
	jit.jmp(asmjit::host::rax);
#else
	jit.mov(asmjit::host::eax, asmjit::Ptr(target));
	jit.jmp(asmjit::host::eax);
#endif
	auto code_size = jit.getCodeSize();
	std::vector<BYTE> c_code(code_size * 3);
	auto real_size = jit.relocCode(c_code.data(), asmjit::Ptr(addr));
	return real_size;
}


void HookEngine::make_jmp_reg(PVOID target, PVOID addr)
{
	asmjit::JitRuntime runtime;
	asmjit::X86Assembler jit(&runtime);
#ifdef _WIN64
	jit.mov(asmjit::host::rax, asmjit::Ptr(target));
	jit.jmp(asmjit::host::rax);
#else
	jit.mov(asmjit::host::eax, asmjit::Ptr(target));
	jit.jmp(asmjit::host::eax);
#endif
	auto code_size = jit.getCodeSize();
	std::vector<BYTE> c_code(code_size * 3);
	auto real_size = jit.relocCode(c_code.data(), asmjit::Ptr(addr));
	write_mem(addr, c_code.data(), real_size);
}

using pfnRtlDispatchException = ULONG(WINAPI *)(PEXCEPTION_RECORD pExcptRec, CONTEXT * pContext);
pfnRtlDispatchException m_fnRtlDispatchException = nullptr;
ULONG WINAPI _RtlDispatchException(PEXCEPTION_RECORD pExcptRec, CONTEXT * pContext)
{
	if (m_ptr_hookengine(pExcptRec, pContext)) return 1;
	return m_fnRtlDispatchException(pExcptRec, pContext);
}
DWORD NTAPI ExceptionHandler(EXCEPTION_POINTERS * exceptioninfo) 
{
	//异常处理函数
	if (m_ptr_hookengine(exceptioninfo->ExceptionRecord,exceptioninfo->ContextRecord))
	{
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	return EXCEPTION_CONTINUE_SEARCH;
}


void HookEngine::init_int3_hook()
{
	if (m_ptr_hookengine)
	{
		return;
	}
	m_ptr_hookengine = std::bind(&HookEngine::Handler_RtlDispatch, this,std::placeholders::_1,std::placeholders::_2);
//#ifdef _X86_
//	BYTE *pAddr = (BYTE *)::GetProcAddress(::GetModuleHandleA("ntdll"), "KiUserExceptionDispatcher");
//	if (pAddr)
//	{
//		while (*pAddr != 0xE8)pAddr++;  
//		m_fnRtlDispatchException = (pfnRtlDispatchException)((*(DWORD *)(pAddr + 1)) + 5 + (DWORD)pAddr);  //得到原函数地址
//		DWORD dwNewAddr = (DWORD)_RtlDispatchException - (DWORD)pAddr - 5;
//		DWORD dwOld;
//		VirtualProtect((LPVOID)pAddr, 0x1000, PAGE_EXECUTE_READWRITE, &dwOld);
//		RtlCopyMemory((PVOID)((DWORD)pAddr + 1), (PVOID)&dwNewAddr, 4);  //这个写内存的自己改造吧
//		m_int3_init = true;
//	}
//#else
	//注册VEH
	AddVectoredExceptionHandler(1, (PVECTORED_EXCEPTION_HANDLER)ExceptionHandler);
	m_int3_init = true;
//#endif
}


BOOL HookEngine::Handler_RtlDispatch(PEXCEPTION_RECORD pExcptRec, CONTEXT * pContext)
{
#ifdef _X86_
	auto dwEip = pContext->Eip;
#else
	auto dwEip = pContext->Rip;
#endif
	auto p = this->m_int3_handler_map.find(PVOID(dwEip));
	if (p!=this->m_int3_handler_map.end())
	{
		printf("what\r\n");
#ifdef _X86_
		pContext->Eip = decltype(pContext->Eip)(m_int3_handler_map[PVOID(dwEip)]);
#else
		pContext->Rip = decltype(pContext->Rip)(m_int3_handler_map[PVOID(dwEip)]);
#endif
		return TRUE;
	}
	return FALSE;
}


PVOID HookEngine::alloc_mem(PVOID wanna_address, size_t alloc_size)
{
	auto AllocateSize = alloc_size;

	if ((ULONG_PTR)wanna_address >= 0x70000000 && (ULONG_PTR)wanna_address < 0x80000000)
		wanna_address = (PVOID)0x70000000;

	while (1)
	{
		MEMORY_BASIC_INFORMATION mbi = { 0 };
		if (!VirtualQuery(wanna_address, &mbi, sizeof(mbi)))
			break;
		if (wanna_address != mbi.AllocationBase)
		{
			wanna_address = mbi.AllocationBase;
		}
		else
		{
			wanna_address = (PVOID)((ULONG_PTR)mbi.AllocationBase - 0x1000);
		}

		if (mbi.State == MEM_FREE)
		{
			auto pmem = VirtualAlloc(mbi.BaseAddress, AllocateSize, MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (pmem)
			{
				auto ret = VirtualAlloc(pmem, AllocateSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
				if (ret)
				{
					printf("%p\r\n", ret);
					return ret;
				}
			}
		}
	}
	return NULL;
}
