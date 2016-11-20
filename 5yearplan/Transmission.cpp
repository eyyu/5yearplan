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

void Transmitter::addDataToQueue(const std::string& data) {
    for (const auto& content : packetizeData(data)) {
        outputQueue.push(buildPacket(content));
    }
}

Packet Transmitter::buildPacket(const std::string & data) const {
    return Packet(data);
}

std::vector<std::string> Transmitter::packetizeData(const std::string& data) const {
    const size_t packetNum = data.length() / DATA_SIZE;
    std::vector<std::string> dataChunks;

    //Starting packet to be implemented when character is chosen
    //dataChunks.emplace_back(DATA_SIZE, START_BYTE);

    for (size_t i = 0; i < packetNum; ++i) {
        dataChunks.emplace_back(data, i * DATA_SIZE, DATA_SIZE);
    }
    if (data.length() % DATA_SIZE) {
        std::string remaining = data.substr(packetNum * DATA_SIZE);
        dataChunks.push_back(remaining.append(DATA_SIZE - remaining.length(), NULL_BYTE));
    }
    dataChunks.emplace_back(DATA_SIZE, NULL_BYTE);
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

void Transmitter::sendPacket(const HANDLE& hComm) {
    timeoutReached = false;
    OVERLAPPED over = {0};
    over.hEvent = CreateEvent(nullptr, false, false, nullptr);
    SetCommMask(hComm, EV_RXCHAR);

    if (outputQueue.empty()) {
        //Error, tried to send packet that doesn't exist
        throw std::runtime_error("Tried to send a packet from an empty queue");
    }

    std::string data = outputQueue.front().getOutputString();
    //Overlapped struct goes in last parameter to writefile call
    WriteFile(hComm, &data, data.size(), nullptr, &over);
    ackTimer.start();

    std::string buf;
    DWORD success;

    while (true) {
        while (!timeoutReached) {
            if (WaitCommEvent(hComm, nullptr, &over)) {
                ReadFile(hComm, &buf, 1, &success, &over);
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
            WriteFile(hComm, &data, data.size(), nullptr, &over);
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
