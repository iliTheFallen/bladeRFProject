/*
 * TunTapOps.cpp
 *
 *  Created on: Mar 17, 2019
 *      Author: Ilker GURCAN, Fehime Betul CAVDARLI, Umay Ezgi KADAN
 *
 */

#ifndef _TUN_TAP_OPS_H_
#define _TUN_TAP_OPS_H_

#include <iostream>
#include <cstring>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <arpa/inet.h>

typedef struct tun_device
{
    char devName[IFNAMSIZ];
    char ipAddress[IFNAMSIZ];
    int fd;
}TunDevice;

/*
 * Creates tun device with the name 
 * specified in "TunDevice" structure.
 * Args:
 *    dev :  Device structure
 * Returns:
 *    errCode: 0 if dev is allocated; <0 o.w.
 *
 */
int tunAlloc(TunDevice& dev);

/*
 * Set ip address to tun device 
 * specified in "TunDevice" structure.
 * Args:
 *    dev :  Device structure
 * Returns:
 *    errCode: 0 if dev is allocated; <0 o.w.
 *
 */
int setIPAddress(TunDevice& dev);

/*
 * Activate (up) tun device with the name 
 * specified in "TunDevice" structure
 * Args:
 *    dev :  Device structure
 * Returns:
 *    errCode: 0 if dev is allocated; <0 o.w.
 *
 */
int tunUp(TunDevice& dev);

#endif

