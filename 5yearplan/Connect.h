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
#include <Win32>
#include "timer.h"

namespace Connect{

   //Default timer with member function pointer
    class Connection {
    private: 
        static const int IDLE_TIMER      = 500 ; // in ms
        static const int RAN_TIMER_START = 0 ; // in ms
        static const int RAN_TIMER_STOP  = 100 ; // in ms

        //std::thread readThread;
        //std::thread writeThread;
        Timer <Connection, & enqLine, RAN_TIMER_START, RAN_TIMER_STOP> randomEnqTimer;
        Timer <Connection, & startRandomEnqTimer , IDLE_STATE_TIME > idleStateTimer;
        
        Transmit::Transmitter   TX;
        /****REPLACE WITH RX OBJECT!!!****/
        Recieve::Reception      RX;

        void readInBuffer(); // event driven
        void enqLine();
        void startTx();
        //three handles: one for displaying data , one stats , handle to comm port 
        //  
        void startRx(HWND hDisplay, HANDLE hcomm);
        void startRandomEnqTimer();
        void startIdleStateTimer();

    public:
        Connection();
        bool startConnnection(LPCTSTR commPortAddress);
        bool stopConnnection();
    };

}