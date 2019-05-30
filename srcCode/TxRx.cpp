/*
 * TunTapOps.cpp
 *
 *  Created on: Apr 1, 2019
 *      Author: Ilker GURCAN, Fehime Betul CAVDARLI, Umay Ezgi KADAN
 *
 */

#include <iostream>
#include "TxRx.h"

int setUpTunTap(TxRx& txRx)
{
    int errCode;
    std::cout 
        << "**************************************************\n"
        << "Allocating tun/tap device: " << txRx.tunDev.devName
        << std::endl;
    errCode = tunAlloc(txRx.tunDev);
    if(errCode != 0)
    {
        std::cerr 
            << "TUN/TAP device allocation failed!\n"
            << "Error Code: " << errCode
            << std::endl;
        return -1;
    }
    std::cout 
        << "Setting ip addr for tun/tap device: " << txRx.tunDev.ipAddress
        << std::endl;
    errCode = setIPAddress(txRx.tunDev);
    if(errCode != 0)
    {
        std::cerr 
            << "Setting ip address for tun/tap device failed!"
            << std::endl;
        return -1;
    }
    std::cout 
        << "Activating tun/tap device: " << txRx.tunDev.devName
        << std::endl;
    errCode = tunUp(txRx.tunDev);
    if(errCode != 0)
    {
        std::cerr 
            << "Activating tun/tap device failed!"
            << std::endl;
        return -1;
    }
    std::cout 
        << "**************************************************"
        << std::endl;
    return 0;
}

int setUpBladeRF(
    TxRx& txRx, 
    std::string&& serial,
    ModuleConfig config
)
{
    int status;
    //
    // Open, Get device info, and Load FPGA
    //
    status = initBladeRF(
        std::move(serial),
        txRx.blDevInfo,
        &txRx.blDev
    );
    if(status != 0)
    {
        return -1;
    }
    status = initSync(txRx.blDev, txRx.layout, txRx.direction);
    if(status != 0)
    {
        return -1;
    }
    return 0;
}

void shutdownBlRF(TxRx& txRx)
{
    //
    // Disable TX, shutting down our underlying TX stream
    //
    int status = bladerf_enable_module(
        txRx.blDev, 
        txRx.direction, 
        false
    );
    if (status != 0) 
    {
        fprintf(
            stderr, 
            "Failed to disable TX/RX streaming: %s\n", 
            bladerf_strerror(status)
        );
    }
    bladerf_close(txRx.blDev);
}

int clientCreate(TxRx& txRx, const char* servIpAddress)
{
    struct sockaddr_in servAddr;
    struct ifreq ifr;
    int sock = 0;
    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    { 
        std::cerr << "Socket creation error..." << std::endl;
        return -1; 
    } 
    memset(&servAddr, '0', sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port   = htons(8080);
    if(inet_pton(AF_INET, servIpAddress, &servAddr.sin_addr) <= 0)  
    { 
        std::cerr << "Invalid server address/ Address not supported" << std::endl;
        close(sock);
        return -1; 
    }
/*
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, txRx.tunDev.devName, sizeof(ifr.ifr_name));  
    if(setsockopt(
        sock, 
        SOL_SOCKET, 
        SO_BINDTODEVICE, 
        (void *)&ifr,
        sizeof(ifr)) < 0
    ) 
    {
        std::cerr << "setsockopt()--BINDTODEVICE" << std::endl;
        close(sock);
        return -1;
    }
*/
    txRx.sock    = sock;
    txRx.address = servAddr;
    txRx.addrLen = (socklen_t) sizeof(servAddr);
    return 0;
}

int serverCreate(TxRx& txRx)
{
    struct sockaddr_in address;
    int sock   = 0;
    int optVal = 1;
    struct ifreq ifr;
    struct timeval tv;
    
    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    { 
        std::cerr << "Socket creation error..." << std::endl;
        return -1; 
    }
    tv.tv_sec  = 1;
    tv.tv_usec = 500000;  // 500 msecs
    if(setsockopt(
        sock, 
        SOL_SOCKET, 
        SO_RCVTIMEO, 
        (char *)&tv, sizeof(tv)) < 0
    ) 
    {
        std::cerr << "setsockopt()-timeout" << std::endl;
        close(sock);
        return -1;
    }
    if(setsockopt(
        sock, 
        SOL_SOCKET, 
        SO_REUSEADDR, 
        (char *)&optVal, sizeof(optVal)) < 0
    ) 
    {
        std::cerr << "setsockopt()-reuseaddr" << std::endl;
        close(sock);
        return -1;
    }
    memset(&address, '0', sizeof(address));
    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port        = htons(8080);
    if(bind(
        sock, 
        (const struct sockaddr *)&address,  
        sizeof(address)) < 0
    ) 
    { 
        std::cerr << "bind()" << std::endl;
        close(sock);
        return -1;
    }
    txRx.sock    = sock;
    txRx.address = address;
    txRx.addrLen = (socklen_t) sizeof(address);
    return 0;
}

