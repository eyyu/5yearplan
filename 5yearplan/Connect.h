/*------------------------------------------------------------------------------
-- SOURCE FILE: Command.cpp - The COMMAND or IDLE state of the protocol
--
-- PROGRAM: [5YearPlan] 
--
-- FUNCTIONS:
-- *list all functions here!* 
-- bool startConnectProc	(HWND, HWND);
-- bool startConnnection	(LPCTSTR, HWND);
-- bool stopConnnection		(void);
-- bool sendNewFile			(LPCSTR);
-- bool sendNewData			(LPCSTR);
-- bool writeChar			(const char);
-- void enqLine				(void);
-- void startRandomEnqTimer	(void);
-- void resetDataValues		(void);
--
-- DATE: Nov. 09, 2016
--
-- REVISIONS: 
-- Version 1.0.1.0 - [EY] - 2016/NOV/09  - created class 
-- Version 1.0.1.5 - [EY] - 2016/NOV/15	 - turned class to classless 
-- Version 2.0.1.0 - [TK] - 2016/NOV/28  - added overalpping class 
-- Version 2.0.1.1 - [EY] - 2016/DEC/03  - fixed biasing issue    
--
-- DESIGNER: Eva Yu / John Agapeyev / Tim Makimov / Terry Kang
--
-- PROGRAMMER: Eva Yu
--
-- NOTES:
-- This is the main starting point for the communication.
-- the functions deal with idle state handling and the ENQ and ACK procedures in 
-- idle state   
------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>

#include "constants.h"
#include "transmission.h"
#include "reception.h"
#include "timer.h"

static int enqCount = 0;
static bool isConnected = false;
static bool isReading = false;
static bool isWriting = false;
static bool isWaitingForPacket = false;
static bool isWaitingForAck = false;
static int  packetCount = 0;

static HANDLE  hComm = nullptr;

static std::thread connectedThread;

static transmit::Transmitter   TX;
static receive::Reception      RX;

bool startConnectProc(HWND, HWND);

bool startConnnection(LPCTSTR, HWND);
bool stopConnnection();
bool sendNewFile(LPCSTR);
bool sendNewData(LPCSTR);
bool writeChar(const char);

void enqLine();
void startRandomEnqTimer();
void resetDataValues();

static Timer <&enqLine, RAN_TIMER_MIN, RAN_TIMER_MAX> randomEnqTimer;
static Timer <&startRandomEnqTimer, IDLE_STATE_TIME> idleStateTimer;
