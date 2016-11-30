/*------------------------------------------------------------------------------
-- SOURCE FILE: Command.cpp - The COMMAND or IDLE state of the protocol
--
-- PROGRAM: 5YearPlan
--
-- FUNCTIONS:
--
-- DATE: Nov. 16, 2016
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

#include "Connect.h"

void startRandomEnqTimer()
{
    randomEnqTimer.start();
}

// event driven
void enqLine()
{
    //if (enqCount >= MAX_RETRIES)
    //{
    //    stopConnnection();
    //    return;
    //}
    writeChar(ENQ);
    return;
}

bool startConnnection(LPCTSTR commPortAddress, HWND hwnd)
{
    COMMCONFIG cc;
    GetDefaultCommConfig(commPortAddress, &cc, &cc.dwSize);
    cc.dcb.BaudRate = 9600;
    hComm = CreateFile(commPortAddress,
                       GENERIC_WRITE | GENERIC_READ,  // access ( write)
                       0,                             // (share) 0:cannot share the COM port
                       0,                             // security  (None)
                       OPEN_EXISTING,
                       FILE_FLAG_OVERLAPPED,   // we want overlapped operation
                       0                               // no templates file for COM port...
                       );
    //catch errors 
    if (hComm == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, "CANNOT OPEN COMM PORT ", "ERROR", MB_OK);
        return false;
    }
	if(CommConfigDialog(commPortAddress, hwnd, &cc))
		SetCommState(hComm, &cc.dcb);
    isConnected = true;
    connectedThread = std::thread(startConnectProc, hwnd, hwnd); // NULL to be replaces with stats Display!!
    connectedThread.detach(); // run connected threas in background
    return true;
}

bool startConnectProc(HWND hDisplay, HWND hwnd)
{
	OVERLAPPED timeoutEvent;
	memset((char *)&timeoutEvent, 0, sizeof(OVERLAPPED));
	timeoutEvent.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	char inBuff[PACKET_SIZE];
	DWORD nBytesRead, dwEvent, dwError;
	COMSTAT cs;

	OVERLAPPED osReader = { 0 };

	// Create the overlapped event. Must be closed before exiting
	// to avoid a handle leak.
	osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (osReader.hEvent == NULL) {
		return false;
	}

	if (!SetCommMask(hComm, EV_RXCHAR))
	{
		MessageBox(NULL, "Cannot Set Comm Mask", "ERROR", MB_OK);
		return false;
	}

	if (TX.outGoingDataInBuffer())
	{
		randomEnqTimer.start();
	}
	else
	{
		idleStateTimer.start();
	}
	bool timedout = false;
	while (isConnected)
	{
		if (!WaitCommEvent(hComm, &dwEvent, &timeoutEvent))
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				if (WaitForSingleObject(timeoutEvent.hEvent, (DWORD)DISCONNECT_TIMEOUT) == WAIT_OBJECT_0)
				{
					timedout = false;
				}
				else {
					timedout = true;

				}
				ResetEvent(timeoutEvent.hEvent);
			}else{
			}
			ClearCommError(hComm, &dwError, &cs);
		}
		else {
			timedout = false;
		}

		if (!timedout) {
			ClearCommError(hComm, &dwError, &cs);
			if ((dwEvent & EV_RXCHAR) && cs.cbInQue)
			{
				char chRead;
				bool boolRead = false;
				if (!ReadFile(hComm, &chRead, 1, &nBytesRead, &osReader))
				{
					if (GetLastError() == ERROR_IO_PENDING) {
						if (WaitForSingleObject(osReader.hEvent, INFINITE) == WAIT_OBJECT_0)
							if (GetOverlappedResult(hComm, &osReader, &nBytesRead, FALSE))
								boolRead = true;
					}else{
						boolRead = false;
					}
				}
				else
					boolRead = true;

				if (boolRead)
				{
					if (nBytesRead >= 1)
					{
						if (chRead == ACK || chRead == 'F')
						{
							if (isWaitingForAck)
							{
								idleStateTimer.stop();
								randomEnqTimer.stop();
								enqCount = 0;
								isWaitingForAck = false;
								if (TX.outGoingDataInBuffer()) {
									isWriting = true;
									TX.sendPacket(hComm);
								}
								else
								{
									idleStateTimer.start();
								}
							}
						}
						else if (chRead == ENQ
							&& !isReading
							&& !isWriting)
						{
							idleStateTimer.stop();
							randomEnqTimer.stop();
							writeChar(ACK);
							if (isWaitingForPacket)
							{
								if (packetCount < MAX_RETRIES
									&& RX.start(hDisplay, hwnd, hComm))
								{
									isWaitingForPacket = false;
									packetCount = 0;
								}
								else {
									++packetCount;
								}
							}
						}
					}
				}
			}
		}
	}
	CloseHandle(timeoutEvent.hEvent);
	CloseHandle(osReader.hEvent);

	PurgeComm(hComm, PURGE_RXCLEAR);
	return 0;
}

bool stopConnnection()
{
    if (isConnected)
    {
		idleStateTimer.stop();
		randomEnqTimer.stop();
        isConnected = false;
		Sleep(DISCONNECT_TIMEOUT);
        TX.closeTransmitter();
        RX.closeReceiption();
		resetDataValues();
        //connectedThread.join();
        PurgeComm(hComm, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);
        CloseHandle(hComm);
        return true;
    }

}

void resetDataValues()
{
    isWriting = false;
    packetCount = 0;
    isReading = false;
    isWaitingForPacket = false;
    isWaitingForAck = false;
    enqCount = 0;
}
bool sendNewFile(LPCSTR filePath)
{
    if (isConnected)
    {
        TX.addFileToQueue(filePath);
        enqLine();
        idleStateTimer.stop();
        randomEnqTimer.start();
        return true;
    }
    return false;
}

bool sendNewData(LPCSTR dataString)
{
    if (isConnected)
    {
        TX.addDataToQueue(dataString);
        idleStateTimer.stop();
        randomEnqTimer.start();
        return true;
    }
    return false;
}

bool writeChar(const char c)
{
	OVERLAPPED osWrite = { 0 };
	DWORD dwWritten;
	bool result = true;
	char ch = 'A';
	// Create this writes OVERLAPPED structure hEvent.
	osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (osWrite.hEvent == NULL) {
		return false;
	}
    if (!WriteFile(hComm,
                  &c,
                  1,
                  &dwWritten,
                  &osWrite
                  ))
    {
		if (GetLastError() != ERROR_IO_PENDING) {
			result = false;
		}
		else {
			if (WaitForSingleObject(osWrite.hEvent, INFINITE) == WAIT_OBJECT_0) {
				if (!GetOverlappedResult(hComm, &osWrite, &dwWritten, TRUE))
					result = false;
			}else result = false;


		}
	}/*else {
		result = true;
	}*/

	if (result) {
		if (c == ACK) {
			isWaitingForPacket = true;
		}
		else if (c == ENQ)
		{
			++enqCount;
			isWaitingForAck = true;
		}
	}
	CloseHandle(osWrite.hEvent);

    return result;
}