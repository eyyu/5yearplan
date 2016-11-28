/*------------------------------------------------------------------------------
-- SOURCE FILE: Command.cpp - The COMMAND or IDLE state of the protocol
--
-- PROGRAM: [5YearPlan] 
--
-- FUNCTIONS:
-- *list all functions here!* 
-- [returnType] [funcName] (funcParams)
--
-- DATE: Nov. 09, 2016
--
-- REVISIONS: 
-- Version 1.2.
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- NOTES:
-- This is the main starting point for the communication once the port has been open.  
------------------------------------------------------------------------------*/
#pragma once

#include <thread>
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
