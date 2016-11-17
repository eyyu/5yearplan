#pragma once
#include <string>
#include <cstdint>
#include "constants.h"

struct Packet {
    static constexpr unsigned char syn = SYN;
    std::string data;
    uint16_t crc;

    Packet(const std::string& data = std::string(DATA_SIZE, NULL_BYTE)) :
        data(data),
        crc(calculateCRC16(data)) {}
};

uint16_t calculateCRC16(const std::string& data);

bool validateCRC(const Packet& p);
bool validateCRC(const std::string& data, const uint16_t crc);