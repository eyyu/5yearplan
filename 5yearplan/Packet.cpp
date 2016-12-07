/*------------------------------------------------------------------------------
-- SOURCE FILE: packet.cpp - The packet defined in the protocol and its CRC methods
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
-- This source file contains implementations of the methods declared in packet.h
------------------------------------------------------------------------------*/

#include <string>
#include <cstdint>
#include <vector>
#include <sstream>
#include <iomanip>
#include "constants.h"
#include "packet.h"

/*--------------------------------------------------------------------------
-- FUNCTION: getOutputString
--
-- DATE: DEC. 6, 2016
--
-- REVISIONS:
-- Version 1.0 - [JA] - 2016/NOV/19 - created function
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- INTERFACE: std::string Packet::getOutputString() const;
--
-- NOTES:
-- This method returns a string suitable to be written to the COM port as
-- there is no native method of writing a struct to the port.
--------------------------------------------------------------------------*/
std::string Packet::getOutputString() const {
    std::string rtn;
    rtn.push_back(syn);
    rtn.append(data);
    rtn.push_back(crc >> 8);
    rtn.push_back(0x00FF & crc);
    return rtn;
}

/*--------------------------------------------------------------------------
-- FUNCTION: validateCRC
--
-- DATE: DEC. 6, 2016
--
-- REVISIONS:
-- Version 1.0 - [JA] - 2016/NOV/19 - created function
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- INTERFACE: bool validateCRC(const Packet& p);
--
-- NOTES:
-- This method validates crc by calling a overloaded version of this method which takes
-- the data string and crc separately.
--------------------------------------------------------------------------*/
bool validateCRC(const Packet& p) {
    return validateCRC(p.data, p.crc);
}

/*--------------------------------------------------------------------------
-- FUNCTION: validateCRC
--
-- DATE: DEC. 6, 2016
--
-- REVISIONS:
-- Version 1.0 - [JA] - 2016/NOV/19 - created function
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- INTERFACE: bool validateCRC(const std::string& data, const uint16_t crc);
--
-- NOTES:
-- This method validates crc by calculating the crc for the data and returning
-- whether or not the calculated crc is equal to the received one. The reason crc
-- validation is done this way instead of the traditional method of calculating
-- crc with the received value instead of 0's and checking for a zero syndrome
-- was because the crc generation was taken from online, and I don't understand it enough
-- to validate it any other way.
--------------------------------------------------------------------------*/
bool validateCRC(const std::string& data, const uint16_t crc) {
    return !(calculateCRC16(data) - crc);
}


/*--------------------------------------------------------------------------
-- FUNCTION: calculateCRC
--
-- DATE: DEC. 6, 2016
--
-- REVISIONS:
-- Version 1.0 - [JA] - 2016/NOV/19 - created function
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- INTERFACE: uint16_t calculateCRC16(const std::string& data);
--
-- NOTES:
-- This method vcalculates the crc for the given data.
-- THIS METHOD WAS TAKEN DIRECTLY FROM https://stackoverflow.com/questions/21939392/crc-16-program-to-calculate-check-sum
-- I have modified this method from that version to work with C++ strings
-- with dynamic lengths.
--------------------------------------------------------------------------*/
uint16_t calculateCRC16(const std::string& data) {
    static constexpr auto poly = 0x8005;
    auto size = data.size();
    uint16_t out = 0;
    int bits_read = 0;
    bool bit_flag;

    std::vector<char> bytes(data.begin(), data.end());

    int i = 0;
    while (size > 0) {
        bit_flag = (out >> 15) != 0;

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
        out = (out << 1) ^ (poly * ((out >> 15) != 0));
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