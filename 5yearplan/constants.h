/*------------------------------------------------------------------------------
-- SOURCE FILE: N/A
--
-- PROGRAM: 5yearplan
--
-- DATE: DEC. 06, 2016
--
-- REVISIONS:

-- DESIGNER: Eva Yu, Terry Kang, John Agapeyev, Tim Makimov
--
-- PROGRAMMER: Terry Kang
--
-- NOTES:
-- Constants that are used by various classes
------------------------------------------------------------------------------*/

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
static constexpr int MAX_RETRIES = 2;
static constexpr int ENQ_MAX_RETRIES = 500;

static constexpr unsigned long  RAN_TIMER_MIN = 70; // in ms
static constexpr unsigned long  RAN_TIMER_MAX = 100; // in ms
static constexpr unsigned long  IDLE_STATE_TIME = 1000; //ms
static constexpr unsigned long	TRANSMISSION_TIMEOUT = 1000;
static constexpr unsigned long  RECEPTION_TIMEOUT = TRANSMISSION_TIMEOUT * 3;//2500*2;
static constexpr unsigned long  DISCONNECT_TIMEOUT = RECEPTION_TIMEOUT + IDLE_STATE_TIME;
static constexpr unsigned long  BYTE_TIMEOUT = 100;