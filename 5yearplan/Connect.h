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

#include "transmission.h"
#include "reception.h"
#include "timer.h"


namespace connect{

   //Default timer with member function pointer
    class Connection {
    private: 
        static const int			MAX_ENQ_COUNT	= 3 ; // in ms
        static const int			BUFF_SIZE		= 1027; // in ms
        static const unsigned long  RAN_TIMER_MIN = 0; // in ms
        static const unsigned long  RAN_TIMER_MAX  = 100; // in ms
		static const unsigned long  IDLE_STATE_TIME = 500; // (2/9600)s ; measure in in MicroSec
       
        int              enqCount;
        CHAR   inBuffer[BUFF_SIZE];
        HANDLE hWrite;
        HANDLE hRead;
        
        std::thread readThread;
        std::thread writeThread;
        
		transmit::Transmitter   TX;
        receive::Reception      RX;

        void readInBuffer(); // event driven
        void enqLine();
        void startTx();
        //three handles: one for displaying data , one stats , handle to comm port 
        //  
        void startRx(HWND hDisplay, HANDLE hcomm);
        void startRandomEnqTimer();
        void stopRandomEnqTimer();
        void startIdleStateTimer();
        void stopIdleStateTimer();

		Timer <Connection, &enqLine, RAN_TIMER_MIN, RAN_TIMER_MAX> randomEnqTimer;
		Timer <Connection, &startRandomEnqTimer, IDLE_STATE_TIME > idleStateTimer;
    public:
        Connection();
        bool startConnnection(LPCTSTR commPortAddress);
        bool stopConnnection();
    };

}