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
    const size_t packetNum = data.length() / DATA_SIZE;
    std::vector<std::string> dataChunks;

    for (size_t i = 0; i < packetNum; ++i) {
        dataChunks.emplace_back(data, i * DATA_SIZE, DATA_SIZE);
    }
    if (data.length() % DATA_SIZE) {
        std::string remaining = data.substr(packetNum * DATA_SIZE);
        dataChunks.push_back(remaining.append(DATA_SIZE - remaining.length(), NULL_BYTE));
    }
    if (addEmptyPacket) {
        dataChunks.emplace_back(DATA_SIZE, NULL_BYTE);
    }
    return dataChunks;
}

bool validateCRC(const Packet& p) {
    return transmit::validateCRC(p.data, p.crc);
}

bool validateCRC(const std::string& data, const uint_fast16_t crc) {
    //Temp until method is fleshed out
    return false;
}

/*
* Copied and modified from:
* https://stackoverflow.com/questions/21939392/crc-16-program-to-calculate-check-sum
*/
uint16_t calculateCRC16(const std::string& data) {
    static constexpr auto poly = 0x8005;
    auto size = data.size();
    uint16_t out = 0;
    int bits_read = 0;
    bool bit_flag;

    std::vector<char> bytes(data.begin(), data.end());

    int i = 0;
    while (size > 0) {
        bit_flag = out >> 15;

        /* Get next bit: */
        // item a) work from the least significant bits
        out = (out << 1) | ((bytes[i] >> bits_read) & 1);

        /* Increment bit counter: */
        if (++bits_read > 7) {
            bits_read = 0;
            i++;
            size--;
        }

        /* Cycle check: */
        if (bit_flag) {
            out ^= poly;
        }
    }

    // item b) "push out" the last 16 bits
    for (int i = 0; i < 16; ++i) {
        out = (out << 1) ^ (poly * static_cast<bool>(out >> 15));
    }

    // item c) reverse the bits
    uint16_t crc = 0;
    for (int i = 0x8000, j = 0x001; i; i >>= 1, j <<= 1) {
        if (i & out) {
            crc |= j;
        }
    }
    return crc;
}


