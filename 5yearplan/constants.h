#pragma once

static constexpr unsigned char NULL_BYTE = 0x00;
static constexpr unsigned char ENQ = 0x05;
static constexpr unsigned char ACK = 0x06;
static constexpr unsigned char SYN = 0x16;
static constexpr unsigned char DC1 = 0x11;
static constexpr int DATA_SIZE = 1024;
static constexpr int CRC_SIZE = 2;
static constexpr int PACKET_SIZE = 1 + DATA_SIZE + CRC_SIZE;
static constexpr int BAUD_RATE = 9600;
static constexpr int MAX_RETRIES = 200;

static constexpr unsigned long  RAN_TIMER_MIN = 0; // in ms
static constexpr unsigned long  RAN_TIMER_MAX = 200; // in ms
static constexpr unsigned long  IDLE_STATE_TIME = 500; //ms
static constexpr unsigned long  RECEPTION_TIMEOUT = (PACKET_SIZE / BAUD_RATE) * 3000;
static constexpr unsigned long  DISCONNECT_TIMEOUT = 5000;