/*------------------------------------------------------------------------------
-- SOURCE FILE: packet.h - The packet defined in the protocol
--
-- PROGRAM: 5yearplan
--
-- FUNCTIONS:
--                 Packet              (const std::string& data = std::string(DATA_SIZE, NULL_BYTE)) CTOR
-- uint16_t        calculateCRC16      (const std::string& data);
-- bool            validateCRC         (const Packet& p);
-- bool            validateCRC         (const std::string& data, const uint16_t crc);
-- std::string     getOutputString     () const;
-- 
-- DATE: Nov. 09, 2016
--
-- REVISIONS:
-- Version 1.2.1.0 - [JA] - Nov. 09, 2016 - initial
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- NOTES:
-- This header holds the struct definition of the data packets, as well as relevant
-- methods for CRC
------------------------------------------------------------------------------*/

#pragma once
#include <string>
#include <cstdint>
#include <iostream>
#include "constants.h"

struct Packet;

uint16_t calculateCRC16(const std::string& data);

bool validateCRC(const Packet& p);
bool validateCRC(const std::string& data, const uint16_t crc);


struct Packet {
    static constexpr unsigned char syn = SYN;
    std::string data;
    uint16_t crc;

    Packet(const std::string& data = std::string(DATA_SIZE, NULL_BYTE)) :
        data(data),
        crc(calculateCRC16(data)) {}

    std::string getOutputString() const;
};