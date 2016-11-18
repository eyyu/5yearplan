#pragma once

#include <string>
#include <cstdint>
#include <queue>
#include <fstream>
#include <vector>
#include <windows.h>

#include "packet.h"
#include "constants.h"
#include "timer.h"

namespace transmit {
    class Transmitter {
        std::queue<Packet> outputQueue;
        std::fstream currentFile;
        int retryCounter = 0;

        void ackTimeout();
        typedef Timer<Transmitter, &ackTimeout, (PACKET_SIZE / BAUD_RATE) * 1000> AckTimer;
        AckTimer ackTimer;

        std::vector<std::string> packetizeData(const std::string& data, const bool addEmptyData = false) const;
        Packet buildPacket(const std::string& data) const;

    public:
        Transmitter() = default;
        void addDataToQueue(const std::string& data);
        void addFileToQueue(const LPTSTR& filePath);
        void addFileToQueue(const std::string& filePath);
        void sendPacket(const HANDLE& hComm);
        bool outGoingDataInBuffer() const { return !outputQueue.empty(); };
        void closeTransmitter();
    };
}