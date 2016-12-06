/*------------------------------------------------------------------------------
-- SOURCE FILE: Transmission.cpp - transmits the packets
--
-- PROGRAM: 5yearplan
--
-- FUNCTIONS:
-- Transmitter                         (void); // CTOR
-- void        addDataToQueue          (const std::string& data);
-- void        addFileToQueue          (const LPTSTR& filePath);
-- void        addFileToQueue          (const std::string& filePath);
-- void        sendPacket              (const HANDLE& commHandle);
-- bool        outGoingDataInBuffer    (void);
-- void        closeTransmitter        (void);
-- static void ackTimeout              (void);
-- Packet      buildPacket             (const std::string& data) const;
--
-- DATE: NOV. 12, 2016
--
-- REVISIONS: 
-- Version 1.0 - [JA] - 2016/NOV/12 - Created Functions 

-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
------------------------------------------------------------------------------*/
#include <vector>
#include <string>
#include <cstdint>
#include <sstream>
#include <bitset>
#include <iostream>
#include <iterator>
#include <windows.h>

#include "timer.h"
#include "transmission.h"
#include "constants.h"
#include "packet.h"

using namespace transmit;

std::atomic_bool Transmitter::timeoutReached = false;

/*--------------------------------------------------------------------------
-- FUNCTION: addDataToQueue
--
-- DATE: DEC. 6, 2016
--
-- REVISIONS:
-- Version 1.0 - [JA] - 2016/NOV/19 - created function
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- INTERFACE: void Transmitter::addDataToQueue(const std::string& data);
--
-- NOTES:
-- This method calls packetizeData() and for each data chunk returned, it
-- creates the packet based on that data, and adding that new packet to the queue.
--------------------------------------------------------------------------*/
void Transmitter::addDataToQueue(const std::string& data) {
    for (const auto& content : packetizeData(data)) {
        outputQueue.push(buildPacket(content));
    }
}

/*--------------------------------------------------------------------------
-- FUNCTION: buildPacket
--
-- DATE: DEC. 6, 2016
--
-- REVISIONS:
-- Version 1.0 - [JA] - 2016/NOV/19 - created function
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- INTERFACE: Packet Transmitter::buildPacket(const std::string & data) const;
--
-- NOTES:
-- This method creates and returns a packet based on the data passed to it by
-- calling the packet constructor.
--------------------------------------------------------------------------*/
Packet Transmitter::buildPacket(const std::string & data) const {
    return Packet(data);
}

/*--------------------------------------------------------------------------
-- FUNCTION: packetizeData
--
-- DATE: DEC. 6, 2016
--
-- REVISIONS:
-- Version 1.0 - [JA] - 2016/NOV/19 - created function
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- INTERFACE: std::vector<std::string> Transmitter::packetizeData(const std::string& data) const;
--
-- NOTES:
-- This method takes a raw string of data and splits it into chunks equal to the packet data size.
-- It also handles the creation of the DC1 starting packet, as well as appending nulls or creating
-- the null packet based on the size of the data.
--------------------------------------------------------------------------*/
std::vector<std::string> Transmitter::packetizeData(const std::string& data) const {
    const size_t packetNum = (data.length() / DATA_SIZE);
    std::vector<std::string> dataChunks;

   // dataChunks.emplace_back(DATA_SIZE, DC1);

    std::string temp(data);

    while (1) {
        if (temp.size() < DATA_SIZE) {
            if (temp.length() % DATA_SIZE) {
                dataChunks.push_back(temp.append(DATA_SIZE - temp.length(), NULL_BYTE));
            }
            else {
                dataChunks.emplace_back(DATA_SIZE, NULL_BYTE);
            }
            break;
        }
        else {
            dataChunks.emplace_back(temp, 0, DATA_SIZE);
            temp.erase(0, DATA_SIZE);
        }
    }
    return dataChunks;
}

/*--------------------------------------------------------------------------
-- FUNCTION: addFileToQueue
--
-- DATE: DEC. 6, 2016
--
-- REVISIONS:
-- Version 1.0 - [JA] - 2016/NOV/19 - created function
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- INTERFACE: void Transmitter::addFileToQueue(const LPTSTR& filePath);
--
-- NOTES:
-- This method is an overloaded version of the addFileToQueue method that takes a string.
-- It creates a string temporary and calls the addFileToQueue method.
--------------------------------------------------------------------------*/
void Transmitter::addFileToQueue(const LPTSTR& filePath) {
    Transmitter::addFileToQueue(std::string(filePath));
}

/*--------------------------------------------------------------------------
-- FUNCTION: addFileToQueue
--
-- DATE: DEC. 6, 2016
--
-- REVISIONS:
-- Version 1.0 - [JA] - 2016/NOV/19 - created function
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- INTERFACE: void Transmitter::addFileToQueue(const std::string& filePath);
--
-- NOTES:
-- This method takes a string as a file path and reads in the entirety of the file
-- stored at that path, and adds that data to the queue.
--------------------------------------------------------------------------*/
void Transmitter::addFileToQueue(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) {
        //File opening error
        throw std::runtime_error("File cannot be opened");
    }
    std::string fileContents{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    Transmitter::addDataToQueue(fileContents);
}

/*--------------------------------------------------------------------------
-- FUNCTION: sendPacket
--
-- DATE: DEC. 6, 2016
--
-- REVISIONS:
-- Version 1.0 - [JA] - 2016/NOV/19 - created function
-- Version 1.1 - [TK] - 2016/DEC/4 - Changed ACK listening code
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- INTERFACE: void Transmitter::sendPacket(const HANDLE& commHandle);
--
-- NOTES:
-- This method sends the packet at the head of the output queue.
-- It is responsible for writing the packet data chaaracter by character to the
-- comm port, listening for the ACK response, resending if it doesnt receive one
-- and then removing that sent packet from the queue.
--------------------------------------------------------------------------*/
void Transmitter::sendPacket(const HANDLE& commHandle) {
    timeoutReached = false;
    OVERLAPPED over = { 0 };
    over.hEvent = CreateEvent(nullptr, false, false, nullptr);
    SetCommMask(commHandle, EV_RXCHAR);

    if (outputQueue.empty()) {
        //Error, tried to send packet that doesn't exist
        throw std::runtime_error("Tried to send a packet from an empty queue");
    }
    outputQueue.front().data;
    std::string data = outputQueue.front().getOutputString();
    //Overlapped struct goes in last parameter to writefile call

    OVERLAPPED osWrite = { 0 };
    OVERLAPPED osReader = { 0 };

    DWORD dwWritten;
    bool result = false;

    // Create this writes OVERLAPPED structure hEvent.
    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    //if (!WriteFile(commHandle,
    //    data.c_str(),
    //    PACKET_SIZE,
    //    &dwWritten,
    //    &osWrite
    //))
    //{
    //    if (GetLastError() == ERROR_IO_PENDING) {
    //        WaitForSingleObject(osWrite.hEvent, INFINITE);
    //        //GetOverlappedResult(commHandle, &osWrite, &dwWritten, TRUE);
    //    }
    //}
    for (int i = 0; i < PACKET_SIZE; i++) {
        WriteFile(commHandle, &data[i], 1, &dwWritten, &osWrite);
    }
    //ackTimer.start();

    std::string buf;
    DWORD success;
    DWORD test;
    DWORD dwEvent;
    char chRead;
    bool boolReadAck = false;
    while (1) {
        if (!WaitCommEvent(commHandle, &dwEvent, &over))
        {
            if (GetLastError() == ERROR_IO_PENDING)
            {
                if (WaitForSingleObject(over.hEvent, (DWORD)DISCONNECT_TIMEOUT) == WAIT_OBJECT_0)
                {
                    if (GetOverlappedResult(commHandle, &over, &test, FALSE)) {
                        if (!ReadFile(commHandle, &chRead, 1, &success, &osReader))
                        {
                            if (GetLastError() == ERROR_IO_PENDING) {
                                if (WaitForSingleObject(over.hEvent, (PACKET_SIZE / BAUD_RATE) * 1000) == WAIT_OBJECT_0)
                                    if (GetOverlappedResult(commHandle, &osReader, &success, FALSE))
                                        if (success && chRead == ACK) {
                                            boolReadAck = true;
                                        }
                            }
                        }
                        else
                            if (success && chRead == ACK) {
                                boolReadAck = true;
                            }
                    }
                }
            }
        }
        else {
            if (!ReadFile(commHandle, &chRead, 1, &success, &osReader))
            {
                if (GetLastError() == ERROR_IO_PENDING) {
                    if (WaitForSingleObject(over.hEvent, (PACKET_SIZE / BAUD_RATE) * 1000) == WAIT_OBJECT_0)
                        if (GetOverlappedResult(commHandle, &osReader, &success, FALSE))
                            if (success && chRead == ACK) {
                                boolReadAck = true;
                            }
                }
            }
            else
                if (success && chRead == ACK) {
                    boolReadAck = true;
                }
        }

        if (!boolReadAck) {
            retryCounter++;
            if (retryCounter > MAX_RETRIES) {
                closeTransmitter();
                return;
            }
            for (int i = 0; i < PACKET_SIZE; i++) {
                WriteFile(commHandle, &data[i], 1, &dwWritten, &osWrite);
            }
        }
        else {
            break;
        }
    }
    outputQueue.pop();

    //while (true) {
    //    while (!timeoutReached) {
    //        if (WaitCommEvent(commHandle, nullptr, &over)) {
    //            ReadFile(commHandle, &buf, 1, &success, &over);
    //            if (success && buf.front() == ACK) {
    //                ackTimer.stop();
    //                break;
    //            }
    //        }
    //    }

    //    ackTimer.stop();

    //    if (timeoutReached) {
    //        retryCounter++;
    //        if (retryCounter > MAX_RETRIES) {
    //            closeTransmitter();
    //            return;
    //        }
    //        for (int i = 0; i < PACKET_SIZE; i++) {
    //            WriteFile(commHandle, &data[i], 1, &dwWritten, &osWrite);
    //        }
    //        ackTimer.start();
    //    } else {
    //        break;
    //    }
    //}
    //outputQueue.pop();
}

/*--------------------------------------------------------------------------
-- FUNCTION: closeTransmitter
--
-- DATE: DEC. 6, 2016
--
-- REVISIONS:
-- Version 1.0 - [JA] - 2016/NOV/19 - created function
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- INTERFACE: void Transmitter::closeTransmitter();
--
-- NOTES:
-- This method closes down the transmitter by resetting its try counter and
-- emptying the trasmitter queue.
--------------------------------------------------------------------------*/
void Transmitter::closeTransmitter() {
    retryCounter = 0;
    std::queue<Packet>().swap(outputQueue);
}

/*--------------------------------------------------------------------------
-- FUNCTION: ackTimeout
--
-- DATE: DEC. 6, 2016
--
-- REVISIONS:
-- Version 1.0 - [JA] - 2016/NOV/19 - created function
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- INTERFACE: void Transmitter::ackTimeout();
--
-- NOTES:
-- This method is called by the AckTimer for the transmitter class and sets
-- an atomic bool to true, which indicates that the timeout has been reached.
--------------------------------------------------------------------------*/
void Transmitter::ackTimeout() {
    timeoutReached = true;
}
