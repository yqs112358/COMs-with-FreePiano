#include "dll.h"
#include <windows.h>
#include <cstring>

#define WINDOW_TITLE "Wispow Freepiano 2"
#define CLASS_NAME "FreePianoMainWindow"

//Typedef
typedef LRESULT (WINAPI* pfn_WindProc)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
typedef LRESULT (CALLBACK* pfn_LowLevelKeyboardProc)(int nCode, WPARAM wParam, LPARAM lParam);
typedef struct _KeyboardLL_Msg {
	int nCode;
	WPARAM wParam;
	KBDLLHOOKSTRUCT lParam;
} KeyboardLL_Msg, *PKeyboardLL_Msg;

//Global Data
pfn_LowLevelKeyboardProc g_LowLevelKeyboardProc = (pfn_LowLevelKeyboardProc)0x49b60;
pfn_WindProc g_WndProc = NULL;
HWND hPiano;

DllClass::DllClass()
{

}

DllClass::~DllClass()
{

}

HWND FindFreepiano()
{
	HWND hwnd = FindWindow(CLASS_NAME, WINDOW_TITLE);
	if(hwnd == NULL) {
		hwnd =  FindWindow(CLASS_NAME, NULL);
		if(hwnd == NULL) {
			hwnd =  FindWindow(NULL, WINDOW_TITLE);
		}
	}

	return hwnd;
}

LRESULT WINAPI fakeWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if(Msg == WM_COPYDATA) {
		COPYDATASTRUCT* CopyData = (COPYDATASTRUCT*)lParam;
		if(CopyData->cbData == sizeof(KeyboardLL_Msg))
		{
			KeyboardLL_Msg* Msg = (KeyboardLL_Msg*)CopyData->lpData;
			g_LowLevelKeyboardProc(Msg->nCode, Msg->wParam, (LPARAM)&Msg->lParam);
		}
	}
	return g_WndProc(hWnd, Msg, wParam, lParam);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	switch(fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			while(true)
			{
				hPiano=FindFreepiano();
				if(!hPiano)
					Sleep(50);
				g_WndProc = (pfn_WindProc)GetWindowLong(hPiano, GWL_WNDPROC);
				if(g_WndProc)
				{
					SetWindowLong(hPiano, GWL_WNDPROC, (LONG)fakeWindowProc);
					break;
				}
			}
			HMODULE hExe = GetModuleHandle(NULL);
			g_LowLevelKeyboardProc = (pfn_LowLevelKeyboardProc)((DWORD)hExe + (DWORD)g_LowLevelKeyboardProc); 
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			break;
		}
		case DLL_THREAD_ATTACH:
		{
			break;
		}
		case DLL_THREAD_DETACH:
		{
			break;
		}
	}
	
	/* Return TRUE on success, FALSE on failure */
	return TRUE;
}
