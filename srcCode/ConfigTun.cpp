/*
 * ConfigTun.cpp
 *
 *  Created on: Mar 17, 2019
 *      Author: Ilker GURCAN, Fehime Betul CAVDARLI, Umay Ezgi KADAN
 *
 */

#include <iostream>
#include "TxRx.h"
#include "GlobalConfig.h"

int main(int argc, char* argv[])
{
    TxRx txRx;
    int errCode;
    ModuleConfig config;
    //
    // Create transmitter interfaces
    //
    for(int i = 0; i<NUM_TRANSMITS; i++)
    {
        strcpy(txRx.tunDev.devName, _tunIntTNames[i]);
        strcpy(txRx.tunDev.ipAddress, _tunIntTIP[i]);
        errCode = setUpTunTap(txRx);
        if(errCode != 0)
        {
            return -1;
        }
    }
    //
    // Create interface for receiver
    //
    strcpy(txRx.tunDev.devName, _servIntName);
    strcpy(txRx.tunDev.ipAddress, _servIPAddress);
    errCode = setUpTunTap(txRx);
    if(errCode != 0)
    {
        return -1;
    }
    return 0;
}

