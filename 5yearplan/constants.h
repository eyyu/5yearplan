#pragma once

static constexpr unsigned char NULL_BYTE = 0x00;
static constexpr unsigned char SYN = 0x16;
static constexpr unsigned char DATA_SIZE = 1024;
static constexpr unsigned char CRC_SIZE = 2;
static constexpr unsigned char BAUD_RATE = 9600;

struct Packet { 
    static constexpr unsigned char syn = SYN;
    std::string data;
    uint16_t crc;

    Packet(const std::string& data = std::string(1024, NULL_BYTE)) : 
        data(data), 
        crc(calculateCRC16(data)) {}
};
