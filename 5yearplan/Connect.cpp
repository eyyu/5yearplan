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
//
//enqCount = 0;
//isConnected = false;
//isReading = false;
//isWriting = false;
//isWaitingForPacket = false;
//isWaitingForAck = false;
//packetCount = 0;

void startRandomEnqTimer()
{
	randomEnqTimer.start();
}

// event driven
void enqLine()
{
	if (enqCount >= MAX_RETRIES)
	{
		stopConnnection();
		return;
	}
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
	timeoutEvent.hEvent = CreateEvent(NULL, true, true, 0);

	char inBuff[PACKET_SIZE];
	DWORD nBytesRead, dwEvent, dwError;
	COMSTAT cs;

	if (!SetCommMask(hComm, EV_RXCHAR))
	{
		MessageBox(NULL, "Cannot Set Comm Mask", "ERROR", MB_OK);
		return false;
	}

	if (TX.outGoingDataInBuffer())
	{
		//randomEnqTimer.start();
	}
	else
	{
		enqLine();
		//idleStateTimer.start();
	}
	while (isConnected)
	{
		if (WaitCommEvent(hComm, &dwEvent, &timeoutEvent))
		{
			ClearCommError(hComm, &dwError, &cs);
			if ((dwEvent & EV_RXCHAR) && cs.cbInQue)
			{
				if (!ReadFile(hComm, inBuff, cs.cbInQue, &nBytesRead, NULL))
				{
					MessageBox(NULL, "Error reading from com port", "ERROR", MB_OK);
				}
				else
				{
					if (nBytesRead >= 1)
					{
						if (inBuff[0] == ACK)
						{
							if (1)
							{
								enqCount = 0;
								isWaitingForAck = false;
								while (1)
								{
									if (TX.outGoingDataInBuffer()) {
										isWriting = true;
										TX.sendPacket(hComm);
									}

								}
								//else
								{
									// idleStateTimer.start();
								}
							}
						}
						else if (inBuff[0] == ENQ
								 && !isReading
								 && !isWriting)
						{
							//idleStateTimer.stop();
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
			else
			{
				MessageBox(NULL, "Error detecting event in com port", "ERROR", MB_OK);
			}
		}
		else
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				if (WaitForSingleObject(timeoutEvent.hEvent, (DWORD)DISCONNECT_TIMEOUT) != WAIT_OBJECT_0)
				{
					stopConnnection();
				}
			}
		}
	}

	PurgeComm(hComm, PURGE_RXCLEAR);
	return 0;
}

bool stopConnnection()
{
	if (isConnected)
	{
		isConnected = false;
		TX.closeTransmitter();
		RX.closeReceiption();


		connectedThread.join();
		Sleep(5000);

		PurgeComm(hComm, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);
		CloseHandle(hComm);
		return true;
	}

}

void resetValues()
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
	DWORD bytesRead;

	if (WriteFile(hComm,
				  &c,
				  sizeof(c),
				  &bytesRead,
				  NULL
				  ))
	{
		if (c == ACK) {
			isWaitingForPacket = true;

		}
		else if (c == ENQ)
		{
			++enqCount;
			isWaitingForAck = true;
		}
		return true;
	}
	return false;
}