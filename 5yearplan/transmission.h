/*------------------------------------------------------------------------------
-- SOURCE FILE: .cpp - The COMMAND or IDLE state of the protocol
--
-- PROGRAM: [5YearPlan] 
--
-- FUNCTIONS:
-- *list all functions here!* 
-- Transmitter                         (void); // CTOR
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
        static void ackTimeout();       //timeout to receive an ACK for packet sent

        std::queue<Packet> outputQueue;  
        int retryCounter = 0;           // dependent on MAX_RETRIES constant

        //typedef Timer<&ackTimeout, (PACKET_SIZE / BAUD_RATE) * 1000> AckTimer;
        typedef Timer<&ackTimeout, 858> AckTimer;
        AckTimer ackTimer; // Timer Object to mearure timeout 
                           // for ACK after sending packet  

        std::vector<std::string> packetizeData(const std::string& data) const;
        Packet buildPacket(const std::string& data) const;

    public:
        Transmitter() = default;
        void addDataToQueue(const std::string& data);     // data received as const a string 
        void addFileToQueue(const LPTSTR& filePath);      // path to data is receives. Stream needed to read
        void addFileToQueue(const std::string& filePath); // path to data is receives. Stream needed to read
        void sendPacket(const HANDLE& commHandle);        // Called by connect.  
        bool outGoingDataInBuffer() const { return !outputQueue.empty(); }; // allows for check of availability 
                                                                            // of outgoing data 
        void closeTransmitter(); 
    };
}