// Program WinMenu.cpp
// COMP 3980, Final Project
// Tim Makimov, A009031109

#define STRICT

#include "Command.h"
#pragma warning (disable: 4096)
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

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
				//configCommPort(hComm, comPort, hwnd);
				Button_Enable(GetDlgItem(hwnd, BTN_CONNECT), FALSE);
				Button_Enable(GetDlgItem(hwnd, BTN_DISCONNECT), TRUE);
				Button_Enable(GetDlgItem(hwnd, BTN_ATTACH), TRUE);
			}
			break;
		}
		if ((HWND)lParam == GetDlgItem(hwnd, BTN_DISCONNECT))
		{
			stopConnnection();
			Button_Enable(GetDlgItem(hwnd, BTN_CONNECT), TRUE);
			Button_Enable(GetDlgItem(hwnd, BTN_DISCONNECT), FALSE);
			Button_Enable(GetDlgItem(hwnd, BTN_ATTACH), FALSE);
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
			GetWindowText(GetDlgItem(hwnd, EDIT_TX), message, 500);
			//test Send button and the user's iput
			Edit_SetText(GetDlgItem(hwnd, EDIT_RX), message);
		}
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			DestroyWindow(hwnd);
			break;

		//case MENU_NEW_CON:
		//	hwnd2 = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_COMDIALOG), hwnd, comDialogProc); // a dialog for the list of available com port.
		//	break;

		case MENU_PROP: // popup a dialog for changing the properties of selected com port.
				configCommPort(hComm, comPort, hwnd);
			break;

		case MENU_HELP:
			MessageBox(NULL,
				"Click CONNECT to connect to the default COMM port or select a COMM port from settings and then click CONNECT.",
				"", MB_OK);
			break;
		case MENU_EXIT:
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_CHAR:	// Process keystroke
		if (wParam == 27)
		{
			stopConnnection();
			Button_Enable(GetDlgItem(hwnd, BTN_CONNECT), TRUE);
			Button_Enable(GetDlgItem(hwnd, BTN_DISCONNECT), FALSE);
			Button_Enable(GetDlgItem(hwnd, BTN_ATTACH), FALSE);
		}
		break;
	case WM_DESTROY:	// Terminate program
		PostQuitMessage(0);
		break;
	default:
		return FALSE;
	}
	return FALSE;
}

void configCommPort(HANDLE hComm, LPCSTR lpszCommName, HWND hwnd)
{
	cc.dwSize = sizeof(COMMCONFIG);
	cc.wVersion = 0x100;
	GetCommConfig(hComm, &cc, &cc.dwSize);
	if (!CommConfigDialog(lpszCommName, hwnd, &cc))
	{
		MessageBox(NULL, "Error connecting to COMM port", lpszCommName, MB_OK);
		return;
	}
	if (!(SetCommState(hComm, &cc.dcb)))
	{
		MessageBox(NULL, "Error configuring comm settings", lpszCommName, MB_OK);
		return;
	}
}

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
		//MessageBox(NULL, filePath, "File Name", MB_OK);
		return true;
	}
	return false;
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