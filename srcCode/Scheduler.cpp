/*
 * Scheduler.cpp
 *
 *  Created on: Apr 11, 2019
 *      Author: Ilker GURCAN, Fehime Betul CAVDARLI, Umay Ezgi KADAN
 *
 */
#include "Scheduler.h"
#include "BladeRFOps.h"

static int _numTrans;
static TxRx* _txRx;
static boost::thread _schThread;
static boost::atomic<bool> _exitReq(false);

void initScheduler(int numTrans, TxRx* txRx)
{
    _numTrans = numTrans;
    _txRx     = txRx;
}

int findMaxFD()
{
    int maxFD = -1;
    for(int i = 0; i<_numTrans; i++)
    {
        if(maxFD < _txRx[i].tunDev.fd)
        {
            maxFD = _txRx[i].tunDev.fd;
        }
    }
    return maxFD;
}

void sendToBladeRF(TxRx& txRx, char* buf, int nRead)
{
    unsigned int symbLen;
    int errCode;
    //
    // Create TX samples for bladeRF
    //
    int16_t* txSamples = createTXSamples(
        (unsigned char *)buf,
        nRead,
        symbLen
    );
    //
    // Send frame generated by liquidSDR to 
    // the RX device through bladeRF
    //
    errCode = syncTX(txRx.blDev, txSamples, symbLen);
    if(errCode != 0)
    {
        std::cerr << "syncTX()-failed. Stopping scheduler" << std::endl;
        stopScheduler();
    }
    free((void*)txSamples);
    txRx.msg++;
    std::cout 
        << "\r"
        << "Sent packet count for interface " << txRx.tunDev.devName
        << " is " << txRx.msg
        << std::flush;
}

void readNetworkMsg(TxRx& txRx, char* buf, int& nRead)
{
    memset((void*)buf, 0x00, PAYLOAD_LEN);
    nRead = read(
        txRx.tunDev.fd, 
        buf, 
        PAYLOAD_LEN
    );
}

void schedule()
{ 
    fd_set readfds;
    int nReads, maxFD;
    char buf[PAYLOAD_LEN] = {0};
    int nRead             = 0;
    maxFD                 = findMaxFD();
    while(!_exitReq.load())
    {
        try
        {
            nReads = 0;
            while(nReads < _numTrans)
            {
                FD_ZERO(&readfds);
                for(int i = 0; i < _numTrans; i++)
                {
                    FD_SET(_txRx[i].tunDev.fd, &readfds);
                }
                select(maxFD+1, &readfds, NULL, NULL, NULL);
                for(int i = 0; i < _numTrans; i++)
                {
                    if(FD_ISSET(_txRx[i].tunDev.fd, &readfds))
                    {
                        readNetworkMsg(_txRx[i], buf, nRead);
                        sendToBladeRF(_txRx[i], buf, nRead);
                        break;
                    }
                }
                nReads++;
            } //End of inner while-loop
        }
        catch(boost::thread_interrupted& ex)
        {
            std::cout << "Scheduler thread is interrupted!" << std::endl;
        }
    } //End of outer while-loop
    std::cout << "Scheduler thread quits..." << std::endl;
}

void startScheduler()
{
    std::cout << "Starting scheduler..." << std::endl;
    _schThread = boost::thread(schedule);
}

void stopScheduler()
{
    std::cout << "Stopping scheduler..." << std::endl;
    _exitReq.store(true);
    _schThread.interrupt();
    _schThread.join();
}

