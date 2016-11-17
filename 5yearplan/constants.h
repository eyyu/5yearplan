#pragma once

static constexpr unsigned char NULL_BYTE = 0x00;
static constexpr unsigned char ENQ = 0x05;
static constexpr unsigned char ACK = 0x06;
static constexpr unsigned char SYN = 0x16;
static constexpr unsigned char DATA_SIZE = 1024;
static constexpr unsigned char CRC_SIZE = 2;
static constexpr unsigned char PACKET_SIZE = 1 + DATA_SIZE + CRC_SIZE;
static constexpr unsigned char BAUD_RATE = 9600;
