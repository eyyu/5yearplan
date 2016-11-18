#include <vector>
#include <string>
#include <cstdint>
#include <sstream>
#include <bitset>
#include <iostream>
#include <iterator>

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
