// Program WinMenu.cpp
// COMP 3980, Final Project
// Tim Makimov, A009031109
//Test
#define STRICT

#include "Command.h"

#pragma warning (disable: 4096)


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
	LPSTR lspszCmdParam, int nCmdShow)
{
	MSG Msg;
	generateViews(hInst, nCmdShow);
	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message,
	WPARAM wParam, LPARAM lParam)
{
	HDC hdc = NULL;
	PAINTSTRUCT paintstruct;

	switch (Message)
	{
	case WM_COMMAND:

		if ((HWND)lParam == hConnectButton)
		{
			connect(hComm, lpszCommName, connected);
			break;
		}
		if ((HWND)lParam == hDisconnectButton)
		{
			disconnect(hComm, connected, lpszCommName);
			break;
		}
		if ((HWND)lParam == browse)
		{
			attach();
			break;
		}
		switch (LOWORD(wParam))
		{
		case IDM_COM1:
			lpszCommName = "COM1";
			selectCommPort(hComm, lpszCommName, hwnd, connected);
			break;
		case IDM_COM2:
			lpszCommName = "COM2";
			selectCommPort(hComm, lpszCommName, hwnd, connected);
			break;
		case IDM_COM3:
			lpszCommName = "COM3";
			selectCommPort(hComm, lpszCommName, hwnd, connected);
			break;
		case IDM_COM4:
			lpszCommName = "COM4";
			selectCommPort(hComm, lpszCommName, hwnd, connected);
			break;
		case IDM_HELP:
			MessageBox(NULL, 
					"Click CONNECT to connect to the default COMM port or select a COMM port from settings and then click CONNECT.", 
					"", MB_OK);
			break;
		case IDM_Exit:
			PostQuitMessage(0);
			break;
		}
		break;

	case WM_CHAR:	// Process keystroke
		if (wParam == 27)
		{
			disconnect(hComm, connected, lpszCommName);
		}
		break;

	case WM_PAINT:		// Process a repaint message
		hdc = BeginPaint(hwnd, &paintstruct); // Acquire DC
		TextOut(hdc, 0, 0, str, strlen(str)); // output character
		EndPaint(hwnd, &paintstruct); // Release DC
		break;

	case WM_DESTROY:	// Terminate program
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

void generateViews(HINSTANCE hInst, int nCmdShow)
{
	HWND hwnd;
	WNDCLASSEX Wcl;

	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // cursor style

	Wcl.lpfnWndProc = WndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //white background
	Wcl.lpszClassName = Name;

	Wcl.lpszMenuName = "MYMENU"; // The menu Class
	Wcl.cbClsExtra = 0;      // no extra memory needed
	Wcl.cbWndExtra = 0;

	if (!RegisterClassEx(&Wcl))
		return;

	hwnd = CreateWindow(Name, 
						Name, 
						WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,			
						CW_USEDEFAULT, CW_USEDEFAULT,
						WIN_WIDTH, WIN_LENGTH,
						NULL, //parent of window
						NULL, //menu bar
						hInst, // first aparam in WinMain
						NULL);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	createUIWindows(hwnd);
}

void selectCommPort(HANDLE hComm, LPCSTR lpszCommName, HWND hwnd, bool &connected)
{
	COMMCONFIG	cc;
	cc.dwSize = sizeof(COMMCONFIG);
	cc.wVersion = 0x100;
	GetCommConfig(hComm, &cc, &cc.dwSize);
	if (!CommConfigDialog(lpszCommName, hwnd, &cc))
	{
		MessageBox(NULL, "Error connecting to COMM port", lpszCommName, MB_OK);
		return;
	}
	else
	{
		//connect(hComm, lpszCommName, connected);
	}
	if ((SetCommState(hComm, &cc.dcb)) == FALSE)
	{
		return;
	}
}


void connect(HANDLE &commPort, LPCSTR CommName, bool &connection)
{
	if (connection == false)
	{
		//if startConnection = false
		if ((commPort = CreateFile(CommName, GENERIC_READ | GENERIC_WRITE, 0,
			NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL))
			== INVALID_HANDLE_VALUE)
		{
			MessageBox(NULL, "Error opening COM port", CommName, MB_OK);
			//invoke HANDLE CONNECTION ERRORS
		}
		else
		{
			MessageBox(NULL, "Conected to port", CommName, MB_OK);
			connection = true;
			EnableWindow(hConnectButton, !connection);
			EnableWindow(hDisconnectButton, connection);
		}
	}
	else
	{
		return;
	}
}


void disconnect(HANDLE& commPort, bool& connection, LPCSTR CommName)
{
	if (connection == true)
	{
		CloseHandle(commPort);
		//ExitThread(threadId);//terminate thread
		MessageBox(NULL, "Disconnected from port", CommName, MB_OK);
		connection = false;
		EnableWindow(hDisconnectButton, connection);
		EnableWindow(hConnectButton, !connection);
	}
	else
	{
		return;
	}
}


void createUIWindows(HWND hwnd)
{
	userInputTextBox = CreateWindow("STATIC",
		NULL,
		WS_BORDER | WS_CHILD | WS_VISIBLE | WS_DISABLED,
		USERINPUT_TEXTBOX_START_X,
		USERINPUT_TEXTBOX_START_Y,
		TEXTBOX_WIDTH,
		TEXTBOX_HEIGTH,
		hwnd,
		NULL,
		NULL,
		NULL
	);

	readInputTextBox = CreateWindow(
		"STATIC",
		NULL,
		WS_BORDER | WS_CHILD | WS_VISIBLE | WS_DISABLED, 
		READINPUT_TEXTBOX_START_X,
		READINPUT_TEXTBOX_START_Y,
		TEXTBOX_WIDTH,
		TEXTBOX_HEIGTH,
		hwnd,
		NULL,
		NULL,
		NULL
	);
	hConnectButton = CreateWindow(
		"BUTTON",
		"CONNECT",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		50,
		200,
		BUTTON_WIDTH,
		BUTTON_HEIGHT,
		hwnd,
		NULL,
		NULL,
		NULL
	);
	hDisconnectButton = CreateWindow(
		"BUTTON",
		"DISCONNECT",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED,
		50,
		250,
		BUTTON_WIDTH,
		BUTTON_HEIGHT,
		hwnd,
		NULL,
		NULL,
		NULL
	);

	browse = CreateWindow(
		"BUTTON",
		"ATTACH",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		50,
		300,
		BUTTON_WIDTH,
		BUTTON_HEIGHT,
		hwnd,
		NULL,
		NULL,
		NULL
	);
}

void attach()
{
	// open a file name
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ENABLEHOOK;
	
	if (GetOpenFileName(&ofn))
	{
		// Now simpley display the file name 
		MessageBox(NULL, ofn.lpstrFile, "File Name", MB_OK);
	}

	
}