// Program WinMenu.cpp
// COMP 3980, Final Project
// Tim Makimov, A009031109

#define STRICT

#include "Command.h"

#pragma warning (disable: 4096)
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")



int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
	LPSTR lspszCmdParam, int nCmdShow)
{
	MSG Msg;
	hInstance = hInst;
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

	//availableCOM(hwnd);

	switch (Message)
	{
	case WM_COMMAND:

		if ((HWND)lParam == hConnectButton)
		{
			connect(hComm, comPort, connected);
			break;
		}
		if ((HWND)lParam == hDisconnectButton)
		{
			disconnect(hComm, connected, comPort);
			break;
		}
		if ((HWND)lParam == browse)
		{
			attach();
			break;
		}
		switch (LOWORD(wParam))
		{
		case IDM_NEWCONNECT:
			DialogBox(hInstance, MAKEINTRESOURCE(IDD_COMDIALOG), hwnd, comDialogProc); // a dialog for the list of available com port.
			break;

		case IDM_PROPERTIES: // popup a dialog for changing the properties of selected com port.
			selectCommPort(hComm, comPort, hwnd, connected);
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
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_COMDIALOG), hwnd, comDialogProc); // a dialog for the list of available com port.
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
		CONNECT_BUTTON_X,
		CONNECT_BUTTON_Y,
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
		DISCONNECT_BUTTON_X,
		DISCONNECT_BUTTON_Y,
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
		ATTACH_BUTTON_X,
		ATTACH_BUTTON_Y,
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
		filePath = ofn.lpstrFile;
		
		// Now simply display the file name 
		MessageBox(NULL, filePath, "File Name", MB_OK);
	}
}

void availableCOM(HWND hwnd) {
	TCHAR szDevices[65535];
	unsigned long dwChars = QueryDosDevice(NULL, szDevices, 65535); // get all available devices.
	TCHAR *ptr = szDevices;

	while (dwChars)
	{
		int port;
		if (sscanf_s(ptr, "COM%d", &port) == 1 || sscanf_s(ptr, "\\\\.\\COM%d", &port) == 1) // if the availbel device name begins with COM,
		{
			SendDlgItemMessage(hwnd, IDM_COM_COMBOBOX, CB_ADDSTRING, 0, (LPARAM)ptr); // populates the combobox with the device name.
		}
		TCHAR *temp_ptr = strchr(ptr, 0);
		dwChars -= (DWORD)((temp_ptr - ptr) / sizeof(TCHAR) + 1);
		ptr = temp_ptr + 1; // point to next device.
	}
	SendDlgItemMessage(hwnd, IDM_COM_COMBOBOX, CB_SETCURSEL, (WPARAM)0, 0L); // set focus on the first item in the list.
}


INT_PTR CALLBACK comDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_INITDIALOG:
		availableCOM(hDlg); // Check the list of available com ports and populate the combobox with it.
		break;

	case WM_COMMAND:
		switch (wParam) {
		case IDM_CANCEL:
			EndDialog(hDlg, 0); // close dialog
			return TRUE;
		case IDM_OK:
			GetDlgItemText(hDlg, IDM_COM_COMBOBOX, comPort, sizeof(comPort)); // get selected port name from the list.
			//session.initilize(comPort); // initilize the session for the selected com port.
			EndDialog(hDlg, 0); // close dialog
			return TRUE;
		}
		break;

	case WM_SETFOCUS:
		return TRUE;
	}
	return FALSE;
}

//void selectComport() {
//	DialogBox(hInst, MAKEINTRESOURCE(IDD_COMDIALOG), hwnd, comDialogProc); // a dialog for the list of available com port.
//}