#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <queue>
#include "timer.h"
#include "constants.h"
#include "packet.h"

namespace receive {
    static const DWORD TEXTBOX_HEIGTH = 200;
    static const DWORD TEXTBOX_WIDTH = 400;

    class Process {
    private:
        std::queue<std::string> dataQueue;
        std::vector<char> writeBuffer;
        BOOL isProcessing = false;
        HANDLE processThread;
        DWORD threadId;
        HWND handleDisplay;
        int char_x;
        int char_y;

        static DWORD WINAPI readCharacters(LPVOID params);
        BOOL handleChar(char c);
        void writeCharToBuffer(char c);
        void displayChar(char c);
        BOOL saveBufferToFile();
        void successSaveFile();
        void failToSaveFile();
        void cls();
    public:
        void resetProcess();
        void startProcess(HWND handleDisplayParam, std::string&);
    };

    class Reception {
    private:
        DWORD packetCounter;
        DWORD errorCounter;
        Process process;

        BOOL isPacketTimedOut = false;


        void sendACK(HANDLE handleCom);
        BOOL waitForPacket(HANDLE handleCom);
        BOOL retrievePacket(HANDLE handleCom, std::vector<BYTE> &buffer);
        BOOL parsePacket(Packet &packet, std::vector<BYTE> &buffer);
        BOOL validatePacket(Packet &packet);
        void errorStat(HWND handleStat);
        void packetTimeout();
    public:
        BOOL start(HWND handleDisplay, HWND handleStat, HANDLE handleCom);
        void closeReceiption();

    };

};