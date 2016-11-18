#pragma once
#ifndef RECEIVE_H
#define RECEIVE_H

#include <windows.h>
#include <vector>
#include <string>
#include <queue>
#include "timer.h"
#include "constants.h"
#include "packet.h"

namespace receive {
	class Reception {
	private:
		DWORD packetCounter;
		DWORD errorCounter;
		Process process;
		BOOL isPacketTimedOut;

		void sendACK(HANDLE handleCom);
		BOOL waitForPacket(HANDLE handleCom, std::vector<BYTE> &buffer);
		BOOL retrievePacket(HANDLE handleCom, std::vector<BYTE> &buffer);
		BOOL parsePacket(Packet &packet, std::vector<BYTE> &buffer);
		BOOL validatePacket(Packet &packet);
		void errorStat(HWND hwnd);
		void packetTimeout();
		typedef Timer<Reception, &packetTimeout, (DATA_SIZE + CRC_SIZE + 1 / BAUD_RATE)*3> AckTimer;
		AckTimer packetTimer;
	public:
		BOOL start(HWND handleWin, HANDLE handleCom);
	};

	class Process {
	private:
		std::queue<BYTE *> dataQueue;
		std::vector<char> writeBuffer;
		BOOL isProcessing = false;
		HANDLE processThread;
		DWORD threadId;
		static DWORD WINAPI readCharacters(LPVOID params);
		BOOL handleChar(char c);
		void writeCharToBuffer(char c);
		void displayChar(char c);
		BOOL saveBufferToFile();
		void successSaveFile();
		void failToSaveFile();
	public:
		void startProcess(HWND &hwnd, BYTE *data);
	};
};
#endif // !RECEIVE_H
