#pragma once
#include "../common/common.h"
#include "distorm3.hpp"
#include "../3rd/asmjit/src/asmjit/asmjit.h"
#include <mutex>
class HookEngine
{
public:
	enum hook_type
	{
		JMP_HOOK = 1,
		JMP_REG_HOOK,
		INT3_HOOK,
		DRX_HOOK,//TODO£ºÓ²¶ÏÄ£Ê½
	};
	typedef struct hook_record 
	{
		PVOID hookfunction;
		std::vector<BYTE> old_code;
	}hook_record;
	HookEngine();
	~HookEngine();
	bool hook_function(PVOID function, PVOID new_function, PVOID * old_function,hook_type type=JMP_HOOK);
private:
	std::vector<hook_record> m_hook_list;
	std::mutex m_hook_lock;
	std::map<PVOID, PVOID>m_int3_handler_map;
public:
	bool unhook_function(PVOID function);
private:
	void write_mem(PVOID mem, PVOID buff, SIZE_T buff_size);
	size_t make_jmp_code(PVOID target, PVOID addr);
	size_t get_jmp_size(PVOID target, PVOID addr);
	size_t get_jmp_reg_size(PVOID addr, PVOID target);
	void make_jmp_reg(PVOID target, PVOID addr);
	bool m_int3_init;
	void init_int3_hook();
public:
	BOOL Handler_RtlDispatch(PEXCEPTION_RECORD pExcptRec, CONTEXT * pContext);
	PVOID alloc_mem(PVOID wanna_address, size_t alloc_size);
};

