/*------------------------------------------------------------------------------
-- SOURCE FILE: .cpp - The COMMAND or IDLE state of the protocol
--
-- PROGRAM: [5YearPlan] 
--
-- FUNCTIONS:
-- *list all functions here!* 
-- void        addDataToQueue          (const std}::string& );
-- void        addFileToQueue          (const LPTSTR}& );
-- void        addFileToQueue          (const std}::string& );
-- void        sendPacket              (const HANDLE& );
-- bool        outGoingDataInBuffer    (void);
-- void        closeTransmitter        (void);
-- static void ackTimeout              (void);
-- Packet      buildPacket             (const striag) const;
--
-- DATE: Nov. 09, 2016
--
-- REVISIONS: 
-- Version 1.2.1.0 - [JA] - Nov. 09, 2016 - initial 
-- Version 2.0.0.0 - [JA] - Nov. 09, 2016 - adding the "DC1" file start function
--
-- DESIGNER: Tim Makimov & John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- NOTES:
-- This header holds the class declaration of the transmission object
-- the Tx Object deals with outgoing packages  
------------------------------------------------------------------------------*/
#pragma once

#include <string>
#include <cstdint>
#include <queue>
#include <fstream>
#include <vector>
#include <windows.h>
#include <atomic>

#include "packet.h"
#include "constants.h"
#include "timer.h"

namespace transmit {
    class Transmitter {
        static std::atomic_bool timeoutReached;
        static void ackTimeout();

        std::queue<Packet> outputQueue;
        int retryCounter = 0;

        typedef Timer<&ackTimeout, (PACKET_SIZE / BAUD_RATE) * 1000> AckTimer;
        AckTimer ackTimer;

        std::vector<std::string> packetizeData(const std::string& data) const;
        Packet buildPacket(const std::string& data) const;

    public:
        Transmitter() = default;
        void addDataToQueue(const std::string& data);
        void addFileToQueue(const LPTSTR& filePath);
        void addFileToQueue(const std::string& filePath);
        void sendPacket(const HANDLE& commHandle);
        bool outGoingDataInBuffer() const { return !outputQueue.empty(); };
        void closeTransmitter();
    };
}