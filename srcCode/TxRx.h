/*
 * TunTapOps.cpp
 *
 *  Created on: Apr 1, 2019
 *      Author: Ilker GURCAN, Fehime Betul CAVDARLI, Umay Ezgi KADAN
 *
 */

#ifndef _TX_RX__H_
#define _TX_RX__H_


#include <stdlib.h>
#include <stdlib.h>
#include <string>
#include <libbladeRF.h>
//
// Socket Programming
//
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netinet/in.h>

#include "TunTapOps.h" 
#include "BladeRFOps.h"

typedef struct tx_rx 
{
    TunDevice tunDev;
    bladerf_channel_layout layout;
    bladerf_direction direction;
    struct bladerf_devinfo blDevInfo;
    struct bladerf* blDev;
    //
    // For UDP-Socket communication.
    // Server address in case of TX; while
    // in case of RX, it is the address  
    // where socket listens to in.
    //
    int sock;
    struct sockaddr_in address;
    socklen_t addrLen;
    int msg;
}TxRx;
//
// Methods which receives TxRx structure and 
// modifies it in accordance with parameters it holds.
//
int setUpTunTap(TxRx& txRx);

int setUpBladeRF(TxRx& txRx, std::string&& serial, ModuleConfig config);

void shutdownBlRF(TxRx& txRx);

int serverCreate(TxRx& txRx);

int clientCreate(TxRx& txRx, const char* servIpAddress);

#endif

