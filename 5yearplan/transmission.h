#pragma once

#include "timer.h"

#include <string>
#include <cstdint>
#include <queue>
#include <fstream>
#include <vector>
#include <windows.h>

struct Packet {
    static constexpr unsigned char syn = 0x16;
    std::string data;
    uint_fast16_t crc;
    
    Packet();
    Packet(const std::string& data);
};

class Transmitter {
    std::queue<Packet> outputQueue;
    std::fstream currentFile;

    void ackTimeout();
    typedef Timer<Transmitter, &ackTimeout, (1027 / 9600)> AckTimer;
    AckTimer ackTimer;

    std::vector<std::string> packetizeData(const std::string& data) const;
    Packet buildPacket(const std::string& data) const;

public:
    Transmitter();
    void addDataToQueue(const std::string& data);
    void addFileToQueue(const LPTSTR& filePath);
    void addFileToQueue(const std::string& filePath);
    void sendPacket(const Packet& p);
    bool outGoingDataInBuffer() const { return !outputQueue.empty();};

};

uint16_t calculateCRC(const std::string& data);

bool validateCRC(const Packet& p);
bool validateCRC(const std::string data, const uint_fast16_t crc);

