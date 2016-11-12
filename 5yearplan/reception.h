#pragma once
#ifndef RECEIVE_H
#define RECEIVE_H

#define DATA_SIZE 1024
#define CRC_SIZE 2
#define BAUD_RATE 9600
#define SYN 0x16
#define ACK 0x06
#define NULL_BYTE 0x00

#include <windows.h>
#include <vector>
#include <string>
#include <queue>

namespace receive {


	struct Packet {
		BYTE syn;
		BYTE data[DATA_SIZE];
		BYTE crc[CRC_SIZE];
	};

	class Reception {
	private:
		DWORD packetCounter;
		DWORD errorCounter;
		Process process;

		void sendACK(HANDLE handleCom);
		BOOL waitForPacket(HANDLE handleCom);
		BOOL retrievePacket(HANDLE handleCom, Packet &packet);
		BOOL parsePacket(Packet &packet);
		BOOL validatePacket(Packet &packet);
		void errorStat(HWND hwnd);
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
		void startProcess(HWND &hwnd, BYTE *d);
	};
};
#endif // !RECEIVE_H
