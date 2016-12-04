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

void Transmitter::addDataToQueue(const std::string& data) {
    for (const auto& content : packetizeData(data)) {
        outputQueue.push(buildPacket(content));
    }
}

Packet Transmitter::buildPacket(const std::string & data) const {
    return Packet(data);
}

std::vector<std::string> Transmitter::packetizeData(const std::string& data) const {
    const size_t packetNum = (data.length() / DATA_SIZE);
    std::vector<std::string> dataChunks;

   // dataChunks.emplace_back(DATA_SIZE, DC1);

    std::string temp(data);

    while (1) {
        if (temp.size() < DATA_SIZE) {
            if (temp.length() % DATA_SIZE) {
                dataChunks.push_back(temp.append(DATA_SIZE - temp.length(), NULL_BYTE));
            } else {
                dataChunks.emplace_back(DATA_SIZE, NULL_BYTE);
            }
            break;
        } else {
            dataChunks.emplace_back(temp, 0, DATA_SIZE);
            temp.erase(0, DATA_SIZE);
        }
    }
    return dataChunks;
}

void Transmitter::addFileToQueue(const LPTSTR& filePath) {
    Transmitter::addFileToQueue(std::string(filePath));
}

void Transmitter::addFileToQueue(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) {
        //File opening error
        throw std::runtime_error("File cannot be opened");
    }
    std::string fileContents{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    Transmitter::addDataToQueue(fileContents);
}

void Transmitter::sendPacket(const HANDLE& commHandle) {
    timeoutReached = false;
    OVERLAPPED over = {0};
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
    DWORD dwWritten;
    bool result = false;

    // Create this writes OVERLAPPED structure hEvent.
    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    //if (!WriteFile(commHandle,
    //  data.c_str(),
    //  PACKET_SIZE,
    //  &dwWritten,
    //  &osWrite
    //))
    //{
    //  if (GetLastError() == ERROR_IO_PENDING) {
    //      WaitForSingleObject(osWrite.hEvent, INFINITE);
    //      //GetOverlappedResult(commHandle, &osWrite, &dwWritten, TRUE);
    //  }
    //}
    for (int i = 0; i < PACKET_SIZE; i++) {
        WriteFile(commHandle, &data[i], 1, &dwWritten, &osWrite);
    }
    ackTimer.start();

    std::string buf;
    DWORD success;

    while (true) {
        while (!timeoutReached) {
            if (WaitCommEvent(commHandle, nullptr, &over)) {
                ReadFile(commHandle, &buf, 1, &success, &over);
                if (success && buf.front() == ACK) {
                    ackTimer.stop();
                    break;
                }
            }
        }

        ackTimer.stop();

        if (timeoutReached) {
            retryCounter++;
            if (retryCounter > MAX_RETRIES) {
                closeTransmitter();
                return;
            }
            for (int i = 0; i < PACKET_SIZE; i++) {
                WriteFile(commHandle, &data[i], 1, &dwWritten, &osWrite);
            }
            ackTimer.start();
        } else {
            break;
        }
    }
    outputQueue.pop();
}

void Transmitter::closeTransmitter() {
    retryCounter = 0;
    while (!outputQueue.empty()) {
        outputQueue.pop();
    }
}


void Transmitter::ackTimeout() {
    timeoutReached = true;
}
