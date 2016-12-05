/*------------------------------------------------------------------------------
-- SOURCE FILE: main.cpp - The COMMAND state of the protocol. 
-- 							Main entrance point
--
-- PROGRAM: 5YearPlan
--
-- DATE: Nov. 29, 2016
--
-- FUNCTIONS:
-- BOOL CALLBACK 	WndProc			(HWND, UINT, WPARAM, LPARAM);
-- BOOL 			attach			(void);
-- INT_PTR CALLBACK comDialogProc	(HWND, UINT, WPARAM, LPARAM);
-- void 			configCommPort	(HANDLE, LPCSTR, HWND);
-- void 			availableCOM	(HWND);
-- void 			buttonEnable	(void);
-- void 			buttonDisable	(void);
--
-- REVISIONS:
-- Version 1.0.1.0 - [TM] - 2016/NOV/19 - created functions
-- Version 2.0.1.0 - [TM] - 2016/NOV/29 - updated version 
--
-- DESIGNER: Tim Makimov & Eva Yu
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

/****************************
DATA MEMEBER DECLARATIONS*
******************************/
LPCSTR	lpszCommName = "COM5"; // *** TO TM: IF NOT NEEDED, PLEASE REMOVE. 

LPTSTR filePath; 	//String that holds path of the browsed file
string text;		//Holds user's input

char szFile[100];  // *** TO TM: PLEASE REMOVE IF NOT NEEDED!
char comPort[10];	//Currently selected COMM PORT

COMMCONFIG cc; 		// configuration struct for com port.
OPENFILENAME ofn;// Structure that contains attachment file info
HDC hdc;
HINSTANCE hInstance;
HWND hwnd1; // Window handler for the main window
MSG Msg;//Holds message(s) from message queue

/****************************
MEMEBER FUNCTION DECLARATIONS*
******************************/
BOOL CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL attach(void);
INT_PTR CALLBACK comDialogProc(HWND, UINT, WPARAM, LPARAM);
void configCommPort(HANDLE, LPCSTR, HWND);
void availableCOM(HWND);
void buttonEnable();
void buttonDisable();