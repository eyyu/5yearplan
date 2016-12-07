/*------------------------------------------------------------------------------
-- SOURCE FILE: main.cpp - The COMMAND state of the protocol
--
-- PROGRAM: 5YearPlan
--
-- FUNCTIONS:
--
-- DATE: DEC. 04, 2016
--
-- REVISIONS:
-- Version 3.2.
--
-- DESIGNER: Tim Makimov
--
-- PROGRAMMER: Tim Makimov
--
-- NOTES:
-- This is the main of the application - provides command and control.
------------------------------------------------------------------------------*/

#define STRICT
#include "Command.h"
#pragma warning (disable: 4096)
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

/*--------------------------------------------------------------------------
-- FUNCTION:  WinMain
--
-- DATE: DEC. 04, 2016
--
-- REVISIONS:
-- Version 1.0
--
-- DESIGNER: Tim Makimov
--
-- PROGRAMMER: Tim Makimov
--
-- INTERFACE: int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
--
-- RETURNS: If the function succeeds, terminating when it receives a WM_QUIT message,
--			it should return the exit value contained in that message's wParam parameter.
--			If the function terminates before entering the message loop, it should return zero.
--
-- NOTES: Retrieves messages from message loop translating incoming messages and dispatching them to the application's message
procedure
--------------------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
{
	hInstance = hInst;
	hwnd1 = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, WndProc);
	HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(MENU_MYMENU));
	SetMenu(hwnd1, hMenu);
	
	// Display & update window
	ShowWindow(hwnd1, nCmdShow);
	UpdateWindow(hwnd1);
	
	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}
/*--------------------------------------------------------------------------
-- FUNCTION:  WndProc
--
-- DATE: DEC. 04, 2016
--
-- REVISIONS:
-- Version 2.3
--
-- DESIGNER: Tim Makimov
--
-- PROGRAMMER: Tim Makimov
--
-- INTERFACE: BOOL CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM)
--
-- RETURNS: LRESULT - the result of the message processing and depends on the message sent
--
-- NOTES: Processes mesages received from WinMain
--------------------------------------------------------------------------*/
BOOL CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_COMMAND:
		if ((HWND)lParam == GetDlgItem(hwnd, BTN_CONNECT))
		{
			DialogBox(hInstance, MAKEINTRESOURCE(IDD_COMDIALOG), hwnd, comDialogProc);
			if (comPort[0] != 0)
			{
				startConnnection(comPort, hwnd);
				buttonEnable();
			}
			break;
		}
		if ((HWND)lParam == GetDlgItem(hwnd, BTN_DISCONNECT))
		{
			//show image
			int msgboxID = MessageBox(NULL, "Are you sure you want to disconnect? That may take several seconds", "Disconnect?", MB_OKCANCEL| MB_ICONQUESTION);
			if (msgboxID == IDOK)
			{
				buttonDisable();
				stopConnnection();
			}
			break;
		}
		if ((HWND)lParam == GetDlgItem(hwnd, BTN_ATTACH))
		{
			if (attach())
			{
				sendNewFile(filePath);
			}
			break;
		}
		if ((HWND)lParam == GetDlgItem(hwnd, BTN_SEND))
		{
			generateString();
			sendNewData(text.c_str());
			break;
		}
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			stopConnnection();
			PostQuitMessage(0);
			break;
		case MENU_PROP: // popup a dialog for changing the properties of selected com port.
				configCommPort(hComm, comPort, hwnd);
			break;
		case MENU_HELP:
			MessageBox(NULL,
				"Click CONNECT to connect to the default COMM port or select a COMM port from settings and then click CONNECT.",
				"", MB_OK | MB_ICONINFORMATION);
			break;
		case MENU_EXIT:
			stopConnnection();
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_CHAR:	// Process keystroke
		if (wParam == 27)
		{
			stopConnnection();
			buttonDisable();
		}
		break;
	case WM_DESTROY:	// Terminate program
		stopConnnection();
		PostQuitMessage(0);
		break;
	default:
		return FALSE;
	}
	return FALSE;
}
/*--------------------------------------------------------------------------
-- FUNCTION:  configCommPort
--
-- DATE: DEC. 04, 2016
--
-- REVISIONS:
-- Version 1.0
--
-- DESIGNER: Tim Makimov
--
-- PROGRAMMER: Tim Makimov
--
-- INTERFACE: void configCommPort(HANDLE, LPCSTR, HWND)
--
-- RETURNS: void
--
-- NOTES: Configures the DCB structure of the created Comm Port (BPS, Data bits, Parity, Stop bits, Flow Control)
--------------------------------------------------------------------------*/
void configCommPort(HANDLE hComm, LPCSTR lpszCommName, HWND hwnd)
{
	GetCommConfig(hComm, &cc, &cc.dwSize);
	if (!CommConfigDialog(lpszCommName, hwnd, &cc))
	{
		MessageBox(NULL, "Error connecting to COMM port", lpszCommName, MB_OK | MB_ICONERROR);
		return;
	}
}
/*--------------------------------------------------------------------------
-- FUNCTION:  attach
--
-- DATE: DEC. 04, 2016
--
-- REVISIONS:
-- Version 1.0
--
-- DESIGNER: Tim Makimov
--
-- PROGRAMMER: Tim Makimov
--
-- INTERFACE: BOOL attach()
--
-- RETURNS: true if file is openned and it's path is stored in ofn structure, else returns false
--
-- NOTES: Stores fie path in ofn structure, global variable filePath stores the file path
--------------------------------------------------------------------------*/
BOOL attach()
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
		Edit_SetText(GetDlgItem(hwnd1, ATTACHMENT), filePath);
		return true;
	}
	return false;
}
/*--------------------------------------------------------------------------
-- FUNCTION: availableCOM
--
-- DATE: DEC. 04, 2016
--
-- REVISIONS:
-- Version 1.0
--
-- DESIGNER: Tim Makimov
--
-- PROGRAMMER: Tim Makimov, Terry Kang
--
-- INTERFACE: void availableCOM(HWND hwnd)
--
-- RETURNS: void
--
-- NOTES: 
--	This function looks for the list of available com ports and populate the combobox from which
--	users can select the com port that they want to connect to.
--------------------------------------------------------------------------*/
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
/*--------------------------------------------------------------------------
-- FUNCTION: comDialogProc
--
-- DATE: DEC. 04, 2016
--
-- REVISIONS:
-- Version 1.0
--
-- DESIGNER: Tim Makimov
--
-- PROGRAMMER: Tim Makimov, Terry Kang
--
-- INTERFACE: INT_PTR CALLBACK comDialogProc(HWND, UINT, WPARAM, LPARAM)
--
-- RETURNS: TRUE if it processed the message, and FALSE if it did not.
-- If the dialog box procedure returns FALSE, the dialog manager performs the default dialog operation in response to the message.
--
-- NOTES:
-- Application-defined callback function used with the CreateDialog and DialogBox families of functions.
-- It processes messages that is from the dialog box for selecting available com port.
--------------------------------------------------------------------------*/
INT_PTR CALLBACK comDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_INITDIALOG:
		availableCOM(hDlg); // Check the list of available com ports and populate the combobox with it.
		break;
	case WM_COMMAND:
		switch (wParam) {
		case IDCANCEL:
			EndDialog(hDlg, 0); // close dialog
			return TRUE;
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
/*--------------------------------------------------------------------------
-- FUNCTION: buttonEnable
--
-- DATE: DEC. 04, 2016
--
-- REVISIONS:
-- Version 1.0
--
-- DESIGNER: Tim Makimov
--
-- PROGRAMMER: Tim Makimov
--
-- INTERFACE: void buttonEnable()
--
-- RETURNS: void
--
-- NOTES:
-- This function enables Disconnect, Browse and Send buttons and disables Connect button if Comm Port is connected
--------------------------------------------------------------------------*/
void buttonEnable()
{
	Button_Enable(GetDlgItem(hwnd1, BTN_CONNECT), FALSE);
	Button_Enable(GetDlgItem(hwnd1, BTN_DISCONNECT), TRUE);
	Button_Enable(GetDlgItem(hwnd1, BTN_ATTACH), TRUE);
	Button_Enable(GetDlgItem(hwnd1, BTN_SEND), TRUE);
}
/*--------------------------------------------------------------------------
-- FUNCTION: buttonDisable
--
-- DATE: DEC. 04, 2016
--
-- REVISIONS:
-- Version 1.0
--
-- DESIGNER: Tim Makimov
--
-- PROGRAMMER: Tim Makimov
--
-- INTERFACE: void buttonDisable()
--
-- RETURNS: void
--
-- NOTES:
-- This function disables Disconnect, Browse and Send buttons and enables Connect butto if Comm Port is connected
--------------------------------------------------------------------------*/
void buttonDisable()
{
	Button_Enable(GetDlgItem(hwnd1, BTN_CONNECT), TRUE);
	Button_Enable(GetDlgItem(hwnd1, BTN_DISCONNECT), FALSE);
	Button_Enable(GetDlgItem(hwnd1, BTN_ATTACH), FALSE);
	Button_Enable(GetDlgItem(hwnd1, BTN_SEND), FALSE);
}
/*--------------------------------------------------------------------------
-- FUNCTION: generateString
--
-- DATE: DEC. 04, 2016
--
-- REVISIONS:
-- Version 1.0
--
-- DESIGNER: Tim Makimov
--
-- PROGRAMMER: Tim Makimov
--
-- INTERFACE: void generateString
--
-- RETURNS: void
--
-- NOTES:
-- Stores user's input into a global string
--------------------------------------------------------------------------*/
void generateString()
{
	const int bufferLength = GetWindowTextLength(GetDlgItem(hwnd1, EDIT_TX)) + 1;
	text.resize(bufferLength);
	GetWindowText(GetDlgItem(hwnd1, EDIT_TX), &text[0], bufferLength);
	text.resize(bufferLength - 1);
	//SetWindowText(GetDlgItem(hwnd1, EDIT_RX), text.c_str()); 
}
