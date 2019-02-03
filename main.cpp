#include <cstdio>
#include <windows.h>
#include <cstring>
//#include <ctime>
//#include "KeyScanCode.h"
using namespace std;

#define BAUD_RATE CBR_115200
#define PARITY NOPARITY
#define BYTE_SIZE 8
#define STOP_BYTE ONESTOPBIT
#define MAXN 256

#define DEFAULT_PORT_NAME "\\\\.\\COM22"
#define WINDOW_TITLE "Wispow Freepiano 2"
#define CLASS_NAME "FreePianoMainWindow"

#define PIANO_PATH ".\\Piano\\freepiano.exe"
#define REMOTE_DLL_PATH "..\\Piano_RemoteDLL.dll"
#define DELAY_AFTER_START 500

typedef DWORD (__stdcall *PTHREAD_START_ROUTINE) ( LPVOID lpThreadParameter);
typedef struct _KeyboardLL_Msg {
	int nCode;
	WPARAM wParam;
	KBDLLHOOKSTRUCT lParam;
} KeyboardLL_Msg, *PKeyboardLL_Msg;

//Global value
const char remotePath[]=REMOTE_DLL_PATH;
HANDLE hPort,hPro;
HWND hPiano;
DWORD keyList[256]={0};
bool isKeyDown[256]={0};

//Config
char portName[MAXN];

//Window Message
COPYDATASTRUCT CopyData={0};
KeyboardLL_Msg msg={0};
KBDLLHOOKSTRUCT *kbData=(KBDLLHOOKSTRUCT*)&msg.lParam;

//Temp
char recvBuf[3];

void InitKey()
{
	keyList['0']='0';
	keyList['1']='1';
	keyList['2']='2';
	keyList['3']='3';
	keyList['4']='4';
	keyList['5']='5';
	keyList['6']='6';
	keyList['7']='7';
	keyList['8']='8';
	keyList['9']='9';
	
	keyList['-']=VK_OEM_MINUS;
	keyList['=']=VK_OEM_PLUS;
	
	keyList['A']='A';
	keyList['B']='B';
	keyList['C']='C';
	keyList['D']='D';
	keyList['E']='E';
	keyList['F']='F';
	keyList['G']='G';
	keyList['H']='H';
	keyList['I']='I';
	keyList['J']='J';
	keyList['K']='K';
	keyList['L']='L';
	keyList['M']='M';
	keyList['N']='N';
	keyList['O']='O';
	keyList['P']='P';
	keyList['Q']='Q';
	keyList['R']='R';
	keyList['S']='S';
	keyList['T']='T';
	keyList['U']='U';
	keyList['V']='V';
	keyList['W']='W';
	keyList['X']='X';
	keyList['Y']='Y';
	keyList['Z']='Z';
}

HWND FindPiano()
{
	HWND hwnd = FindWindow(CLASS_NAME, WINDOW_TITLE);
	if(hwnd == NULL)
	{
		hwnd =  FindWindow(CLASS_NAME, NULL);
		if(hwnd == NULL)
		{
			hwnd =  FindWindow(NULL, WINDOW_TITLE);
		}
	}

	return hwnd;
}

bool Run(HANDLE *hPro)
{
	STARTUPINFO si={sizeof(si)};
	PROCESS_INFORMATION pi;
	
	if(CreateProcess(NULL,(LPSTR)PIANO_PATH,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))
	{
		*hPro=pi.hProcess;
		CloseHandle(pi.hThread);
		return true;
	}
	else
		return false;
}

void PostPushKey(char c)
{
	if(!isKeyDown[c])
	{
		isKeyDown[c]=true;

		msg.wParam=WM_KEYDOWN;
		kbData->vkCode=keyList[c];
		kbData->scanCode=MapVirtualKey(kbData->vkCode,0);
		kbData->flags=0;
		kbData->dwExtraInfo=0;
		SendMessage(hPiano,WM_COPYDATA,(WPARAM)hPiano,(LPARAM)&CopyData);
	}
}

void PostReleaseKey(char c)
{
	if(isKeyDown[c])
	{
		isKeyDown[c]=false;

		msg.wParam=WM_KEYUP;
		kbData->vkCode=keyList[c];
		kbData->scanCode=MapVirtualKey(kbData->vkCode,0);
		kbData->flags=128;
		kbData->dwExtraInfo=0;
		SendMessage(hPiano,WM_COPYDATA,(WPARAM)hPiano,(LPARAM)&CopyData);
	}
}

void ReadConf()
{
	char buf[MAXN]={0};
	
	//Port
	if(GetPrivateProfileString("Main","Port","",buf,sizeof(buf),".\\config.ini") > 0)
	{
		strcpy(portName,"\\\\.\\");
		strcat(portName,buf);
	}
	else
		strcpy(portName,DEFAULT_PORT_NAME);
	printf("[Debug] From Port: %s\n",portName);
}

void ErrorExit(const char *str)
{
	TerminateProcess(hPro,0);
	DWORD dRes=GetLastError();
	printf("[Error] %s\n -- Error Code: %d\n",str,dRes);
	system("pause>nul");
	exit(dRes);
}

void DebugOutput(const char *str)
{
	printf("[Debug] %s\n",str);
}

void StartPiano()
{
	DebugOutput("Starting the Piano...");
	if(!Run(&hPro) || hPro == INVALID_HANDLE_VALUE || hPro == NULL)
		ErrorExit("Fail to start the Piano!");
	DebugOutput("The Piano started.");
	
	Sleep(DELAY_AFTER_START);
	int strSize=sizeof(remotePath);
	PSTR remoteStr=(PSTR)VirtualAllocEx(hPro,NULL,strSize,MEM_COMMIT,PAGE_READWRITE);
	if(remoteStr == NULL)
		ErrorExit("Fail to Alloc memory in the Piano!");
	if(!WriteProcessMemory(hPro,remoteStr,(PVOID)remotePath,strSize,NULL))
		ErrorExit("Fail to Write data into the Piano!");
	
	PTHREAD_START_ROUTINE pfnThreadRtn=
		(PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("Kernel32"),"LoadLibraryA");
	if(pfnThreadRtn == NULL)
		ErrorExit("Fail to Find LoadLibraryA!");
	HANDLE hThr=CreateRemoteThread(hPro,NULL,0,pfnThreadRtn,remoteStr,0,NULL);
	if(hThr == NULL)
		ErrorExit("Fail to Create Remote thread!");
	WaitForSingleObject(hThr,INFINITE);
	
	DWORD dRet;
	if(!GetExitCodeThread(hThr,&dRet) || dRet == 0)
		ErrorExit("Fail to Load Remote.dll!");
	CloseHandle(hThr);
	VirtualFreeEx(hPro,remoteStr,0,MEM_RELEASE);
	DebugOutput("Piano Config Done.");
}

int main(int argc, char** argv) {
	InitKey();
	ReadConf();
	
	//Init WM
	msg.nCode=HC_ACTION;
	CopyData.cbData = sizeof(KeyboardLL_Msg);
	CopyData.dwData=0;
	CopyData.lpData=&msg;
	
	//Init Piano
	StartPiano();
	hPiano=FindPiano();
	if(hPiano == NULL)
		ErrorExit("Fail to Find the Piano!");
	//ShowWindow(hPiano,SW_MINIMIZE);
	DebugOutput("Catch the Piano succeed.");
	
	//Init MicroBit
	hPort=CreateFile(portName,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hPort == INVALID_HANDLE_VALUE)
		ErrorExit("MicroBit Device no found!");
	DebugOutput("MicroBit Device found.");
	
	DCB DCBData;
	if(!GetCommState(hPort,&DCBData))
		ErrorExit("Fail to Read DCBData!");
	DebugOutput("Read DCBData succeed.");
	
	DCBData.BaudRate=BAUD_RATE;
	DCBData.fParity=PARITY;
	DCBData.ByteSize=BYTE_SIZE;
	DCBData.StopBits=STOP_BYTE;
	if(!SetCommState(hPort,&DCBData))
		ErrorExit("Fail to Write DCBData!");
	DebugOutput("Write DCBData succeed.");
	
	if(!PurgeComm(hPort, PURGE_RXCLEAR|PURGE_TXCLEAR|PURGE_RXABORT|PURGE_TXABORT))
		ErrorExit("Fail to Init Clear!");
	DebugOutput("Init succeed.");
	
	DebugOutput("Connect to MicroBit Device Succeed!");
	
	DWORD dRead,dNowRead;
	while(true)
	{
		dRead=0;
		while(dRead < 2)
		{
			if(!ReadFile(hPort,recvBuf,2-dRead,&dNowRead,NULL))
				break;
			dRead+=dNowRead;
		}
		if(recvBuf[0] == '1')
			PostPushKey(recvBuf[1]);
		else
			PostReleaseKey(recvBuf[1]);
		putc(recvBuf[1],stdout);
	}
	
	CloseHandle(hPort);
	return 0;
}
