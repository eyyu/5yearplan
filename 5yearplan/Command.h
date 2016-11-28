#pragma once
#include <windows.h>
#include <stdio.h>
#include "winmenu2.h"
#include "Connect.h"


char Name[] = "Wireless communication portal";
LPCSTR    lpszCommName = "COM5";
char comPort[10];//Currently selected COMM PORT
char str[80] = "";
OPENFILENAME ofn;// Structure that contains attachment file info
char szFile[100];
LPTSTR filePath;//String that holds path of the browsed file
HINSTANCE hInstance;

//Function prototypes
void generateViews(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void selectCommPort(HANDLE, LPCSTR, HWND, bool&);
void createUIWindows(HWND);
void attach(void);
void availableCOM(HWND);
INT_PTR CALLBACK comDialogProc(HWND, UINT, WPARAM, LPARAM);

//Window handlers and variables
static const int WIN_LENGTH = 500;          // Main window length
static const int WIN_WIDTH = 650;          // Main window height

//DATA MEMBERS
static const DWORD BUFF_SIZE = 256;
static const DWORD TEXTBOX_HEIGTH = 200;
static const DWORD TEXTBOX_WIDTH = 350;
static const DWORD USERINPUT_TEXTBOX_START_X = 240;
static const DWORD USERINPUT_TEXTBOX_START_Y = 10;
static const DWORD READINPUT_TEXTBOX_START_X = 240;
static const DWORD READINPUT_TEXTBOX_START_Y = 220;
static const DWORD STATS_TEXTBOX_START_X = 20;
static const DWORD STATS_TEXTBOX_START_Y = 220;
static const DWORD BUTTON_WIDTH = 100;
static const DWORD BUTTON_HEIGHT = 30;
static const DWORD CONNECT_BUTTON_X = 50;
static const DWORD CONNECT_BUTTON_Y = 20;
static const DWORD DISCONNECT_BUTTON_X = 50;
static const DWORD DISCONNECT_BUTTON_Y = 60;
static const DWORD ATTACH_BUTTON_X = 50;
static const DWORD ATTACH_BUTTON_Y = 100;

HDC hdc;
TEXTMETRIC tm;
HWND userInputTextBox;
HWND readInputTextBox;
HWND statsTextBox;
HWND hConnectButton;
HWND hDisconnectButton;
HWND browseButton;
HWND label;
HWND label2;