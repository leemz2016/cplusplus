// TestAmsi.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "amsi.h"
#include <windows.h>
#include <tchar.h>
#pragma comment(lib, "amsi.lib")
#pragma comment(lib, "ole32.lib")
using namespace std;

#define EICAR "X5O!P%@AP[4\\PZX54(P^)7CC)7}$EICAR-STANDARD-ANTIVIRUS-TEST-FILE!$H+H*" //av测试样本字符串
#define AMSIPROJECTNAME "TestAmsi"
#define AMSIDLL "amsi.dll"

struct ScanResult {
	HRESULT RiskLevel;
	BOOL IsMalware;
};

struct Sample {
	BYTE* data;
	ULONG size;
};

class AmsiUtils {
public:
	LPSTR static GetErrorReason(DWORD errCode) {
		LPSTR reason = nullptr;
		FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			errCode,
			0,
			(LPSTR)&reason,
			0,
			NULL
		);
		return reason;
	}

	void static GetSampleFile(LPCTSTR fname, struct Sample* sample) {
		HANDLE hFile = CreateFileA(fname, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			throw std::runtime_error("Invalid file handle");
		}

		DWORD dwFileSize = GetFileSize(hFile, NULL);
		if (dwFileSize == INVALID_FILE_SIZE || dwFileSize == 0) {
			throw std::runtime_error("Failed to get the file size");
		}

		BYTE* buffer = (BYTE*)VirtualAlloc(NULL, dwFileSize, MEM_COMMIT, PAGE_READWRITE);
		if (!buffer) {
			throw std::runtime_error("Failed to allocate memory for file");
		}

		DWORD dwBytesRead;
		if (!ReadFile(hFile, buffer, dwFileSize, &dwBytesRead, NULL)) {
			throw std::runtime_error("Failed to read file");
		}

		CloseHandle(hFile);

		sample->data = (BYTE*)buffer;
		sample->size = dwFileSize;
	}

	LPSTR static GetResultDescription(HRESULT score) {
		 LPSTR description;
		switch (score) {
		case AMSI_RESULT_CLEAN:
			description = (LPSTR)"File is clean";
			break;
		case AMSI_RESULT_NOT_DETECTED:
			description = (LPSTR)"No threat detected";
			break;
		case AMSI_RESULT_BLOCKED_BY_ADMIN_START:
			description = (LPSTR)"Threat is blocked by the administrator";
			break;
		case AMSI_RESULT_BLOCKED_BY_ADMIN_END:
			description = (LPSTR)"Threat is blocked by the administrator";
			break;
		case AMSI_RESULT_DETECTED:
			description = (LPSTR)"File is considered malware";
			break;
		default:
			description = (LPSTR)"N/A";
			break;
		}

		return description;
	}
};

class AmsiScanner {
public:
	AmsiScanner() {
		HRESULT hResult = CoInitializeEx(0, COINIT_MULTITHREADED);
		if (hResult != S_OK) {
			throw std::runtime_error("COM library failed to initialize");
		}
	}

	~AmsiScanner() {
		CoUninitialize();
	}

	HRESULT Scan(LPCTSTR fname, BYTE* sample, ULONG sampleSize, struct ScanResult* scanResult) {
		HRESULT hResult = S_OK;
		HAMSICONTEXT amsiContext;
		AMSI_RESULT amsiRes = AMSI_RESULT_DETECTED;
		HAMSISESSION session = nullptr;

		ZeroMemory(&amsiContext, sizeof(amsiContext));

		hResult = AmsiInitialize((LPCWSTR)AMSIPROJECTNAME, &amsiContext);
		if (hResult != S_OK) {
			OutputDebugString("AmsiInitialize failed");
			return hResult;
		}

		hResult = AmsiOpenSession(amsiContext, &session);
		if (hResult != S_OK || session == nullptr) {
			OutputDebugString("AmsiOpenSession failed");
			return hResult;
		}

	
		hResult = AmsiScanBuffer(amsiContext, sample, sampleSize, (LPCWSTR)fname, session, &amsiRes);
		if (hResult != S_OK) {
			OutputDebugString("AmsiScannerBuffer failed");
			cerr << "AmsiScanBuffer failed. Did you disable something for Windows Defender?" << endl;;
			return hResult;
		}

	
		scanResult->IsMalware = AmsiResultIsMalware(amsiRes);

		AmsiUninitialize(amsiContext);
		CoUninitialize();

		return S_OK;
	}

	static void Start(LPCTSTR fname = NULL) {
		AmsiScanner* scanner = new AmsiScanner();
		struct ScanResult scanResult;
		HRESULT hResult;
		struct Sample sample;

		if (fname) {
			AmsiUtils::GetSampleFile(fname, &sample);
		}
		else {
			fname = "EICAR";
			sample.data = (BYTE*)EICAR;
			sample.size = strlen(EICAR);
		}

		hResult = scanner->Scan(fname, sample.data, sample.size, &scanResult);
		cout << "Sample size: " << sample.size << " bytes" << endl;
		if (hResult == S_OK) {
			if (scanResult.IsMalware) {
				if (!fname) { fname = "EICAR Sample"; }
				cout << "Malware detected: " << fname << endl;
			}
			cout << "Risk level = " << scanResult.RiskLevel << " (" << AmsiUtils::GetResultDescription(scanResult.RiskLevel) << ")" << endl;
		}
		else {
			LPSTR errReason = AmsiUtils::GetErrorReason(hResult);
			printf("Failed to scan with error code 0x%x. Reason: %s", hResult, errReason);
		}

		delete(scanner);
	}
};

int main(int args, char** argv[]) {
	if (args != 2) {
		cout << "No sample provided, EICAR string will be used for testing" << endl;
	}

	try {
		AmsiScanner::Start((LPCTSTR)argv[1]);
	}
	catch (std::runtime_error& ex) {
		cerr << ex.what() << endl;
	}
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
