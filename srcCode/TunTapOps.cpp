/*
 * TunTapOps.cpp
 *
 *  Created on: Mar 17, 2019
 *      Author: Ilker GURCAN, Fehime Betul CAVDARLI, Umay Ezgi KADAN
 *
 */


#include "TunTapOps.h"

int tunAlloc(TunDevice& dev)
{
    struct ifreq ifr;
    int fd, err;

    if((fd = open("/dev/net/tun", O_RDWR)) < 0)
    {
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    /* Flags: IFF_TUN - TUN device (no Ethernet headers) 
     * IFF_TAP        - TAP device  
     * IFF_NO_PI      - Do not provide packet information  
     */ 
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI; 
    if(dev.devName)
        strncpy(ifr.ifr_name, dev.devName, IFNAMSIZ);

    if((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0)
    {
        close(fd);
        return err;
    }
    //
    // To make it persistent after application is closed
    //
    if((err = ioctl(fd, TUNSETPERSIST, 1)) < 0)
    {
        close(fd);
        return err;
    }
    strcpy(dev.devName, ifr.ifr_name);
    dev.fd = fd;
    return 0;
}

int setIPAddress(TunDevice& dev)
{

    struct ifreq ifr;
    struct sockaddr_in* addr = (struct sockaddr_in*)&ifr.ifr_addr;
    int fd                   = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    
    ifr.ifr_addr.sa_family   = AF_INET;
    strncpy(ifr.ifr_name, dev.devName, IFNAMSIZ);
    inet_pton(AF_INET, dev.ipAddress, &addr->sin_addr);
    if(ioctl(fd, SIOCSIFADDR, (caddr_t)&ifr) < 0)
    {
        std::cout 
            << "Failed to set IP: " << strerror(errno) 
            << std::endl;
        return -1;
    }
    inet_pton(AF_INET, "255.255.255.0", &addr->sin_addr);
    if(ioctl(fd, SIOCSIFNETMASK, (caddr_t)&ifr) < 0)
    {
        std::cout 
            << "Failed to set subnet mask: " << strerror(errno) 
            << std::endl;
        return -2;
    }

    return 0;
}

int tunUp(TunDevice& dev)
{
    struct ifreq ifr;
    int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    //
    // Gather flags from allocated device
    //
    strncpy(ifr.ifr_name, dev.devName, IFNAMSIZ);
    if(ioctl(fd, SIOCGIFFLAGS, &ifr) < 0)
    {
        std::cout << "Failed to get flags: " << strerror(errno) << std::endl;
        return -2;
    }
    //
    // Up the tun device
    //
    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
    if(ioctl(fd, SIOCSIFFLAGS, &ifr) < 0)
    {
        std::cout << "Failed to set flags: " << strerror(errno) << std::endl;
        return -3;
    }
    return 0;
}

