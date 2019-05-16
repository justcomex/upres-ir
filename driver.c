#define UNICODE								// устанавливаем оптимальную кодировку,
											// так как без директивы будет использоватьс€ ANSI 
#include <stdbool.h>
#include <windows.h>

#define PROCNAME_LENGTH 64
#define CACHEINFO_SIZE  64

static LPCWSTR szTitle = L"System information";		// заголовок окна приложени€

static TCHAR computerNameOutput[PROCNAME_LENGTH];	// строка дл€ вывода имени компьютера
static TCHAR L1dOutput[CACHEINFO_SIZE];				// строка дл€ вывода L1d- ЁЎа

static DWORD WINAPI ThreadFunc(void* arg) {
	typedef bool (*ImportFunction)(char*, int*);			// определение типа указател€ на функцию
	HINSTANCE hinstLib = LoadLibrary(TEXT("info.dll"));		// получение указател€ на .dll
	ImportFunction DLLInfo = (ImportFunction)GetProcAddress(hinstLib, "Information");	// загрузка функции Information

	char pc_name[PROCNAME_LENGTH];			// входной параметр дл€ получени€ имени ѕ 
	int cache_size = 0;						// входной параметр дл€ получени€ размера L1d- ЁЎа

	bool success_flag = DLLInfo(pc_name, &cache_size);						// вызов функции из стандартной библиотеки

	wsprintf(computerNameOutput, L"Computer name: %ls", (TCHAR*)pc_name);	// формируем строку с именем ѕ  дл€ вывода
	if (success_flag)
		wsprintf(L1dOutput, L"Size of L1 data cache per core: %i Kbyte", cache_size);	// формируем строку с  ЁЎем дл€ вывода
	else
		wsprintf(L1dOutput, L"Your processor is not currently supported.");				// формируем сообщение, если  ЁЎ не найден

	FreeLibrary(hinstLib);		// закрытие библиотеки

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
		TEXT("Consolas"));			// создание логического шрифта

	switch (msg) {					// обработка сообщений
		case WM_CREATE: {			// создание потока, обработка, удаление дескриптора потока
			HANDLE hThread = CreateThread(NULL, 0, ThreadFunc, NULL, 0, NULL);
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
			break;
		}
		case WM_PAINT: {			// окрашивание окна приложени€
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint(hWnd, &ps);
			SelectObject(hDC, hFont);
			SetTextColor(hDC, RGB(0, 10, 10));
			TextOut(hDC, 15, 17, computerNameOutput, PROCNAME_LENGTH);
			TextOut(hDC, 15, 42, L1dOutput, 50);
			EndPaint(hWnd, &ps);
			break;
		}
		case WM_DESTROY: {			// разрушение окна приложени€
			PostQuitMessage(0);
			break;
		}
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR str, int nWinMode) {
	LPCWSTR szClassName = L"SysInfoFinder";		// название класса окна приложени€

	WNDCLASS wcl;								// дескриптор окна
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
							 hThisInst, NULL);		// создание окна

	ShowWindow(hWnd, nWinMode);		// вывод окна
	UpdateWindow(hWnd);				// обновление окна

	MSG msg;								// информаци€ о всех сообщений
	while (GetMessage(&msg, NULL, 0, 0)) {	// получение сообщений
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}
