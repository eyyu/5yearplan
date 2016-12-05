
#include "reception.h"
#include "winmenu2.h"

using namespace receive;

BOOL Reception::start(HWND handleDisplay, HWND handleStat, HANDLE handleCom) {
	Packet packet;
	std::vector<char> buffer;
	//sendACK(handleCom);
	while (waitForPacket(handleCom)) {
		if (retrievePacket(handleCom, buffer)) {
			if (parsePacket(packet, buffer)) {
				if (validatePacket(packet)) {
					process.startProcess(handleDisplay, packet.data);
					sendACK(handleCom);
					return true;
				}
				else
					errorStat(handleDisplay);
			}
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

	OVERLAPPED osWrite = { 0 };
	bool result = true;
	// Create this writes OVERLAPPED structure hEvent.
	osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (osWrite.hEvent == NULL) {
		return;
	}

	if (!WriteFile(handleCom,&ch,1,&dwWritten,&osWrite))
	{
		if (GetLastError() == ERROR_IO_PENDING) {
			if (WaitForSingleObject(osWrite.hEvent, INFINITE) == WAIT_OBJECT_0)
			{
				GetOverlappedResult(handleCom, &osWrite, &dwWritten, TRUE);
			}
		}
	}

	CloseHandle(osWrite.hEvent);
}
BOOL Reception::waitForPacket(HANDLE handleCom) {
#if 1
	DWORD dwRead;
	char  chRead;
	OVERLAPPED osReader = { 0 };
	osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	bool result = false;

	// Retrieve the timeout parameters for all read and write operations
	// on the port. 
	COMMTIMEOUTS CommTimeouts;
	GetCommTimeouts(handleCom, &CommTimeouts);

	// Change the COMMTIMEOUTS structure settings.
	CommTimeouts.ReadIntervalTimeout = MAXDWORD;
	CommTimeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
	CommTimeouts.ReadTotalTimeoutConstant = RECEPTION_TIMEOUT;

	// Set the timeout parameters for all read and write operations
	// on the port. 
	if (!SetCommTimeouts(handleCom, &CommTimeouts))
	{
		return FALSE;
	}

	if (ReadFile(handleCom, &chRead, 1, &dwRead, &osReader)) {
		if (chRead == SYN)
			result = true;
		else if (chRead == ENQ)
			result = false;
	}
	else {
		if (GetLastError() == ERROR_IO_PENDING) {
			if (WaitForSingleObject(osReader.hEvent, RECEPTION_TIMEOUT) == WAIT_OBJECT_0) {
				if (GetOverlappedResult(handleCom, &osReader, &dwRead, FALSE))
					if (chRead == SYN)
						result = true;
					else if (chRead == ENQ)
						result = false;
			}
		}
	}

	CloseHandle(osReader.hEvent);

	return result;
#endif


#if 0
	OVERLAPPED Timeout_Event;
	memset((char *)&Timeout_Event, 0, sizeof(OVERLAPPED));
	Timeout_Event.hEvent = CreateEvent(NULL, TRUE, TRUE, 0);
	DWORD dwCommEvent;
	BOOL result = true;

	//   if (!SetCommMask(handleCom, EV_RXCHAR)) // Set event listener for RX_CHAR, occurs when a message comes from the port.
	//       MessageBox(NULL, "SetCommMask", "Error", MB_OK | MB_ICONINFORMATION);
	//while (1) {
	//	if (WaitCommEvent(handleCom, &dwCommEvent, &Timeout_Event))
	//	{ // wait for RXCHAR event.
	//		return true;
	//	}
	//	else
	//	{
	//		if (GetLastError() == ERROR_IO_PENDING)
	//		{
	//			if (WaitForSingleObject(Timeout_Event.hEvent, (DWORD)RECEPTION_TIMEOUT) == WAIT_OBJECT_0)
	//			{
	//				return true;
	//			}
	//			else {
	//				return false;
	//			}
	//		}
	//	}
	//}


	CloseHandle(Timeout_Event.hEvent);
	return result;
#endif
};

BOOL Reception::retrievePacket(HANDLE handleCom, std::vector<char> &buffer) {
	DWORD dwRead;
	char  tempBuffer[1026];
	buffer.clear();
	bool findSyn = false;
	OVERLAPPED osReader = { 0 };
	osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// Retrieve the timeout parameters for all read and write operations
	// on the port. 
	COMMTIMEOUTS CommTimeouts;
	GetCommTimeouts(handleCom, &CommTimeouts);

	// Change the COMMTIMEOUTS structure settings.
	//CommTimeouts.ReadIntervalTimeout = (RECEPTION_TIMEOUT / (PACKET_SIZE - 1))+1;
	//CommTimeouts.ReadTotalTimeoutMultiplier = 0;
	//CommTimeouts.ReadTotalTimeoutConstant = RECEPTION_TIMEOUT;

	CommTimeouts.ReadIntervalTimeout = RECEPTION_TIMEOUT;
	CommTimeouts.ReadTotalTimeoutMultiplier = 0;
	CommTimeouts.ReadTotalTimeoutConstant = RECEPTION_TIMEOUT;
	// Set the timeout parameters for all read and write operations
	// on the port. 
	if (!SetCommTimeouts(handleCom, &CommTimeouts))
	{
		return FALSE;
	}

	if (ReadFile(handleCom, tempBuffer, (PACKET_SIZE - 1) , &dwRead, &osReader))
		buffer.assign(tempBuffer, tempBuffer + (PACKET_SIZE - 1));
	else {
		if (GetLastError() == ERROR_IO_PENDING) {
			if (WaitForSingleObject(osReader.hEvent, INFINITE) == WAIT_OBJECT_0) {
				if (GetOverlappedResult(handleCom, &osReader, &dwRead, FALSE))
					buffer.assign(tempBuffer, tempBuffer + (PACKET_SIZE - 1));
			}
		}
	}

	/*
	auto syn_pos = std::find(buffer.begin(), buffer.end(), SYN);
	if (syn_pos == buffer.end())
	return false;
	buffer.erase(buffer.begin(), syn_pos);
	*/
	CloseHandle(osReader.hEvent);

	if (buffer.size() != (PACKET_SIZE - 1))
		return false;

	packetCounter++;

	return true;
}

BOOL Reception::parsePacket(Packet &packet, std::vector<char> &buffer) {
	packet.data = std::string(buffer.begin(), buffer.begin() + DATA_SIZE);

	char crc[2];
	copy(buffer.begin() + DATA_SIZE, buffer.end(), crc);

	packet.crc = crc[0] << 8;
	packet.crc += 0x00FF & crc[1];

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
		thisObj->displayChar();
		thisObj->dataQueue.pop();
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
		writeCharToBuffer(c);
		return true;
	}
	return false;
}
void Process::writeCharToBuffer(char c) {
	writeBuffer += c;
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

void Process::displayChar() {
	SetWindowText(GetDlgItem(handleDisplay, EDIT_RX), writeBuffer.c_str());
}
void Process::cls() {
	//HDC hdc; // handle for device contect
	//SIZE sz; // size of charcter
	//hdc = GetDC(handleDisplay);             // get device context
	//int x = 0, y = 0;
	//char space = ' ';
	//while (y < TEXTBOX_HEIGTH) {
	//    while (x < TEXTBOX_WIDTH) {
	//        TextOut(hdc, x, y, &space, 1); // display character on window
	//        GetTextExtentPoint32(hdc, &space, 1, &sz); // get the size of current character.
	//        x += sz.cx;
	//    }
	//    GetTextExtentPoint32(hdc, &space, 1, &sz); // get the size of current character.
	//    y += sz.cy;
	//    x = 0;
	//}
	//ReleaseDC(handleDisplay, hdc); // Release device context
	//char_x = char_y = 0;
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

