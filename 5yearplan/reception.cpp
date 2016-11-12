#include "reception.h"

using namespace receive;

BOOL Reception::start(HWND handleWin, HANDLE handleCom) {
	Packet packet;
	sendACK(handleCom);
	while (waitForPacket(handleCom)) {
		if (retrievePacket(handleCom, packet)) {
			if (parsePacket(packet) && validatePacket(packet)) {
				process.startProcess(handleWin, packet.data);
			}
		}
	}
}

void Reception::sendACK(HANDLE handleCom) {

}
BOOL Reception::waitForPacket(HANDLE handleCom) {

}
BOOL Reception::retrievePacket(HANDLE handleCom, Packet &packet) {

}
BOOL Reception::parsePacket(Packet &packet) {

}
BOOL Reception::validatePacket(Packet &packet) {

}
void Reception::errorStat(HWND hwnd) {

}











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