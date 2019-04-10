// test_hook.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../inlinehook/HookEngine.h"

using T_MessageBoxA = int (WINAPI*)(
	_In_opt_ HWND hWnd,
	_In_opt_ LPCSTR lpText,
	_In_opt_ LPCSTR lpCaption,
	_In_ UINT uType);
T_MessageBoxA OldMessageBoxA;

int
WINAPI
OnMessageBoxA(
	_In_opt_ HWND hWnd,
	_In_opt_ LPCSTR lpText,
	_In_opt_ LPCSTR lpCaption,
	_In_ UINT uType)
{
	printf("fucker\r\n");
	return OldMessageBoxA(hWnd, "fucker", "123", uType);
}
HookEngine hook;
int main()
{
	auto ptr_function = GetProcAddress(GetModuleHandle(_T("user32.dll")), "MessageBoxA");
	auto bok = hook.hook_function(PVOID(ptr_function), PVOID(OnMessageBoxA), reinterpret_cast<PVOID *>(&OldMessageBoxA),HookEngine::JMP_HOOK);
	printf("\r\nbok == %d\r\n", bok);
	MessageBoxA(NULL, "aaa", "222", MB_OK);
	if (bok)
	{
		hook.unhook_function(PVOID(ptr_function));
		MessageBoxA(NULL, "aaa", "222", MB_OK);
	}
    return 0;
}

