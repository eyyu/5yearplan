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


	static constexpr unsigned long  RAN_TIMER_MIN = 0; // in ms
	static constexpr unsigned long  RAN_TIMER_MAX = 100; // in ms
	static constexpr unsigned long  IDLE_STATE_TIME = 500; //ms


		int		enqCount = 0;
		bool	isConnected = false;
		bool	isReading = false;
		bool	isWriting = false;
		bool	isWaitingForPacket = false;
		bool	isWaitingForAck = false;
		int	    packetCount = 0;

		HANDLE  hComm = NULL;

		std::thread connectedThread;

		transmit::Transmitter   TX;
		receive::Reception      RX;

		bool startConnectProc(HWND, HWND);

		bool startConnnection(LPCTSTR, HWND);
		bool stopConnnection();
		bool sendNewFile(LPCSTR);
		bool sendNewData(LPCSTR);
		bool writeChar(const char);

		int  getEnqCount();
		void incrementEnqCount();
		void resetEnqCount();

		void enqLine();
		void startRandomEnqTimer();

	Timer < &enqLine, RAN_TIMER_MIN, RAN_TIMER_MAX> randomEnqTimer ;
	Timer < &startRandomEnqTimer, IDLE_STATE_TIME > idleStateTimer ;
