/*------------------------------------------------------------------------------
-- SOURCE FILE: reception.cpp - the Rx Class and Process Class
--
-- PROGRAM: 5yearplan
--
-- FUNCTIONS:
--
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS:
-- Version 1.0 - [TK] - 2016/NOV/19 - Creted Functions
-- Version 2.0 - [TK] - 2016/DEC/06 - Modified Funtions
--
-- DESIGNER: Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- NOTES:
-- this is the defition for receptions class and Process Class
------------------------------------------------------------------------------*/
#include "reception.h"
#include "winmenu2.h"

using namespace receive;

BOOL Reception::isReceptionTimedOut = false;

/*--------------------------------------------------------------------------
-- FUNCTION: start
--
-- DATE: nov. 19, 2016
--
-- REVISIONS:
-- Version 1.0 - [TK] - 2016/Nov/19 - created function
-- Version 2.0 - [TK] - 2016/Dec/06 - added stats updates
--
-- DESIGNER: Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: BOOl start (HWND, HWND, HANDLE)
-- HWND
-- HWND
-- HANDLE
--
-- RETURNS: ture if packet succefully received and validated
--
-- NOTES:
-- The start process for recieveing a packet.
-- This process wait for packet that starts with SYN as first byte,
-- then retrieve the packets and validates. If success, start processing
-- the received packet on another thread.
--------------------------------------------------------------------------*/
BOOL Reception::start(HWND handleDisplay, HANDLE handleCom) {
	Packet packet;
	std::vector<char> buffer;

	while (waitForPacket(handleCom)) {
		receptionTimer.stop();
		OutputDebugString("RECEIVED SYN \n");
		if (retrievePacket(handleCom, buffer)) {
			if (parsePacket(packet, buffer)) {
				if (validatePacket(packet)) {
					packetCounter++;
					SetWindowText(GetDlgItem(handleDisplay, PACK_RECD), std::to_string(packetCounter).c_str());
					process.startProcess(handleDisplay, packet.data);
					sendACK(handleDisplay, handleCom);
					return true;
				}
				else
					errorStat(handleDisplay);
			}
		}
	}
	return false;
}

/*--------------------------------------------------------------------------
-- FUNCTION: sendACK
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS:
-- Version 1.0 - [TK] - 2016/NOV/19 - created function
-- Version 2.0 - [TK] - 2016/NOV/28 - added overlaped feature
-- Version 3.0 - [TK] - 2016/DEC/06 - added displaying ack counter stat.
--
-- DESIGNER: Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void sendACK (HANDLE)
-- HANDLE 	handltetoComm
--
-- RETURNS: void
--
-- NOTES:
-- This funtion is called when a packet is succefully received and then
-- send ackknoledment to notify the packet successfully send and received.
--------------------------------------------------------------------------*/
void Reception::sendACK(HWND handleDisplay, HANDLE handleCom) {
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

	if (!WriteFile(handleCom, &ch, 1, &dwWritten, &osWrite))
	{
		if (GetLastError() == ERROR_IO_PENDING) {
			if (WaitForSingleObject(osWrite.hEvent, INFINITE) == WAIT_OBJECT_0)
			{
				if (GetOverlappedResult(handleCom, &osWrite, &dwWritten, TRUE)) {
					ackCounter++;
					SetWindowText(GetDlgItem(handleDisplay, ACK_SENT), std::to_string(ackCounter).c_str());
				}
			}
		}
	}

	CloseHandle(osWrite.hEvent);
}

/*--------------------------------------------------------------------------
-- FUNCTION: sendACK
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS:
-- Version 1.0 - [TK] - 2016/NOV/19 - created function
-- Version 2.0 - [TK] - 2016/DEC/06 - added overlaped and timeout features
--
-- DESIGNER: Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void sendACK (HANDLE)
-- HANDLE 	handltetoComm
--
-- RETURNS: ture if SYN charcter is received
--
-- NOTES:
-- This funtion wait for SYN character which indicates a data packet is following.
--------------------------------------------------------------------------*/
BOOL Reception::waitForPacket(HANDLE handleCom) {
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
	CommTimeouts.ReadTotalTimeoutMultiplier = 0;
	CommTimeouts.ReadTotalTimeoutConstant = 0;

	// Set the timeout parameters for all read and write operations
	// on the port. 
	if (!SetCommTimeouts(handleCom, &CommTimeouts))
	{
		return FALSE;
	}
	isReceptionTimedOut = false;
	receptionTimer.start();
	while (!isReceptionTimedOut) {
		if (ReadFile(handleCom, &chRead, 1, &dwRead, &osReader)) {
			//char aaa[100];
			//sprintf_s(aaa, "Reception::waitForPacket - dwRead = %d , %02X \n", dwRead, chRead);
			//OutputDebugString(aaa);
			if (chRead == SYN)
				return true;
		}
		else {
			if (GetLastError() == ERROR_IO_PENDING) {
				if (GetOverlappedResult(handleCom, &osReader, &dwRead, TRUE)) {
					//char aaa[100];
					//sprintf_s(aaa, "Reception::waitForPacket - dwRead = %d , %02X \n", dwRead, chRead);
					//OutputDebugString(aaa);
					if (chRead == SYN)
						return true;
				}
			}
		}
	}

	CloseHandle(osReader.hEvent);

	return false;
};


/*--------------------------------------------------------------------------
-- FUNCTION: retrievePacket
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS:
-- Version 1.0 - [TK] - 2016/NOV/19 - created function
-- Version 2.0 - [TK] - 2016/DEC/06 - added overlaped and timeout features
--
-- DESIGNER: Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: BOOL retrievePacket (HANDLE, std::vector<char>)
-- HANDLE 	handltetoComm
--
-- RETURNS: ture if proper length of packet is received without timeout.
--
-- NOTES:
-- This funtion is called when SYN character is received and then
-- try to read a packet after SYN character.
-- if success, populates the passed buffer with the packet.
--------------------------------------------------------------------------*/
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

	CommTimeouts.ReadIntervalTimeout = BYTE_TIMEOUT;//BYTE_TIMEOUT;
	CommTimeouts.ReadTotalTimeoutMultiplier = 0;
	CommTimeouts.ReadTotalTimeoutConstant = TRANSMISSION_TIMEOUT * 2;
	// Set the timeout parameters for all read and write operations
	// on the port. 
	if (!SetCommTimeouts(handleCom, &CommTimeouts))
	{
		return FALSE;
	}

	if (ReadFile(handleCom, tempBuffer, (PACKET_SIZE - 1), &dwRead, &osReader))
		buffer.assign(tempBuffer, tempBuffer + (PACKET_SIZE - 1));
	else {
		if (GetLastError() == ERROR_IO_PENDING) {
			if (GetOverlappedResult(handleCom, &osReader, &dwRead, TRUE))
				buffer.assign(tempBuffer, tempBuffer + (PACKET_SIZE - 1));
		}
	}

	CloseHandle(osReader.hEvent);

	if (dwRead != (PACKET_SIZE - 1))
		return false;

	return true;
}


/*--------------------------------------------------------------------------
-- FUNCTION: parsePacket
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS:
-- Version 1.0 - [TK] - 2016/NOV/19 - created function
--
-- DESIGNER: Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: BOOL parsePacket (Packet&, std::vector<char>)
--
-- RETURNS: ture if parsing is success.
--
-- NOTES:
--  This function parese the received packet in the passed buffer into
--  the predefined packet fomat (SYN,data,crc).
--------------------------------------------------------------------------*/
BOOL Reception::parsePacket(Packet &packet, std::vector<char> &buffer) {
	packet.data = std::string(buffer.begin(), buffer.begin() + DATA_SIZE);

	char crc[2];
	copy(buffer.begin() + DATA_SIZE, buffer.end(), crc);

	packet.crc = crc[0] << 8;
	packet.crc += 0x00FF & crc[1];

	return true;
}

/*--------------------------------------------------------------------------
-- FUNCTION: validatePacket
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS:
-- Version 1.0 - [TK] - 2016/NOV/19 - created function
--
-- DESIGNER: Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: BOOL validatePacket (Packet&)
--
-- RETURNS: ture if CRC is valid.
--
-- NOTES:
--  This fuction called validateCRC funtion to check the received packet
--  is valid.
--------------------------------------------------------------------------*/
BOOL Reception::validatePacket(Packet &packet) {
	return validateCRC(packet);
}

/*--------------------------------------------------------------------------
-- FUNCTION: errorStat
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS:
-- Version 1.0 - [TK] - 2016/NOV/19 - created function
--
-- DESIGNER: Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: BOOL errorStat (HWND)
--
-- RETURNS: void
--
-- NOTES:
--  This function is called when the received packet is invalid and
--  updates error stat diplay on the windows
--------------------------------------------------------------------------*/
void Reception::errorStat(HWND handleDisplay) {
	errorCounter++;
	SetWindowText(GetDlgItem(handleDisplay, RX_ERR), std::to_string(errorCounter).c_str());

}

/*--------------------------------------------------------------------------
-- FUNCTION: startProcess
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS:
-- Version 1.0 - [TK] - 2016/NOV/19 - created function
--
-- DESIGNER: Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void startProcess (HWND,  std::string&)
--
-- RETURNS: void
--
-- NOTES:
--  The start process for processing the received packet on different thread.
--  This function is called when a packet is successfully received.
--  Insert the received data into processing queue and  if no processing is running,
--  create another thread to process the data for displaying and saving it into a file.
--------------------------------------------------------------------------*/
void  Process::startProcess(HWND handleDisplayParam, std::string &data) {
	handleDisplay = handleDisplayParam;
	dataQueue.push(data);
	SetWindowText(GetDlgItem(handleDisplay, RX_COMP), std::to_string(completedFileCounter).c_str());
	if (!isProcessing)
		processThread = CreateThread(NULL, 0, readCharacters, this, 0, &threadId);
}

/*--------------------------------------------------------------------------
-- FUNCTION: readCharacters
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS:
-- Version 1.0 - [TK] - 2016/NOV/19 - created function
-- Version 2.0 - [TK] - 2016/DEC/06 - added regex for string to display new line.
--
-- DESIGNER: Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: DWORD WINAPI readCharacters(LPVOID param)
--
-- RETURNS: ture if proper length of packet is received without timeout.
--
-- NOTES:
-- This is a thread for proccessing received data.
-- This retreive the data from the queue and process it character by character.
--------------------------------------------------------------------------*/
DWORD WINAPI Process::readCharacters(LPVOID param) {
	Process* thisObj = (Process*)(param);

	thisObj->isProcessing = true;
	while (!thisObj->dataQueue.empty()) {
		std::string &data = thisObj->dataQueue.front();
		std::regex e("(?!\r)\n");
		data = std::regex_replace(data, e, "\r\n");
		for (int i = 0; i < DATA_SIZE; i++) {
			if (!thisObj->handleChar(data[i]))
				break;
		}
		thisObj->displayBuffer();
		thisObj->dataQueue.pop();
	}
	thisObj->isProcessing = false;
	CloseHandle(thisObj->processThread);
	return 0;
}

/*--------------------------------------------------------------------------
-- FUNCTION: handleChar
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS:
-- Version 1.0 - [TK] - 2016/NOV/19 - created function
-- Version 2.0 - [TK] - 2016/DEC/06 - added handling of DC1 charcter
--
-- DESIGNER: Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: BOOL handleChar(char c)
--
-- RETURNS: ture if the character is not NULL or DC1. 
--
-- NOTES:
-- This function reads the data character by character and handle it for protocol.
-- If the charcter is NULL, we assume this data is the last data of the file, then save the buffer into a file.
-- If the character is DC1, we assume the following data is the first data of the file, then clear buffer.
-- Otherwise, save the character into the buffer.
--------------------------------------------------------------------------*/
BOOL Process::handleChar(char c) {
	if (c == NULL_BYTE) {
		if (saveBufferToFile())
			successSaveFile();
		else
			failToSaveFile();
		completedFileCounter++;
		SetWindowText(GetDlgItem(handleDisplay, RX_COMP), std::to_string(completedFileCounter).c_str());
	}
	else if (c == DC1) {
		writeBuffer.clear();
	}
	else {
		writeCharToBuffer(c);
		return true;
	}
	return false;
}

/*--------------------------------------------------------------------------
-- FUNCTION: writeCharToBuffer
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS:
-- Version 1.0 - [TK] - 2016/NOV/19 - created function
--
-- DESIGNER: Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void writeCharToBuffer(char c)
--
-- RETURNS: void
--
-- NOTES:
-- This function save the character into the buffer.
--------------------------------------------------------------------------*/
void Process::writeCharToBuffer(char c) {
	writeBuffer += c;
}

/*--------------------------------------------------------------------------
-- FUNCTION: saveBufferToFile
--
-- DATE: DEC. 06, 2016
--
-- REVISIONS:
-- Version 1.0 - [TK] - 2016/DEC/06 - created function
--
-- DESIGNER: Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: BOOL saveBufferToFile()
--
-- RETURNS: true if file succefully saved.
--
-- NOTES:
-- This function save the data from buffer into a file and then clear the buffer.
--------------------------------------------------------------------------*/
BOOL Process::saveBufferToFile() {
	HANDLE hFile;
	DWORD wmWritten;

	SYSTEMTIME stNow;
	GetSystemTime(&stNow);

	char currentTime[84] = "";

	sprintf_s(currentTime, "%d%d%d_%d%d%d", stNow.wDay, stNow.wMonth, stNow.wYear,
		stNow.wHour, stNow.wMinute, stNow.wSecond);

	std::string fileName = "C:\\Users\\Administrator\\Desktop\\5yearplan_" + std::string(currentTime) + ".txt";
	OutputDebugString(fileName.c_str());
	hFile = CreateFile(fileName.c_str(), GENERIC_WRITE,
		FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!WriteFile(hFile, writeBuffer.c_str(), writeBuffer.size(), &wmWritten, NULL))
		return false;
	CloseHandle(hFile);
	writeBuffer.clear();
	return true;
}

/*--------------------------------------------------------------------------
-- FUNCTION: successSaveFile
--
-- DATE: DEC. 06, 2016
--
-- REVISIONS:
-- Version 1.0 - [TK] - 2016/DEC/06 - created function
--
-- DESIGNER: Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void successSaveFile()
--
-- RETURNS: void
--
-- NOTES:
-- This function is called when the file is succefully saved
-- and pop up a message box for user to be notified the file is saved.
--------------------------------------------------------------------------*/
void Process::successSaveFile() {
	MessageBox(NULL, "Succefully file saved!!", "", MB_OK);
	cls();
}

/*--------------------------------------------------------------------------
-- FUNCTION: failToSaveFile
--
-- DATE: DEC. 06, 2016
--
-- REVISIONS:
-- Version 1.0 - [TK] - 2016/DEC/06 - created function
--
-- DESIGNER: Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void failToSaveFile()
--
-- RETURNS: void
--
-- NOTES:
-- This function is called when saving file is failed
-- and pop up a message box for user to be notified the saving file is failed.
--------------------------------------------------------------------------*/
void Process::failToSaveFile() {
	MessageBox(NULL, "Saving file failed!!", "", MB_OK);
	cls();
}
/*--------------------------------------------------------------------------
-- FUNCTION: displayBuffer
--
-- DATE: DEC. 06, 2016
--
-- REVISIONS:
-- Version 1.0 - [TK] - 2016/DEC/06 - created function
--
-- DESIGNER: Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void displayBuffer()
--
-- RETURNS: void
--
-- NOTES:
-- This function displays the data from buffer on the screen.
--------------------------------------------------------------------------*/
void Process::displayBuffer() {
	SetWindowText(GetDlgItem(handleDisplay, EDIT_RX), writeBuffer.c_str());
}

/*--------------------------------------------------------------------------
-- FUNCTION: cls
--
-- DATE: DEC. 06, 2016
--
-- REVISIONS:
-- Version 1.0 - [TK] - 2016/DEC/06 - created function
--
-- DESIGNER: Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void cls()
--
-- RETURNS: void
--
-- NOTES:
-- This function clear the screen.
--------------------------------------------------------------------------*/
void Process::cls() {
	SetWindowText(GetDlgItem(handleDisplay, EDIT_RX), "");
}

/*--------------------------------------------------------------------------
-- FUNCTION: closeReceiption
--
-- DATE: DEC. 06, 2016
--
-- REVISIONS:
-- Version 1.0 - [TK] - 2016/DEC/06 - created function
--
-- DESIGNER: Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void closeReceiption()
--
-- RETURNS: void
--
-- NOTES:
-- This function is called when closing reception.
-- This resets the counter values and also stop the processing if running.
--------------------------------------------------------------------------*/
void Reception::closeReceiption() {
	packetCounter = 0;
	errorCounter = 0;
	ackCounter = 0;
	process.resetProcess();
}

/*--------------------------------------------------------------------------
-- FUNCTION: resetProcess
--
-- DATE: DEC. 06, 2016
--
-- REVISIONS:
-- Version 1.0 - [TK] - 2016/DEC/06 - created function
--
-- DESIGNER: Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void resetProcess()
--
-- RETURNS: void
--
-- NOTES:
-- This function is called when closing reception.
-- This resets the counter values and also stop the processing if running.
--------------------------------------------------------------------------*/
void Process::resetProcess() {
	if (isProcessing && processThread != NULL)
		CloseHandle(processThread);
	processThread = NULL;
	isProcessing = false;
	dataQueue = {};
	writeBuffer.clear();
	completedFileCounter = 0;

	SetWindowText(GetDlgItem(handleDisplay, PACK_RECD), 0);
	SetWindowText(GetDlgItem(handleDisplay, ACK_SENT), 0);
	SetWindowText(GetDlgItem(handleDisplay, RX_COMP), 0);
	cls();
}

/*--------------------------------------------------------------------------
-- FUNCTION: packetTimeout
--
-- DATE: DEC. 06, 2016
--
-- REVISIONS:
-- Version 1.0 - [TK] - 2016/DEC/06 - created function
--
-- DESIGNER: Terry Kang
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void packetTimeout()
--
-- RETURNS: void
--
-- NOTES:
-- This function is static function that is fired when the reception timer is timed out.
-- Set isPacketTimedOut flage to be true so that the waitForPacket is stopped.
--------------------------------------------------------------------------*/
void Reception::receptionTimeout() {
	isReceptionTimedOut = true;
}