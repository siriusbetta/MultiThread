#include <windows.h>

#include <string>
#include <tchar.h>

#define THREADS_NUMBER 10
#define ITERATIONS_NUMBER 100
#define PAUSE 10 /* ms */

#define ID_START_BUTTON 1000

HINSTANCE hThisInstance;
TCHAR WinName[] = _T("WinPort");
TCHAR WinHead[] = _T("WinComPort terminal");
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

DWORD dwCounter = 0;

DWORD WINAPI ThreadProc(CONST LPVOID lpParam) {
  CONST HANDLE hMutex = (CONST HANDLE)lpParam;
  DWORD i;
  for(i = 0; i < ITERATIONS_NUMBER; i++) {
    WaitForSingleObject(hMutex, INFINITE);
    dwCounter++;
    ReleaseMutex(hMutex);
    Sleep(PAUSE);
  }
  ExitThread(0);
}

VOID Error(CONST HANDLE hStdOut, CONST LPCSTR szMessage) {
  DWORD dwTemp;
  TCHAR szError[256];
  MessageBox(NULL, szMessage, "Error", MB_OK);
  ExitProcess(0);
}

int APIENTRY _tWinMain(HINSTANCE hThisInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR cmd,
					   int mode)
{
	WNDCLASS wc;
	MSG msg;
	HWND hWnd;

	wc.hInstance		= hThisInstance;
	wc.lpszClassName	= WinName;
	wc.lpfnWndProc		= WndProc;
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.hCursor			= LoadCursor(NULL, IDI_APPLICATION);
	wc.hIcon			= LoadIcon(NULL, IDC_ARROW);
	wc.lpszMenuName		= NULL;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hbrBackground	= (HBRUSH) (COLOR_WINDOW + 1);

	if(!RegisterClass(&wc)) return 0;

	hWnd = CreateWindow(WinName,
						WinHead,
						WS_OVERLAPPEDWINDOW,
						100,
						50,
						500,
						350,
						HWND_DESKTOP,
						NULL,
						hThisInstance,
						NULL
						);
	ShowWindow(hWnd, mode);

	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hStartButton;

	switch(message)
	{
	case WM_CREATE:
		{
			CreateWindow("static", "Нажмите кнопки для запуска потоков", WS_CHILD|WS_VISIBLE|SS_CENTER, 
						50,30,300,20,
						hWnd,
						(HMENU)0,
						hThisInstance,
						NULL);

			hStartButton = CreateWindow("button", "Start",
						WS_CHILD|WS_VISIBLE|WS_BORDER|ES_LEFT,
						50, 60, 50, 20,
						hWnd,
						(HMENU) ID_START_BUTTON,
						hThisInstance,
						NULL);
			return 0;
		}
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case ID_START_BUTTON:
				{
					TCHAR szMessage[256];
					  DWORD dwTemp, i;
					  HANDLE hThreads[THREADS_NUMBER];
					  CONST HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
					  CONST HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);
					  if(NULL == hMutex) {
						Error(hStdOut, TEXT("Failed to create mutex.\r\n"));
					  }

					  for(i = 0; i < THREADS_NUMBER; i++) {
						hThreads[i] = CreateThread(NULL, 0, &ThreadProc, hMutex, 0, NULL);
						if(NULL == hThreads[i]) {
						  Error(hStdOut, TEXT("Failed to create thread.\r\n"));
						}
					  }

					  WaitForMultipleObjects(THREADS_NUMBER, hThreads, TRUE, INFINITE);

					  wsprintf(szMessage, TEXT("Counter = %d\r\n"), dwCounter);
					  MessageBox(NULL, szMessage, "Message", MB_OK);
					  WriteConsole(hStdOut, szMessage, lstrlen(szMessage), &dwTemp, NULL);

					  for(i = 0; i < THREADS_NUMBER; i++) {
						CloseHandle(hThreads[i]);
					  }
					  CloseHandle(hMutex);
					  ExitProcess(0);
									}
								}
								
			return 0;
		}
	case WM_DESTROY:
		{
			//CloseHandle(hWnd);
			PostQuitMessage(0);
			break;
		}
		default: return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
  
}