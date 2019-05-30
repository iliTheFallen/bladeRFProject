/*
 * Transmitter.cpp
 *
 *  Created on: Apr 11, 2019
 *      Author: Ilker GURCAN, Fehime Betul CAVDARLI, Umay Ezgi KADAN
 *
 */

#include <unistd.h>  //Read/Write to/from fd
#include <fcntl.h>
#include <signal.h>  //Listening to signal exit
#include <netinet/ip.h>
#include <iostream>
#include <vector>
#include <algorithm>  //std::rand_shuffle
#include <cstring> 
#include <ctime>  //std::time
#include <cstdlib>  //std::rand, std::srand
#include <boost/thread.hpp>

#include "TxRx.h"
#include "Scheduler.h"
#include "GlobalConfig.h"

#define MEASURING_BER 0
#define MEASURING_PCC 1  // Packet count changing size

typedef std::vector<int> ClientIds;

volatile sig_atomic_t exitRequested = 0;
static TxRx _txRx[NUM_TRANSMITS];
static ClientIds _clientIds;

void exitHandler(int s);
void sendMessage(TxRx& txRx, unsigned char* buf);  //App-to-Network
int myRandom(int i);
int initialize();

int main(int argc, char * argv[])
{
    //
    // Connect to all pre-configured tun interfaces.
    // Connect to pre-configured bladeRF TX device.
    //
    int errCode = initialize();
    if(errCode != 0)
    {
        return -1;
    }
    //
    // Register interactive event handler --> Ctrl-C
    //
    struct sigaction intAct;
    intAct.sa_handler = exitHandler;
    intAct.sa_flags   = 0;
    sigemptyset(&intAct.sa_mask);
    sigaction(SIGINT, &intAct, NULL);
    //
    // Allocate buffer
    //
    unsigned char buf[PAYLOAD_LEN] = {0};
    ClientIds::iterator clIt;
    //
    // Main Loop
    //
    startScheduler();
    while(!exitRequested)
    {
        for(clIt = _clientIds.begin(); clIt != _clientIds.end(); ++clIt)
        {
            sendMessage(_txRx[*clIt], buf);
        }
    }
    return 0;
}

int myRandom(int i)
{ 
    return std::rand()%i; 
}

void exitHandler(int s)
{
    exitRequested = 1;
    std::cout << "Exiting Transmitter application..." << std::endl;
    stopScheduler();
    shutdownBlRF(_txRx[0]); //TODO: MIMO
}

void sendMessage(TxRx& txRx, unsigned char* buf)
{
    int nWrite;
    memset((void*)buf, 0x00, PAYLOAD_LEN);
    sprintf(
        (char*)buf, 
        "PacketPacketPacketPacketPacketPacketPacketPacket%d",
#if MEASURING_BER || MEASURING_PCC
        1
#else
        txRx.msg+1
#endif
    );
    nWrite = sendto(
        txRx.sock, 
        (char *)buf, 
        strlen((char *)buf)+1,  // +1 for '\0'
        MSG_CONFIRM,
        (struct sockaddr *)&txRx.address,
        txRx.addrLen
    );
}

int initialize()
{
    int errCode;
    ModuleConfig config;
    //
    // Connect to bladeRF TX device
    //
    config.channel    = BLADERF_CHANNEL_TX(0);
    config.frequency  = TX_FREQ;
    config.bandwidth  = TX_BANDWIDTH;
    config.samplerate = TX_SAMPLING_RATE;
    config.gain       = TX_GAIN;
    for(int i = 0; i<NUM_TRANSMITS; i++)
    {
        _txRx[i].layout    = BLADERF_TX_X1;
        _txRx[i].direction = BLADERF_TX; 
    }
    errCode = setUpBladeRF(
        _txRx[0], 
        std::string(TX_SERIAL), 
        config
    );
    if(errCode != 0)
    {
        std::cerr 
            << "Connection to bladeRF TX module failed"
            << std::endl;
        return -1;
    }
    //
    // Since we have a single TX device
    //
    for(int i = 1; i<NUM_TRANSMITS; i++)
    {
        _txRx[i].blDev     = _txRx[0].blDev;
        _txRx[i].blDevInfo = _txRx[0].blDevInfo;
    }
    //
    // Connect to tun interfaces
    //
    for(int i = 0; i<NUM_TRANSMITS; i++)
    {
        strcpy(_txRx[i].tunDev.devName, _tunIntTNames[i]);
        strcpy(_txRx[i].tunDev.ipAddress, _tunIntTIP[i]);
        errCode = tunAlloc(_txRx[i].tunDev);
        if(errCode != 0)
        {
            std::cerr 
                << "TUN/TAP device allocation failed!\n"
                << "Error Code: " << errCode
                << std::endl;
            return -1;
        }
        _txRx[i].msg = 0;
    }
    //
    // Create client sockets for UDP communication
    // 
    for(int i = 0; i<NUM_TRANSMITS; i++)
    {
        errCode = clientCreate(_txRx[i], _servIPAddress);
        if(errCode != 0)
        {
            std::cerr 
                << "Socket Creation Failed"
                << std::endl;
            return -1;
        }
    }
    //
    // Initialize common structures
    //
    std::srand(unsigned (std::time(0)));
    for(int i = 0; i<NUM_TRANSMITS; i++)
    {
        _clientIds.push_back(i);
    }
    initScheduler(NUM_TRANSMITS, &_txRx[0]);
    return 0;
}

