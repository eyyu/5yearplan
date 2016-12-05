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

#include <thread>
#include <chrono>

#include "Connect.h"

/*--------------------------------------------------------------------------
-- FUNCTION: startRandomEnqTimer
--
-- DATE: NOV.19, 2016
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/NOV/19 - DESCRIPTION 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: void startRandomEnqTimer (void)
--
--
-- NOTES:
-- starts A timer that runs on a random time between 0 - 100 ms 
--------------------------------------------------------------------------*/
void startRandomEnqTimer()
{
    randomEnqTimer.start();
}

/*--------------------------------------------------------------------------
-- FUNCTION: enqLine
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/NOV/19 - created function 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: void enqLine (void)
--
-- NOTES:
-- this is called by the Random Enq timer to Ene the line
--------------------------------------------------------------------------*/
void enqLine()
{
	if (!isReading && !isWriting)
	{
		if (enqCount >= MAX_RETRIES)
		{
			stopConnnection();
			return;
		}
		writeChar(ENQ);
	}
    return;
}

/*--------------------------------------------------------------------------
-- FUNCTION: startConnection
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/NOV/19 - cretated function 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: bool startConnection (LPCSTR, HWND)
-- LPCSTR 	string representnf the address of the com port
-- HWND 	handle to the display window
--
-- RETURNS: 
--
-- NOTES:
-- this is called initiall when the user clicks "connect"
--------------------------------------------------------------------------*/
bool startConnnection(LPCTSTR commPortAddress, HWND hwnd)
{
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
	COMMCONFIG cc;
    GetDefaultCommConfig(commPortAddress, &cc, &cc.dwSize);
    cc.dcb.BaudRate = 9600;

	if (CommConfigDialog(commPortAddress, hwnd, &cc)) {
		SetCommState(hComm, &cc.dcb);
		isConnected = true;
		connectedThread = std::thread(startConnectProc, hwnd, hwnd); // NULL to be replaces with stats Display!!
		connectedThread.detach(); // run connected threas in background
		return true;
	}
	
}

/*--------------------------------------------------------------------------
-- FUNCTION: startConnectProc
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/NOV/19 - created event driven IO 
-- Version 2.0 - [TK] - 2016/NOV/28 - added overlapped features 
-- Version 2.0 - [EY] - 2016/NOV/28 - ensured proper flags and timer set
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: bool startConnectProc (functionParams)
-- HWND 	handle tothe main window	
-- HWND 	handle to the display window
--
-- RETURNS: 
-- a boolean value to inidicae success / failure 
--
-- NOTES:
-- NOTES
--------------------------------------------------------------------------*/
bool startConnectProc(HWND hDisplay, HWND hwnd)
{

	char inBuff[PACKET_SIZE];
	DWORD nBytesRead, dwEvent, dwError;
	COMSTAT cs;
	bool timedout = false;

	OVERLAPPED timeoutEvent;
	OVERLAPPED osReader = { 0 };

	memset((char *)&timeoutEvent, 0, sizeof(OVERLAPPED));
	timeoutEvent.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (osReader.hEvent == NULL || timeoutEvent.hEvent == NULL) {
		return false;
	}

	if (!SetCommMask(hComm, EV_RXCHAR))
	{
		MessageBox(NULL, "Cannot Set Comm Mask", "ERROR", MB_OK);
		return false;
	}

	//check buffer the first time before looping 
	if (TX.outGoingDataInBuffer())
	{
		randomEnqTimer.start();
	}
	else
	{
		idleStateTimer.start();
	}


	while (isConnected)
	{
		if (!WaitCommEvent(hComm, &dwEvent, &timeoutEvent))
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				if (WaitForSingleObject(timeoutEvent.hEvent, (DWORD)DISCONNECT_TIMEOUT) == WAIT_OBJECT_0)
				{
					if (GetOverlappedResult(hComm, &timeoutEvent, &nBytesRead, FALSE))
						timedout = false;
					else
						timedout = true;
				}
				else {
					timedout = true;

				}
				ResetEvent(timeoutEvent.hEvent);
			}
			else
			{
				//Anything in here? 
			}
			ClearCommError(hComm, &dwError, &cs);
		}
		else 
		{
			timedout = false;
		}

		if (!timedout) {
			ClearCommError(hComm, &dwError, &cs);
			if ((dwEvent & EV_RXCHAR) && cs.cbInQue)
			{
				char chRead;
				bool boolRead = false; 
				// should read in a loop? to eliminate all intial ENQS? 
				if (!ReadFile(hComm, &chRead, 1, &nBytesRead, &osReader))
				{
					if (GetLastError() == ERROR_IO_PENDING) {
						if (WaitForSingleObject(osReader.hEvent, INFINITE) == WAIT_OBJECT_0)
							if (GetOverlappedResult(hComm, &osReader, &nBytesRead, FALSE))
								boolRead = true;
					}
					else
					{
						boolRead = false;
					}
				}
				else
					boolRead = true;

				if (boolRead)
				{
					if (nBytesRead >= 1)
					{
						if (chRead == ACK)
						{
							randomEnqTimer.stop();
							idleStateTimer.stop();
							if (isWaitingForAck)
							{
								isWaitingForAck = false;
								enqCount = 0;
								if (TX.outGoingDataInBuffer()) {
									isWriting = true;
									TX.sendPacket(hComm);
									isWriting = false;
								}
								else
								{
									std::this_thread::sleep_for(std::chrono::milliseconds(RECEPTION_TIMEOUT));
									idleStateTimer.start();
								}
							}
							
						}
						//else if (chRead == ENQ)
						//{
						//	idleStateTimer.stop();
						//	randomEnqTimer.stop();
						//	writeChar(ACK);
						//	idleStateTimer.start();
						//}
						//else if (isWaitingForPacket)
						//{

						//	if (RX.start(hDisplay, hwnd, hComm))
						//	{	
						//		idleStateTimer.stop();
						//		isWaitingForPacket = false;
						//	}
						//}
						else if (chRead == ENQ)
						{
							randomEnqTimer.stop();
							idleStateTimer.stop();
							PurgeComm(hComm, PURGE_RXCLEAR);
							writeChar(ACK);
							if (isWaitingForPacket)
							{
								isReading = true;
								if (RX.start(hDisplay, hwnd, hComm))
								{
									isReading = false;
									isWaitingForPacket = false;
									packetCount = 0;

								}
								else
								{
									isReading = false;
									++packetCount;
								}
							}
							if (TX.outGoingDataInBuffer())
							{
								randomEnqTimer.start();
							}
							else
							{
								idleStateTimer.start();
							}
						}
					}
				}
			}
		}
	}

	idleStateTimer.stop();
	randomEnqTimer.stop();
	CloseHandle(timeoutEvent.hEvent);
	CloseHandle(osReader.hEvent);
	PurgeComm(hComm, PURGE_RXCLEAR);
	return 0;
}

/*--------------------------------------------------------------------------
-- FUNCTION: stopConnection
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/NOV/19 - creted function 
-- Version 1.5 - [EY] - 2016/NOV/19 - added sleep to ensure complete timeout 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: bool stopConnection (void)
--
-- RETURNS: 
-- boolean to indicate success / fail of program
--
-- NOTES:
-- called when port needs to close
--------------------------------------------------------------------------*/
bool stopConnnection()
{
    if (isConnected)
    {
		idleStateTimer.stop();
		//randomEnqTimer.stop();
        isConnected = false;
		Sleep(DISCONNECT_TIMEOUT);
        TX.closeTransmitter();
        RX.closeReceiption();
		resetDataValues();
        PurgeComm(hComm, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);
        CloseHandle(hComm);
        return true;
    }

}

/*--------------------------------------------------------------------------
-- FUNCTION: resetDataValues
--
-- DATE: NOV. 30, 2016
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/NOV/30 - created function 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: void resetDataValues (void)
--
-- RETURNS: 
--
-- NOTES:
-- called to reset any flags and values used inthe thread procedure
--------------------------------------------------------------------------*/
void resetDataValues()
{
    isWriting = false;
    packetCount = 0;
    isReading = false;
    isWaitingForPacket = false;
    isWaitingForAck = false;
    enqCount = 0;
}

/*--------------------------------------------------------------------------
-- FUNCTION: sendNewFile
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/NOV/19 - DESCRIPTION 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: bool sendNewFile (LPCSTR)
-- LPCSTR complete file path as string
--
-- RETURNS: 
--
-- NOTES:
-- calls transmitter object and passes new file to it. 
-- if another process is not going on right now, it will immediately start the enqTimer
--------------------------------------------------------------------------*/
bool sendNewFile(LPCSTR filePath)
{
    if (isConnected)
    {
        TX.addFileToQueue(filePath);
        if(!isReading
        	&& !isWriting)
        {
	        idleStateTimer.stop();
	        randomEnqTimer.start();
        }
        return true;
    }
    return false;
}

/*--------------------------------------------------------------------------
-- FUNCTION: sendNewData
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/NOV/19 - created function 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: bool sendNewData (LPCSTR)
-- LPCSTR the user typed data
--
-- RETURNS: 
--
-- NOTES:
-- this sends user typed data 
--------------------------------------------------------------------------*/
bool sendNewData(LPCSTR dataString)
{
    if (isConnected)
    {
        TX.addDataToQueue(dataString);
        if(!isReading
        	&& !isWriting)
        {
	        idleStateTimer.stop();
	        randomEnqTimer.start();
        }
        return true;
    }
    return false;
}

/*--------------------------------------------------------------------------
-- FUNCTION: writeChar
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/NOV/19 - created function 
-- Version 1.0 - [TK] - 2016/NOV/19 - added overlapped 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: bool writeChar (const char)
-- const char {10:desc}
--
-- RETURNS: 
--
-- NOTES:
-- writes an enq or ack char to line
--------------------------------------------------------------------------*/
bool writeChar(const char c)
{
	OVERLAPPED osWrite = { 0 };
	DWORD dwWritten;
	bool result = true;
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
		else 
		{
			if (WaitForSingleObject(osWrite.hEvent, INFINITE) == WAIT_OBJECT_0) 
			{
				if (!GetOverlappedResult(hComm, &osWrite, &dwWritten, TRUE))
					result = false;
			}
			else
			{
				result = false;
			}
		}
	}

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