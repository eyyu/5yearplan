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
#pragma once

#include "Connect.h"

//using namespace connect;

/*Connection()
{
	/*enqCount = 0;
	isConnected = false;
	isReading = false;
	isWriting = false;
	isWaitingForPacket = false;
	isWaitingForAck = false;
	hComm = NULL;
	}*/


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

bool startConnnection( LPCTSTR commPortAddress , HWND hwnd)
{

    hComm = CreateFile( commPortAddress,
						GENERIC_WRITE | GENERIC_READ,  // access ( write)
						0,                             // (share) 0:cannot share the COM port
						0,                             // security  (None)
						OPEN_EXISTING,                 
						NULL,//FILE_FLAG_OVERLAPPED,   // we want overlapped operation
						0                               // no templates file for COM port...
						);
	//catch errors 
	if (hComm == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, "CANNOT OPEN COMM PORT ", "ERROR", MB_OK );
		return false;
	}

	isConnected = true;	
	connectedThread = std::thread(startConnectProc, hwnd , hwnd); // NULL to be replaces with stats Display!!
	connectedThread.detach(); // run connected threas in background
}

bool startConnectProc(HWND hDisplay, HWND hwnd)
{
	char inBuff[PACKET_SIZE];
	DWORD nBytesRead, dwEvent, dwError;
	COMSTAT cs;

	if (!SetCommMask(hComm, EV_RXCHAR))
	{
		MessageBox(NULL, "Cannot Set Comm Mask", "ERROR", MB_OK);
		return false;
	}
	
	while ( isConnected )
	{
		
		if (WaitCommEvent(hComm, &dwEvent, NULL))
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
					if (nBytesRead == 1)
					{
						if (inBuff[0] == ACK)
						{
							if (isWaitingForAck
								&& !isReading
								&& !isWriting)
							{
								resetEnqCount();
								isWaitingForAck = false;
								if (TX.outGoingDataInBuffer())
								{
									isWriting = true;
									TX.sendPacket(hComm);
								}
								else
								{
									idleStateTimer.start();
								}
							}
						}
						else if (inBuff[0] == ENQ
								 && !isReading
								 && !isWriting)
						{
							idleStateTimer.stop();
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
	}

	PurgeComm(hComm, PURGE_RXCLEAR);
	return 0;
}

bool stopConnnection()
{
	isConnected = false;
	TX.closeTransmitter();
	isWriting = false;
	packetCount = 0;
	//RX.closeReception();
	isReading = false;
	isWaitingForPacket = false;
	isWaitingForAck = false;
	WaitForSingleObject(hComm, 5000);
	resetEnqCount();
	PurgeComm(hComm, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);
	CloseHandle(hComm);
	return true;
}

bool sendNewFile(LPCSTR filePath)
{
	if (isConnected)
	{
		TX.addFileToQueue(filePath);
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

int getEnqCount(void)
{
	return enqCount;
}

void incrementEnqCount(void)
{
	++enqCount;
}

void resetEnqCount(void)
{
	enqCount = 0;
}

bool writeChar(const char c)
{
	DWORD bytesRead;
	char  writeBuff[] {c};

	 if ( WriteFile(hComm,
					 writeBuff,
					 sizeof(c),
					 &bytesRead,
					 NULL
					 ) ) 
	{
		if (c == ACK) {
			isWaitingForPacket = true;
			
		}
		else if (c == ENQ)
		{
			incrementEnqCount();
			isWaitingForAck = true;
		}
		return true;
	}
	 return false;
}