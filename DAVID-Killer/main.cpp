#include <conio.h>
#include <iostream>
#include <windows.h>
#include "TaskKill.h" 

using namespace std;

bool Run(HANDLE *hPro)
{
	STARTUPINFO si={sizeof(si)};
	PROCESS_INFORMATION pi;
	
	if(CreateProcess(NULL,(LPSTR)"DAVID.exe",NULL,NULL,FALSE,CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi))
	{
		*hPro=pi.hProcess;
		CloseHandle(pi.hThread);
		return true;
	}
	else
		return false;
}

int main(int argc, char** argv) {
	cout << "    -----  Waiting for Killing...  -----    \n";
	HANDLE hPro;
	while(true)
	{
		char ch=getch();
		if(ch == '\r')
		{
			cout << "[ Restarting... ]";
			TaskKill("DAVID.exe");
			TaskKill("freepiano.exe");

			if(!Run(&hPro) || hPro == INVALID_HANDLE_VALUE || hPro == NULL)
				cout << " Fail to kill DAVID!\n";
			else
				cout << " Killing DAVID succeed.\n";
		}
	}
	return 0;
}
