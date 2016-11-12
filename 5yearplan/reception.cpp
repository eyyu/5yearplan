#include "reception.h"

using namespace receive;

BOOL Reception::start(HWND handleWin, HANDLE handleCom) {
	Packet packet;
	std::vector<BYTE> buffer;

	// instance an object of COMMTIMEOUTS.
	COMMTIMEOUTS comTimeOut;
	// Specify time-out between charactor for receiving.
	comTimeOut.ReadIntervalTimeout = MAXWORD;
	// Specify value that is multiplied 
	// by the requested number of bytes to be read. 
	comTimeOut.ReadTotalTimeoutMultiplier = 0;
	// Specify value is added to the product of the 
	// ReadTotalTimeoutMultiplier member
	comTimeOut.ReadTotalTimeoutConstant = 0;
	SetCommTimeouts(handleCom, &comTimeOut);

	sendACK(handleCom);
	while (waitForPacket(handleCom, buffer)) {
		if (retrievePacket(handleCom, buffer)) {
			if (parsePacket(packet,buffer) && validatePacket(packet)) {
				process.startProcess(handleWin, packet.data);
			}
		}
	}
}

void Reception::sendACK(HANDLE handleCom) {
	char ch = ACK;
	DWORD dwWritten;
	DWORD ackFailCounter = 0;
	// Issue write.
	while (!WriteFile(handleCom, &ch, 1, &dwWritten, NULL)) {
		ackFailCounter++;
	}
}
BOOL Reception::waitForPacket(HANDLE handleCom, std::vector<BYTE> &buffer) {
	packetTimer.reset();
	packetTimer.start();
	DWORD dwRead;
	char  chRead;
	isPacketTimedOut = false;
	BOOL result = false;
	while (!isPacketTimedOut) {
		if (ReadFile(handleCom, &chRead, 1, &dwRead, NULL)) {
			buffer.push_back(chRead);
			result = true;
			break;
		}
	}
	packetTimer.stop();
	return false;
};

BOOL Reception::retrievePacket(HANDLE handleCom, std::vector<BYTE> &buffer) {
	DWORD dwRead;
	char  chRead;
	buffer.clear();
	do {
		if (ReadFile(handleCom, &chRead, 1, &dwRead, NULL))
			buffer.push_back(chRead);
		else
			break;
	} while (dwRead);

	if (buffer.size() != SYN_SIZE + DATA_SIZE + CRC_SIZE)
		return false;

}
BOOL Reception::parsePacket(Packet &packet, std::vector<BYTE> &buffer) {
	copy(buffer.begin(), buffer.begin() + SYN_SIZE, packet.syn);
	copy(buffer.begin() + SYN_SIZE, buffer.begin() + DATA_SIZE + SYN_SIZE, packet.data);
	copy(buffer.begin() + DATA_SIZE + SYN_SIZE, buffer.end(), packet.crc);

	if (packet.syn != SYN)
		return false;
	return true;
}
BOOL Reception::validatePacket(Packet &packet) {

}
void Reception::errorStat(HWND hwnd) {

}
void Reception::packetTimeout() {
	isPacketTimedOut = true;
};










void  Process::startProcess(HWND &hwnd, BYTE *data) {
	dataQueue.push(data);
	if (!isProcessing)
		processThread = CreateThread(NULL, 0, readCharacters, this, 0, &threadId);
}

DWORD WINAPI Process::readCharacters(LPVOID param) {
	Process* thisObj = (Process*)(param);

	thisObj->isProcessing = true;
	while (!thisObj->dataQueue.empty()) {
		BYTE *data = thisObj->dataQueue.front();
		for (int i = 0; i < DATA_SIZE; i++) {
			thisObj->handleChar(data[i]);
		}
	}
	thisObj->isProcessing = false;
	CloseHandle(thisObj->processThread);
	return 0;
}
BOOL Process::handleChar(char c) {
	if (c == NULL_BYTE)
		if (saveBufferToFile())
			successSaveFile();
		else
			failToSaveFile();
	else {
		displayChar(c);
		writeCharToBuffer(c);
		return true;
	}
	return false;
}
void Process::writeCharToBuffer(char c) {
		writeBuffer.push_back(c);
}

BOOL Process::saveBufferToFile() {
	return true;
}
void Process::successSaveFile() {

}
void Process::failToSaveFile() {

}