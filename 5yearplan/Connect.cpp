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
#include "winmenu2.h"

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
-- FUNCTION:  startIdleStateTimer
--
-- DATE: DEC.06, 2016
--
-- REVISIONS:
-- Version 1.0 - [EY] - 2016/DEC/06 - DESCRIPTION
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: void startIdleStateTimer (void)
--
--
-- NOTES:
-- starts A timer that runs on a random time between 0 - 100 ms
--------------------------------------------------------------------------*/
void startIdleStateTimer()
{
	idleStateTimer.start();
}

int count = 0;
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
		if (enqCount >= ENQ_MAX_RETRIES)
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
		SetWindowText(GetDlgItem(hwnd, PACK_SENT), std::to_string(0).c_str());
		SetWindowText(GetDlgItem(hwnd, ACK_RECD), std::to_string(0).c_str());
		SetWindowText(GetDlgItem(hwnd, TX_COMP), std::to_string(0).c_str());
		SetWindowText(GetDlgItem(hwnd, PACK_RECD), std::to_string(0).c_str());
		SetWindowText(GetDlgItem(hwnd, ACK_SENT), std::to_string(0).c_str());
		SetWindowText(GetDlgItem(hwnd, RX_COMP), std::to_string(0).c_str());
		SetWindowText(GetDlgItem(hwnd, RX_ERR), std::to_string(0).c_str());

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

	OVERLAPPED timeoutEvent = { 0 };
	OVERLAPPED osReader = { 0 };

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
				if (WaitForSingleObject(timeoutEvent.hEvent, (DWORD)BYTE_TIMEOUT) == WAIT_OBJECT_0)
				{
					if (GetOverlappedResult(hComm, &timeoutEvent, &nBytesRead, FALSE))
						timedout = false;
					else
						timedout = true;
				}
				else {
					timedout = true;

				}
			}
			else
			{
				timedout = true;
			}
		}
		else
		{
			timedout = false;
		}

		if (!timedout) {
			ClearCommError(hComm, &dwError, &cs);
			if ((dwEvent & EV_RXCHAR) && cs.cbInQue)
			{
				COMMTIMEOUTS CommTimeouts;
				GetCommTimeouts(hComm, &CommTimeouts);

				// Change the COMMTIMEOUTS structure settings.
				CommTimeouts.ReadIntervalTimeout = MAXDWORD;
				CommTimeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
				CommTimeouts.ReadTotalTimeoutConstant = BYTE_TIMEOUT;

				// Set the timeout parameters for all read and write operations
				// on the port. 
				SetCommTimeouts(hComm, &CommTimeouts);
				char chRead;
				bool boolRead = false;
				// should read in a loop? to eliminate all intial ENQS? 
				if (!ReadFile(hComm, &chRead, 1, &nBytesRead, &osReader))
				{
					if (GetLastError() == ERROR_IO_PENDING) {
						if (GetOverlappedResult(hComm, &osReader, &nBytesRead, TRUE)) {
							boolRead = true;
						}
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
							OutputDebugString("RECEIVED ACK \n");

							//waitingReceptionTimoutTimer.stop();
							randomEnqTimer.stop();
							idleStateTimer.stop();
							if (isWaitingForAck)
							{
								isWaitingForAck = false;
								enqCount = 0;
								if (TX.outGoingDataInBuffer()) {
									isWriting = true;
									TX.sendPacket(hDisplay, hComm);
									isWriting = false;
								}
								else
								{
									//waitingReceptionTimoutTimer.start();
									std::this_thread::sleep_for(std::chrono::milliseconds(TRANSMISSION_TIMEOUT * 2 + BYTE_TIMEOUT));
									idleStateTimer.start();

								}
							}

						}
						else if (chRead == ENQ)
						{
							OutputDebugString("RECEIVED ENQ \n");

							//waitingReceptionTimoutTimer.stop();
							randomEnqTimer.stop();
							idleStateTimer.stop();
							PurgeComm(hComm, PURGE_RXCLEAR);
							writeChar(ACK);
							if (isWaitingForPacket)
							{
								isReading = true;
								if (RX.start(hDisplay, hComm))
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
		MessageBox(NULL, "Disconnected!!!", "INFO", MB_OK);
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
			if (!GetOverlappedResult(hComm, &osWrite, &dwWritten, TRUE))
				result = false;
		}
	}

	if (result) {
		if (c == ACK) {
			isWaitingForPacket = true;
			OutputDebugString("SENT ACK");

		}
		else if (c == ENQ)
		{
			++enqCount;
			isWaitingForAck = true;
			char buff[100];
			sprintf_s(buff, "%s : %d\n", "Sent ENQ", ++count);
			OutputDebugString(buff);
		}
	}

	CloseHandle(osWrite.hEvent);
	return result;
}