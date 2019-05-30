/*
 * GlobalConfig.cpp
 *
 *  Created on: Mar 17, 2019
 *      Author: Ilker GURCAN, Fehime Betul CAVDARLI, Umay Ezgi KADAN
 *
 */

#ifndef _GLOBAL_CONFIG__H_
#define _GLOBAL_CONFIG__H_

#define TX_SERIAL "35f88afa69a1e4475bd019bb8d3b1a72"  //the 5th device
#define RX_SERIAL "f005b46aa0600603344a9c3d1c684125"  //the 6th device

#define  TX_FREQ 455550000
#define  TX_BANDWIDTH 5000000
#define  TX_SAMPLING_RATE 5000000
#define  TX_GAIN 60

#define  RX_FREQ 455550000
#define  RX_BANDWIDTH 5000000
#define  RX_SAMPLING_RATE 5000000
#define  RX_GAIN 15

#define NUM_TRANSMITS 1

const char* _tunIntTNames[] = 
{
    "beitunT0",
    "beitunT1",
    "beitunT2",
    "beitunT3",
    "beitunT4"
};

const char* _tunIntTIP[] = 
{
    "192.168.4.4",
    "192.168.4.5",
    "192.168.4.6",
    "192.168.4.7",
    "192.168.4.8"
};

const char* _servIntName   = "beitunS";
const char* _servIPAddress = "192.168.5.1";

#endif

