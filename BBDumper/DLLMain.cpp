#include "../Libs/MinHook/MinHook.h"

#pragma comment(lib, "../Libs/MinHook/libMinHook.x86.lib")

typedef NTSTATUS(NTAPI *NtWow64WriteVirtualMemory64t)
(
	IN  HANDLE   ProcessHandle,
	IN  ULONG64  BaseAddress,
	IN  PVOID    Buffer,
	IN  ULONG64  BufferLength,
	OUT PULONG64 ReturnLength OPTIONAL
);

NtWow64WriteVirtualMemory64t oNtWow64WriteVirtualMemory64 = NULL;
HANDLE hFile;

NTSTATUS NTAPI NtWow64WriteVirtualMemory64_h
(
	IN  HANDLE   ProcessHandle,
	IN  ULONG64  BaseAddress,
	IN  PVOID    Buffer,
	IN  ULONG64  BufferLength,
	OUT PULONG64 ReturnLength OPTIONAL
)
{
	WriteFile(hFile, Buffer, BufferLength, NULL, NULL);

	return oNtWow64WriteVirtualMemory64(ProcessHandle, BaseAddress, Buffer, BufferLength, ReturnLength);
}

void Start()
{
	MH_Initialize();

	hFile = CreateFile
	(
		"Dump.dll",       
		GENERIC_WRITE,       
		0,                   
		NULL,                 
		CREATE_NEW,            
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	auto NtWow64WriteVirtualMemory64 = GetProcAddress(LoadLibrary("ntdll.dll"), "NtWow64WriteVirtualMemory64");

	MH_CreateHook(NtWow64WriteVirtualMemory64, &NtWow64WriteVirtualMemory64_h, reinterpret_cast<LPVOID*>(&oNtWow64WriteVirtualMemory64));

	MH_EnableHook(NtWow64WriteVirtualMemory64);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH) 
	{
		Start();
	}

    return TRUE;
}

