#define UNICODE								// ������������� ����������� ���������,
											// ��� ��� ��� ��������� ����� �������������� ANSI 
#include <stdbool.h>
#include <windows.h>

#define PROCNAME_LENGTH 64
#define CACHEINFO_SIZE  64

static LPCWSTR szTitle = L"System information";		// ��������� ���� ����������

static TCHAR computerNameOutput[PROCNAME_LENGTH];	// ������ ��� ������ ����� ����������
static TCHAR L1dOutput[CACHEINFO_SIZE];				// ������ ��� ������ L1d-����

static DWORD WINAPI ThreadFunc(void* arg) {
	typedef bool (*ImportFunction)(char*, int*);			// ����������� ���� ��������� �� �������
	HINSTANCE hinstLib = LoadLibrary(TEXT("info.dll"));		// ��������� ��������� �� .dll
	ImportFunction DLLInfo = (ImportFunction)GetProcAddress(hinstLib, "Information");	// �������� ������� Information

	char pc_name[PROCNAME_LENGTH];			// ������� �������� ��� ��������� ����� ��
	int cache_size = 0;						// ������� �������� ��� ��������� ������� L1d-����

	bool success_flag = DLLInfo(pc_name, &cache_size);						// ����� ������� �� ����������� ����������

	wsprintf(computerNameOutput, L"Computer name: %ls", (TCHAR*)pc_name);	// ��������� ������ � ������ �� ��� ������
	if (success_flag)
		wsprintf(L1dOutput, L"Size of L1 data cache per core: %i Kbyte", cache_size);	// ��������� ������ � ����� ��� ������
	else
		wsprintf(L1dOutput, L"Your processor is not currently supported.");				// ��������� ���������, ���� ��� �� ������

	FreeLibrary(hinstLib);		// �������� ����������

	return 1;
}

static LRESULT CALLBACK WindowFunc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	HFONT hFont = CreateFont(20, 0, 0, 0,
		FW_REGULAR, FALSE,
		FALSE, FALSE,
		DEFAULT_CHARSET,
		OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY,
		VARIABLE_PITCH,
		TEXT("Consolas"));			// �������� ����������� ������

	switch (msg) {					// ��������� ���������
		case WM_CREATE: {			// �������� ������, ���������, �������� ����������� ������
			HANDLE hThread = CreateThread(NULL, 0, ThreadFunc, NULL, 0, NULL);
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
			break;
		}
		case WM_PAINT: {			// ����������� ���� ����������
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint(hWnd, &ps);
			SelectObject(hDC, hFont);
			SetTextColor(hDC, RGB(0, 10, 10));
			TextOut(hDC, 15, 17, computerNameOutput, PROCNAME_LENGTH);
			TextOut(hDC, 15, 42, L1dOutput, 50);
			EndPaint(hWnd, &ps);
			break;
		}
		case WM_DESTROY: {			// ���������� ���� ����������
			PostQuitMessage(0);
			break;
		}
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR str, int nWinMode) {
	LPCWSTR szClassName = L"SysInfoFinder";		// �������� ������ ���� ����������

	WNDCLASS wcl;								// ���������� ����
	wcl.hInstance = hThisInst;
	wcl.lpszClassName = szClassName;
	wcl.lpfnWndProc = WindowFunc;
	wcl.style = CS_HREDRAW | CS_VREDRAW;
	wcl.hIcon = LoadIcon(NULL, IDI_ASTERISK);
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.lpszMenuName = NULL;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hbrBackground = (HBRUSH)GetStockObject(DEFAULT_PALETTE);
	RegisterClass(&wcl);

	HWND hWnd = CreateWindow(szClassName, szTitle,
							 WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU,
							 720, 480, 426, 130,
							 HWND_DESKTOP, NULL,
							 hThisInst, NULL);		// �������� ����

	ShowWindow(hWnd, nWinMode);		// ����� ����
	UpdateWindow(hWnd);				// ���������� ����

	MSG msg;								// ���������� � ���� ���������
	while (GetMessage(&msg, NULL, 0, 0)) {	// ��������� ���������
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}
