/*
 * Receiver.h
 *
 *  Created on: Apr 15, 2019
 *      Author: Ilker GURCAN, Fehime Betul CAVDARLI, Umay Ezgi KADAN
 *
 */
#include <netinet/ip.h>
#include <unistd.h>  //Read/Write to/from fd
#include <signal.h>  //Listening to signal exit
#include <iostream>
#include <ctime>
#include <chrono>
#include <vector>
#include <boost/thread.hpp>
#include <boost/regex.hpp> 
#include <liquid/liquid.h>

#include "TxRx.h"
#include "GlobalConfig.h"

#define MEASURING_BER 0
#define ADD_CH_IMP 0

volatile sig_atomic_t exitRequested = 0;
static TxRx _txRx;
static char _buffer[1500];
static boost::thread _serverThread;
static ofdmflexframesync _fs;
//
// Performance measurements
//
static int _packetCount[NUM_TRANSMITS] = {0};
static int _firstPacket[NUM_TRANSMITS] = {0};
static int _bitErrors                  = 0;
static int _ipPackLossCount            = 0;
static bool _isTimerStarted            = false;
static std::chrono::steady_clock::time_point _start;
static std::chrono::steady_clock::time_point _lastPacketRcvd;

int initialize();
void exitHandler(int s);
int onReceiveSamples(int16_t* rxSamples, unsigned int sampleLen);
int flexFrameCallback(
    unsigned char* header,
    int headerValid,
    unsigned char* payload,
    unsigned int payloadLen,
    int payloadValid,
    framesyncstats_s stats,
    void*  userdata
);
void service();
//
// Performance measurements
//
void measureStatistics();
unsigned int measureBER(unsigned char* payload);
void calculatePacketStats(const char* ip);

int main(int argc, char * argv[])
{
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
    // Start listening to tun interface
    //
    _serverThread = boost::thread(service);
    //
    // Main Loop
    //
    int16_t *rxSamples = (int16_t*)malloc(SAMPLE_SET_SIZE*2*sizeof(int16_t));
    while(!exitRequested && !errCode)
    {
        //
        // Read messages
        //
        errCode = syncRX(_txRx.blDev, rxSamples, onReceiveSamples);
    }
    free(rxSamples);
    return 0;
}

void exitHandler(int s)
{
    std::cout << "Exiting Receiver application..." << std::endl;
    exitRequested = 1;
    //
    // Wait a few seconds for any remaining TX 
    // samples to finish reaching the RF front-end
    //
    usleep(2000000);
    _serverThread.interrupt();
    _serverThread.join();
    shutdownBlRF(_txRx);
    //
    // Print statistics
    //
    std::cout 
        << "***********************Statistics***********************" 
        << std::endl;
    measureStatistics();
}

void measureStatistics()
{
    for(int i = 0; i<NUM_TRANSMITS; i++)
    {
        std::cout 
            << "# of Packets captured and the first packet for interface " 
            << _tunIntTNames[i] 
            << " are " << "(" << _packetCount[i] << "," << _firstPacket[i] << ")"
            << std::endl;
    }
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
        (_lastPacketRcvd-_start);
    int numPackets = 0;
    for(int i = 0; i<NUM_TRANSMITS; i++)
    {
        numPackets += _packetCount[i];
    }
    double pps = (double)numPackets / ((double)duration.count()/1000.0);
    std::cout 
        << "Throughput: " << pps  << " packets/sec "
        << "for total " << ((double)duration.count()/1000.0) << " secs"
        << std::endl;
    std::cout 
        << "Ip packet loss count: " 
        << _ipPackLossCount
        << std::endl;
#if MEASURING_BER
    double ber = (double)_bitErrors / numPackets;
    std::cout << "BER: " << ber << std::endl;
#endif

}

unsigned int measureBER(unsigned char* payload)
{
    unsigned int numErrors = count_bit_errors_array(
        payload,
        (unsigned char*)"PacketPacketPacketPacketPacketPacketPacketPacket1",
        50
    );
    return numErrors;
}

void calculatePacketStats(const char* ip, char* buffer)
{
    int idx = -1;
    for(int i = 0; i<NUM_TRANSMITS; i++)
    {
        if(strcmp(_tunIntTIP[i], ip) == 0)
        {
            idx = i;
            break;
        }
    }
    if(idx == -1)
    {
        std::cerr 
            << "Unknown client ip address: " 
            << ip 
            << std::endl;
    }
    //
    // Extract the first packet count
    //
    if(_packetCount[idx] == 0)
    {
        std::string input(buffer);
        std::vector<std::string> result;
        boost::regex re("\\d+$");
        boost::sregex_iterator it(input.begin(),input.end(), re);
        std::cout << it->str() << std::endl;
        _firstPacket[idx] = std::stoi(it->str());
    }
    if(!_isTimerStarted)
    {
        _start          = std::chrono::steady_clock::now();
        _isTimerStarted = true;
    }
    _lastPacketRcvd = std::chrono::steady_clock::now();
    _packetCount[idx]++;
    std::cout 
        << "\r"
        << "Received packet count for interface " << _tunIntTNames[idx]
        << " is " << _packetCount[idx] 
        << std::flush;
#if MEASURING_BER
    _bitErrors += measureBER((unsigned char*)buffer);
#endif
}

void service()
{
    int ns, nRead;
    struct sockaddr_in cliAddr;
    char buffer[1600] = {0};
    socklen_t clLen   = (socklen_t)sizeof(cliAddr);

    while(!exitRequested)
    {
        try
        {
            memset(&cliAddr, 0, sizeof(cliAddr)); 
            if((nRead = 
                recvfrom(
                    _txRx.sock, 
                    buffer, 
                    1600, 
                    0, 
                    (struct sockaddr *) &cliAddr, &clLen)) > 0 && 
                errno != EAGAIN && errno != EWOULDBLOCK
            )
            {
                char *ip = inet_ntoa(cliAddr.sin_addr);
                //
                // Calculate packet statistics
                //
                calculatePacketStats(ip, buffer);
            }
            memset((void*)buffer, 0x00, sizeof(buffer));
        }
        catch(boost::thread_interrupted& ex)
        {
            std::cout << "Service thread is interrupted!" << std::endl;
        }
    } //End of outer while-loop
    std::cout << "Service thread quits..." << std::endl;
}

int flexFrameCallback(
    unsigned char* header,
    int headerValid,
    unsigned char* payload,
    unsigned int payloadLen,
    int payloadValid,
    framesyncstats_s stats,
    void*  userData
)
{
    int nWrite;
    int nfds;
    fd_set fds;
    //
    // Write to the tun interface
    //
    if(headerValid)
    {
        if(!payloadValid)
        {
            _ipPackLossCount++;
        }
        nfds = _txRx.tunDev.fd+1;
        FD_ZERO(&fds);
        FD_SET(_txRx.tunDev.fd, &fds);
        nfds = select(nfds, 0, &fds, 0, 0);
        if (nfds < 0) 
        {
            fprintf(
                stderr,
                "select() in flexFrameCallback failed\n"
            );
            return -1;
        }
        if (FD_ISSET(_txRx.tunDev.fd, &fds)) 
        {
            nWrite = write(
                _txRx.tunDev.fd,
                payload,
                payloadLen
            );
        }
    }
    return 0;
}

int onReceiveSamples(int16_t* rxSamples, unsigned int sampleLen)
{
    int status=0;
    std::complex<float>* y = convertSC16Q11ToComplexfloat(rxSamples, sampleLen);
    int symbLen = M+CP_LEN;
#if ADD_CH_IMP
    float noiseFloor = -80.0f;             // noise floor [dB]
    float SNRdB      = 56.0f;              // signal-to-noise ratio [dB]
    float dphi       = 0.02f;              // carrier frequency offset
    //
    // Create channel impairement
    //
    channel_cccf channel = channel_cccf_create();
    channel_cccf_add_awgn(channel, noiseFloor, SNRdB);
    channel_cccf_add_carrier_offset(channel, dphi, 0.0f);
    channel_cccf_execute_block(channel, y, sampleLen, y);
#endif

    if (y != nullptr)
    {
        for (int i=0; i<=sampleLen; i=i+symbLen)
        {
            ofdmflexframesync_execute(_fs, &y[i], symbLen);
        }
        free(y);
    }
    else
    {
        status = BLADERF_ERR_MEM;
    }
    return status;
}

int initialize()
{
    int errCode;
    ModuleConfig config;
    unsigned char p[M];
    unsigned int mPilot, mData, mNull;
    //
    // Connect to bladeRF RX device
    //
    config.channel    = BLADERF_CHANNEL_RX(0);
    config.frequency  = RX_FREQ;
    config.bandwidth  = RX_BANDWIDTH;
    config.samplerate = RX_SAMPLING_RATE;
    config.gain       = RX_GAIN;
    _txRx.layout      = BLADERF_RX_X1;
    _txRx.direction   = BLADERF_RX;
    errCode           = setUpBladeRF(
        _txRx, 
        std::string(RX_SERIAL), 
        config
    );
    if(errCode != 0)
    {
        std::cerr 
            << "Conection to bladeRF RX module failed"
            << std::endl;
        return -1;
    }
    //
    // Connect to tun interfaces
    //
    strcpy(_txRx.tunDev.devName, _servIntName);
    strcpy(_txRx.tunDev.ipAddress, _servIPAddress);
    errCode = tunAlloc(_txRx.tunDev);
    if(errCode != 0)
    {
        std::cerr 
            << "TUN/TAP device allocation failed!\n"
            << "Error Code: " << errCode
            << std::endl;
        return -1;
    }
    //
    // Create listening socket
    //
    errCode = serverCreate(_txRx);
    if(errCode != 0)
    {
        std::cerr 
            << "Socket Creation for Server Failed"
            << std::endl;
        return -1;
    }
    //
    // Create OFDM frame synchronizer
    //
    ofdmframe_init_default_sctype(M, p);
    ofdmframe_validate_sctype(
        p, 
        M, 
        &mNull, 
        &mPilot, 
        &mData
    );
    _fs = ofdmflexframesync_create(
        M, 
        CP_LEN, 
        TAPER_LEN, 
        p, 
        flexFrameCallback, 
        (void*)_buffer
    );
    if(_fs == NULL)
    {
        fprintf(
            stderr,
            "Failed to create frame synchronizer\n"
        );
        return -1;
    }
    ofdmflexframesync_print(_fs);
    return 0;
}

