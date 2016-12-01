/*------------------------------------------------------------------------------
-- SOURCE FILE: main.cpp - The COMMAND state of the protocol
--
-- PROGRAM: [5YearPlan]
--
-- DATE: Nov. 29, 2016
--
-- REVISIONS:
-- Version 2.0.
--
-- DESIGNER: Tim Makimov
--
-- PROGRAMMER: Tim Makimov
--
-- NOTES:
-- This is the main starting point of the program - controls UI and Dialog's behavior
------------------------------------------------------------------------------*/

#pragma once
#include <windowsx.h>
#include <stdio.h>
#include "winmenu2.h"
#include "Connect.h"
#include <string>

using namespace std;

LPCSTR	lpszCommName = "COM5";
LPTSTR filePath;//String that holds path of the browsed file
COMMCONFIG cc; // configuration struct for com port.
char szFile[100];
char comPort[10];//Currently selected COMM PORT
string text;//Holds user's input
OPENFILENAME ofn;// Structure that contains attachment file info
HDC hdc;
HINSTANCE hInstance;
HWND hwnd1; // Window handler for the main window
MSG Msg;//Holds message(s) from message queue

//Function prototypes
BOOL CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL attach(void);
INT_PTR CALLBACK comDialogProc(HWND, UINT, WPARAM, LPARAM);
void configCommPort(HANDLE, LPCSTR, HWND);
void availableCOM(HWND);
void buttonEnable();
void buttonDisable();