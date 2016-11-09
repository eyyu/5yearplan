#pragma once

#include "timer.h"

#include <string>
#include <cstdint>
#include <queue>
#include <fstream>
#include <vector>
#include <windows.h>

namespace transmit {
    static constexpr unsigned char NULL_BYTE = 0x00;
    static constexpr unsigned char SYN = 0x16;
    static constexpr unsigned char DATA_SIZE = 1024;
    static constexpr unsigned char CRC_SIZE = 2;
    static constexpr unsigned char BAUD_RATE = 9600;

    struct Packet { 
        static constexpr unsigned char syn = SYN;
        std::string data;
        uint_fast16_t crc;

        Packet(const std::string& data = std::string(1024, NULL_BYTE)) : data(data), crc(calculateCRC16(data)) {}
    };

    class Transmitter {
        std::queue<Packet> outputQueue;
        std::fstream currentFile;

        void ackTimeout();
        typedef Timer<Transmitter, &ackTimeout, (DATA_SIZE + CRC_SIZE + 1 / BAUD_RATE)> AckTimer;
        AckTimer ackTimer;

        std::vector<std::string> packetizeData(const std::string& data, const bool addEmptyData = false) const;
        Packet buildPacket(const std::string& data) const;

    public:
        Transmitter() = default;
        void addDataToQueue(const std::string& data);
        void addFileToQueue(const LPTSTR& filePath);
        void addFileToQueue(const std::string& filePath);
        void sendPacket(const Packet& p);
        bool outGoingDataInBuffer() const { return !outputQueue.empty(); };
    };

    uint_fast16_t calculateCRC16(const std::string& data);

    bool validateCRC(const Packet& p);
    bool validateCRC(const std::string& data, const uint_fast16_t crc);

}