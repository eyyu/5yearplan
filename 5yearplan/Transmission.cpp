#include "timer.h"
#include "transmission.h"
#include <vector>
#include <string>
#include <cstdint>
#include <sstream>
#include <bitset>
#include <iostream>

using namespace transmit;

void Transmitter::addDataToQueue(const std::string& data) {
    std::vector<std::string> dataChunks = packetizeData(data);
    for (const auto& content : dataChunks) {
        outputQueue.push(buildPacket(content));
    }
}

Packet Transmitter::buildPacket(const std::string & data) const {
    return Packet(data);
}

std::vector<std::string> Transmitter::packetizeData(const std::string& data, const bool addEmptyPacket) const {
    size_t packetNum = data.length() >> 10;
    std::vector<std::string> dataChunks;

    for (size_t i = 0; i < packetNum; ++i) {
        dataChunks.emplace_back(data, i << 10, 1024);
    }
    if (data.length() % 1024) {
        std::string remaining = data.substr(packetNum << 10);
        dataChunks.push_back(remaining.append(1024 - remaining.length(), 0x00));
    }
    if (addEmptyPacket) {
        dataChunks.emplace_back(1024, 0x00);
    }
    return dataChunks;
}

bool validateCRC(const Packet& p) {
    return validateCRC(p.data, p.crc);
}

bool validateCRC(const std::string& data, const uint_fast16_t crc) {
    //Temp until method is fleshed out
    return false;
}

/*
TEMPORARY
DOES NOT WORK
*/
uint_fast16_t calculateCRC16(const std::string& data) {
    static constexpr unsigned long long poly = 0x8005;
    unsigned long long hex;

    std::istringstream iss(data);
    iss >> std::hex >> hex;

    std::bitset<64> bf(hex);

    std::cout << bf << std::endl;
    std::cout << hex << std::endl;
    hex <<= 15;

    bf = std::bitset<64>(hex);
    std::cout << bf << std::endl;

    const auto dataSize = sizeof(hex) << 3;
    uint_fast16_t crc;

    size_t firstSetBit = 0;

    std::bitset<64> bs(poly);

    std::cout << bs << std::endl;

    auto hexCopy = hex;

    while (hexCopy >>= 1) {
        ++firstSetBit;
    }

    std::cout << firstSetBit << std::endl;

    std::cout << dataSize << std::endl;

    for (size_t i = 0; i < firstSetBit - 1 && hex >> 16; ++i) {
        std::cout << std::endl << "Dat: " << std::bitset<64>(hex >> 15) << std::endl << std::endl;
        //Starts with 1
        if (hex & (1ULL << (firstSetBit - i))) {
            //XOR with the polynomial shifted by the size of the poly, and the current bit number
            std::cout << "Pre: " << std::bitset<64>(hex) << std::endl;
            std::cout << "Pat: " << std::bitset<64>(poly << (firstSetBit - 15 - i)) << std::endl;
            hex ^= (poly << (firstSetBit - 15 - i));
            crc = hex ^ 0x0000;
            std::cout << "Pos: " << std::bitset<64>(hex) << std::endl;
            std::cout << std::endl;
        }
    }

    std::cout << "Result: " << (hex << 1) << std::endl;

    if (hex ^ 0x0000) {
        //CRC worked???
        std::cout << "Test: " << (hex ^ 0x0000) << std::endl;
        crc = hex ^ 0x0000;
    } else {
        //CRC failed which should be impossible;
        crc = 0;
    }
    return crc;
}


