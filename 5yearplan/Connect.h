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


namespace connect {
	static constexpr unsigned long  RAN_TIMER_MIN = 0; // in ms
	static constexpr unsigned long  RAN_TIMER_MAX = 100; // in ms
	static constexpr unsigned long  IDLE_STATE_TIME = 500; //ms

	class Connection {
	private:

		static bool isConnected;
		static bool isWaitingForAck;
		static bool isWaitingForPacket;
		static bool isReading;
		static bool isWriting;

		static int    enqCount;

		static HANDLE hComm;

		static std::thread connectedThread;

		static transmit::Transmitter   TX;
		static receive::Reception      RX;

		static bool startConnectProc(HWND, HWND);

	public:
		Connection();
		static bool startConnnection(LPCTSTR, HWND);
		static bool stopConnnection();
		static bool sendNewFile(LPCSTR);
		static bool sendNewData(LPCSTR);
		static bool writeChar(const char);

		static int  getEnqCount();
		static void incrementEnqCount();
		static void resetEnqCount();

		static void enqLine();
		static void startRandomEnqTimer();

		
	};
	Timer < &Connection::enqLine, RAN_TIMER_MIN, RAN_TIMER_MAX> randomEnqTimer ;
	Timer < &Connection::startRandomEnqTimer, IDLE_STATE_TIME > idleStateTimer ;


}