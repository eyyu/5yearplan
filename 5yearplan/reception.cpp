
#include "reception.h"

using namespace receive;

BOOL Reception::isPacketTimedOut = false;

BOOL Reception::start(HWND handleDisplay, HWND handleStat, HANDLE handleCom) {
	Packet packet;
	std::vector<BYTE> buffer;
	sendACK(handleCom);
	while (waitForPacket(handleCom)) {
		if (retrievePacket(handleCom, buffer)) {
			if (!parsePacket(packet, buffer))
				continue;
			if (validatePacket(packet)) {
				process.startProcess(handleDisplay, packet.data);
				sendACK(handleCom);
				return true;
			}
			else
				errorStat(handleDisplay);
		}
	}
	return false;
}

void Reception::sendACK(HANDLE handleCom) {
	char ch = ACK;
	DWORD dwWritten;
	DWORD ackFailCounter = 0;
	// Issue write.
	WriteFile(handleCom, &ch, 1, &dwWritten, NULL);
}
BOOL Reception::waitForPacket(HANDLE handleCom) {
	OVERLAPPED Timeout_Event;
	memset((char *)&Timeout_Event, 0, sizeof(OVERLAPPED));
	Timeout_Event.hEvent = CreateEvent(NULL, TRUE, TRUE, 0);
	DWORD dwCommEvent;
	BOOL result = false;

	if (!SetCommMask(handleCom, EV_RXCHAR)) // Set event listener for RX_CHAR, occurs when a message comes from the port.
		MessageBox(NULL, "SetCommMask", "Error", MB_OK | MB_ICONINFORMATION);

	if (WaitCommEvent(handleCom, &dwCommEvent, &Timeout_Event))
	{ // wait for RXCHAR event.
		result = true;
	}
	else
	{
		if (GetLastError() == ERROR_IO_PENDING)
		{
			if (WaitForSingleObject(Timeout_Event.hEvent, (DWORD)RECEPTION_TIMEOUT) == WAIT_OBJECT_0)
			{
				result = true;
			}		
		}
	}

	return result;
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


	/*
	auto syn_pos = std::find(buffer.begin(), buffer.end(), SYN);
	if (syn_pos == buffer.end())
	return false;
	buffer.erase(buffer.begin(), syn_pos);
	*/

	if (buffer.size() != PACKET_SIZE)
		return false;

	packetCounter++;

	return true;
}

BOOL Reception::parsePacket(Packet &packet, std::vector<BYTE> &buffer) {
	if (buffer[0] != SYN)
		return false;

	packet.data = std::string(buffer.begin() + 1, buffer.begin() + DATA_SIZE + 1);

	char crc[2];
	copy(buffer.begin() + DATA_SIZE + 1, buffer.end(), crc);
	packet.crc = 0;

	for (size_t i = 0; i < 2; ++i) {
		packet.crc += crc[i] << 8 * i;
	}

	return true;
}
BOOL Reception::validatePacket(Packet &packet) {
	return validateCRC(packet);
}
void Reception::errorStat(HWND handleDisplay) {
	errorCounter++;
}
void Reception::packetTimeout() {
	isPacketTimedOut = true;
};








void  Process::startProcess(HWND handleDisplayParam, std::string &data) {
	handleDisplay = handleDisplayParam;
	dataQueue.push(data);
	if (!isProcessing)
		processThread = CreateThread(NULL, 0, readCharacters, this, 0, &threadId);
}

DWORD WINAPI Process::readCharacters(LPVOID param) {
	Process* thisObj = (Process*)(param);

	thisObj->isProcessing = true;
	while (!thisObj->dataQueue.empty()) {
		std::string &data = thisObj->dataQueue.front();
		for (int i = 0; i < DATA_SIZE; i++) {
			if (!thisObj->handleChar(data[i]))
				break;
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
	cls();
}
void Process::failToSaveFile() {
	cls();
}

void Process::displayChar(char c) {
	HDC hdc; // handle for device contect
	SIZE sz; // size of charcter
	hdc = GetDC(handleDisplay);			 // get device context

	GetTextExtentPoint32(hdc, &c, 1, &sz); // get the size of current character.

	if (c == '\n' || char_x >= TEXTBOX_WIDTH - 20) { // if x position is past the window,
		char_y += sz.cy; // increse y position.
		char_x = 0; // set x position to leftmost
	}
	TextOut(hdc, char_x, char_y, &c, 1); // display character on window
	char_x += sz.cx; // increase x position by the current character's width

	ReleaseDC(handleDisplay, hdc); // Release device context
}
void Process::cls() {
	HDC hdc; // handle for device contect
	SIZE sz; // size of charcter
	hdc = GetDC(handleDisplay);			 // get device context
	int x = 0, y = 0;
	char space = ' ';
	while (y < TEXTBOX_HEIGTH) {
		while (x < TEXTBOX_WIDTH) {
			TextOut(hdc, x, y, &space, 1); // display character on window
			GetTextExtentPoint32(hdc, &space, 1, &sz); // get the size of current character.
			x += sz.cx;
		}
		GetTextExtentPoint32(hdc, &space, 1, &sz); // get the size of current character.
		y += sz.cy;
		x = 0;
	}
	ReleaseDC(handleDisplay, hdc); // Release device context
	char_x = char_y = 0;
}

void Reception::closeReceiption() {
	packetCounter = 0;
	errorCounter = 0;
	process.resetProcess();
}

void Process::resetProcess() {
	if (isProcessing && processThread != NULL)
		CloseHandle(processThread);
	processThread = NULL;
	isProcessing = false;
	dataQueue = {};
	writeBuffer.clear();
	cls();

}

