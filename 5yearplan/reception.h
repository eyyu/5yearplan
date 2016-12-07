/*------------------------------------------------------------------------------
-- SOURCE FILE: reception.h - receiption calss heaader
--
-- PROGRAM: 5yearplan
--
-- FUNCTIONS:
-- PROCESS CLASS:
-- static DWORD WINAPI readCharacters(LPVOID params);
-- BOOL handleChar          (char);
-- void writeCharToBuffer   (char);
-- void displayChar         (char);
-- BOOL saveBufferToFile    (void);
-- void successSaveFile     (void);
-- void failToSaveFile      (void);
-- void cls                 (void);
-- void resetProcess        (void);
-- void startProcess        (HWND, std::string&);
--
-- RECEPTION CLASS:
-- void sendACK             (HANDLE );
-- BOOL waitForPacket       (HANDLE );
-- BOOL retrievePacket      (HANDLE , std::vector<BYTE> &);
-- BOOL parsePacket         (Packet &, std::vector<BYTE> &);
-- BOOL validatePacket      (Packet &);
-- void errorStat           (HWND );
-- void packetTimeout       (void);
-- BOOL start               (HWND , HWND , HANDLE);
-- void closeReceiption     (void);
--
-- DATE: NOV. 09, 2016
--
-- REVISIONS:
-- Version 1.0.1.0 - [TK] - 2016/NOV/09 - created class
-- Version 2.0.1.0 - [TK] - 2016/NOV/28th - added overalpping class
--
-- DESIGNER: Eva Yu & Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- NOTES:
-- This is the receiption header for the reception class
-- the receiption class is responsible for reading
-- and parsing incoming packets
-- this namespace also includes the PROCESS class
-- process is a sub class of the RX side.
-- process creates its own thread and processes
-- the incoming packets after verification
------------------------------------------------------------------------------*/

#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <queue>
#include <regex>

#include "timer.h"
#include "constants.h"
#include "packet.h"

namespace receive {
	/**GLOBAL CONSTS**/
	static const DWORD TEXTBOX_HEIGTH = 200;
	static const DWORD TEXTBOX_WIDTH = 400;

	/*********************/
	/**  PROCESS CLASSS **/
	/*********************/
	class Process {
	private:
		/**DATA MEMBERS**/
		std::queue<std::string> dataQueue;
		std::string writeBuffer;
		BOOL isProcessing = false;
		HANDLE processThread;
		DWORD threadId;
		HWND handleDisplay;
		int char_x;
		int char_y;

		DWORD completedFileCounter;

		/**MEMBER FUNCTIONS**/
		static DWORD WINAPI readCharacters(LPVOID params);
		BOOL handleChar(char c);
		void writeCharToBuffer(char c);
		void displayChar();
		BOOL saveBufferToFile();
		void successSaveFile();
		void failToSaveFile();
		void cls();
	public:
		void resetProcess();
		void startProcess(HWND handleDisplayParam, std::string&);
	};
	/***********************/
	/**  RECEPTIONS CLASS **/
	/***********************/
	class Reception {
	private:
		/**DATA MEMBERS**/
		DWORD packetCounter;
		DWORD errorCounter;
		DWORD ackCounter;
		Process process;

		static BOOL isPacketTimedOut;

		/**MEMBER FUNCTIONS**/
		void sendACK(HWND handleDisplay, HANDLE handleCom);
		BOOL waitForPacket(HANDLE handleCom);
		BOOL retrievePacket(HANDLE handleCom, std::vector<char> &buffer);
		BOOL parsePacket(Packet &packet, std::vector<char> &buffer);
		BOOL validatePacket(Packet &packet);
		void errorStat(HWND handleStat);
		static void packetTimeout();
		typedef Timer<&packetTimeout, TRANSMISSION_TIMEOUT * 2 + BYTE_TIMEOUT> PacketTimer;
		PacketTimer packetTimer;
	public:
		BOOL start(HWND handleDisplay, HANDLE handleCom);
		void closeReceiption();

	};

};