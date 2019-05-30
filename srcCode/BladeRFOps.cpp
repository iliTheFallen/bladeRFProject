/*
 * BladeRFOps.cpp
 *
 *  Created on: Mar 17, 2019
 *      Author: Ilker GURCAN, Fehime Betul CAVDARLI, Umay Ezgi KADAN
 *
 */

#include <string.h>

#include "BladeRFOps.h"


int initBladeRF(
    std::string&& serial, 
    struct bladerf_devinfo& devInfo,
    struct bladerf** dev
)
{
    //
    // Trying to open the bladeRF device
    //
    std::string finalSerial = std::string("*:serial=")+std::move(serial);
    int status              = bladerf_open(dev, finalSerial.c_str());
    //
    // Could not be opened!
    //
    if (status != 0)
    {
        std::cerr 
            << "bladeRF device with serial: " << serial 
            << " is failed to open"
            << std::endl;
        std::cerr << "Error: " << bladerf_strerror(status) << std::endl;
        return -1;
    }
    //
    // Trying to read bladeRF device info
    //
    status = bladerf_get_devinfo_from_str(finalSerial.c_str(), &devInfo);
    //
    // Could not be read!
    //
    if (status != 0)
    {
        std::cerr 
            << "bladeRF device with serial: " << serial 
            << " failed at getting device info"
            << std::endl;
        std::cerr << "Error: " << bladerf_strerror(status) << std::endl;
        return -1;
    }
    return 0;
}

int configModule(struct bladerf* dev, ModuleConfig* c)
{
    int status;
    status = bladerf_set_frequency(dev, c->channel, c->frequency);
    if(status != 0) 
    {
        fprintf(stderr, "Failed to set frequency = %u: %s\n", c->frequency,
                bladerf_strerror(status));
        return status;
    }
    status = bladerf_set_sample_rate(dev, c->channel, c->samplerate, NULL);
    if(status != 0) 
    {
        fprintf(stderr, "Failed to set samplerate = %u: %s\n", c->samplerate,
                bladerf_strerror(status));
        return status;
    }
    status = bladerf_set_bandwidth(dev, c->channel, c->bandwidth, NULL);
    if(status != 0) 
    {
        fprintf(stderr, "Failed to set bandwidth = %u: %s\n", c->bandwidth,
                bladerf_strerror(status));
        return status;
    }
    status = bladerf_set_gain(dev, c->channel, c->gain);
    if(status != 0) 
    {
        fprintf(stderr, "Failed to set gain: %s\n", bladerf_strerror(status));
        return status;
    }
    return status;
}

int initSync(
    struct bladerf* dev, 
    bladerf_channel_layout layout,
    bladerf_direction direction
)
{
    int status;
    status = bladerf_sync_config(
        dev,
        layout,
        BLADERF_FORMAT_SC16_Q11_META,
        NUMBER_OF_BUFFERS,
        BUFFER_SIZE,
        NUMBER_OF_TRANSFERS,
        3500
    );
    if (status != 0) 
    {
        fprintf(
            stderr, 
            "Failed to configure RX sync interface: %s\n", 
            bladerf_strerror(status)
        );
        return status;
    }
    status = bladerf_enable_module(dev, direction, true);
    if (status != 0) 
    {
        fprintf(
            stderr, 
            "Failed to enable module: %s\n", 
            bladerf_strerror(status)
        );
        return status;
    }
    return 0;
}

int16_t* convertComplexfloatToSC16Q11(
    std::complex<float>* in, 
    unsigned int inlen
)
{
    int i         = 0;
    int16_t* out  = NULL;
    out           = (int16_t *)malloc(SAMPLE_SET_SIZE*2*sizeof(int16_t));
    if(out != NULL) 
    {
        for(i = 0; i < inlen ; i++)
        {
            out[2*i]= round(real(in[i]) * 2048); // Since bladeRF uses Q4.11 complex 2048=2^11
            out[2*i+1]= round(imag(in[i]) * 2048);
            if (out[2*i] > 2047)
            {
                out[2*i]=2047;
            }
            if(out[2*i] < -2048)
            {
                out[2*i]=-2048;
            }
            if (out[2*i+1] > 2047) 
            {
                out[2*i+1]=2047;
            }
            if (out[2*i+1] < -2048) 
            {
                out[2*i+1]=-2048;
            }
        }
    }
    return (int16_t *)out;
}

std::complex<float>* convertSC16Q11ToComplexfloat(int16_t* in, int16_t inlen)
{

    std::complex<float>* out = nullptr;
    out = (std::complex<float>*)malloc(inlen*sizeof(std::complex<float>));
    if (out != nullptr)
    {
        for(int i = 0; i < inlen ; i++) 
        {
            out[i]= std::complex<float>(in[2*i]/2048.0, in[2*i+1]/2048.0);
        }
    }
    return out;
}

int16_t* createTXSamples(
    unsigned char* payload, 
    unsigned int payloadLen, 
    unsigned int& symbLen
) 
{
    int bufLen = M+CP_LEN;
    unsigned char header[8];
    std::complex<float> buffer[bufLen];
    std::complex<float> outBuffer[BUFFER_SIZE];
    unsigned char p[M];
    unsigned int mPilot, mData, mNull;
    int16_t* txSamples;
    //
    // Create frame generator
    //
    ofdmflexframegenprops_s fgProps;
    ofdmflexframegenprops_init_default(&fgProps);
    ofdmframe_init_default_sctype(M, p);
    ofdmframe_validate_sctype(
        p, 
        M, 
        &mNull, 
        &mPilot, 
        &mData
    );
    fgProps.check       = LIQUID_CRC_32; //validity check
    fgProps.fec0        = LIQUID_FEC_NONE; // inner code
    fgProps.fec1        = LIQUID_FEC_HAMMING128; // outer code
    fgProps.mod_scheme  = LIQUID_MODEM_QPSK; //modulation scheme
    ofdmflexframegen fg = ofdmflexframegen_create(
        M, 
        CP_LEN, 
        TAPER_LEN, 
        p, 
        &fgProps
    );
    //
    // Assemble header and payload
    //
    for(int i=0; i<8; i++)
    {
        header[i] = i & 0xff;
    }


    ofdmflexframegen_assemble(fg, header, payload, payloadLen);
    //ofdmflexframegen_print(fg);
    symbLen = ofdmflexframegen_getframelen(fg);
    //
    // Write symbols
    //
    int lastSymbol = 0;
    int lastPos    = 0;
    while(!lastSymbol)
    {
        lastSymbol = ofdmflexframegen_write(fg, buffer, bufLen);
#if ADD_CH_IMP
#endif
        memcpy(&outBuffer[lastPos], buffer, bufLen*sizeof(std::complex<float>));
        lastPos += bufLen;
    }
    txSamples = convertComplexfloatToSC16Q11(&outBuffer[0], symbLen*bufLen);
    ofdmflexframegen_destroy(fg);
    if (txSamples == NULL) 
    {
        fprintf(
            stderr, 
            "Failed to configure RX sync interface: %s\n", 
            bladerf_strerror(BLADERF_ERR_MEM)
        );
        return NULL;
    }
    symbLen = 2*symbLen*bufLen;
    return txSamples;
}

int syncTX(struct bladerf *dev, int16_t* txSamples, unsigned int symbLen)
{
    int status = 0;
    struct bladerf_metadata meta;
    memset(&meta, 0, sizeof(meta));
    meta.flags = 
        BLADERF_META_FLAG_TX_BURST_START | 
        BLADERF_META_FLAG_TX_NOW | 
        BLADERF_META_FLAG_TX_BURST_END;
    status = bladerf_get_timestamp(dev, BLADERF_TX, &meta.timestamp);
    if (status != 0) 
    {
        fprintf(stderr, "Failed to get current TX timestamp: %s\n",
                bladerf_strerror(status));
        return status;
    } 
    else 
    {
        // printf("Current TX timestamp: %016" PRIu64 "\n", meta.timestamp);
        status = bladerf_sync_tx(
                dev, 
                txSamples, 
                SAMPLE_SET_SIZE, 
                &meta, 
                TIMEOUT_IN_MS
        );
    }
    return status;
}

int syncRX(
    struct bladerf *dev, 
    int16_t *rxSamples,
    int (*onReceiveSamples)(int16_t*, unsigned int)
)
{
    int status = 0;
    struct bladerf_metadata meta;
    if (rxSamples == NULL) 
    {
        fprintf(stdout, "malloc error: %s\n", bladerf_strerror(status));
        return BLADERF_ERR_MEM;
    }   
    //
    // Perform a read immediately
    //
    memset(&meta, 0, sizeof(meta));
    meta.flags = BLADERF_META_FLAG_RX_NOW;
    status     = bladerf_get_timestamp(dev, BLADERF_RX, &meta.timestamp);
    // Receive samples
    status = bladerf_sync_rx(
        dev, 
        rxSamples, 
        SAMPLE_SET_SIZE, 
        &meta, 
        TIMEOUT_IN_MS
    );
    if (status != 0) 
    {
        fprintf(stderr, "Scheduled RX failed: %s\n\n",
                bladerf_strerror(status));
    } 
    else if (meta.status & BLADERF_META_STATUS_OVERRUN) 
    {
        fprintf(stderr, "Overrun detected in scheduled RX. "
                "%u valid samples were read.\n\n",
                meta.actual_count);
    }
    else 
    {
        //printf("RX'd %u samples at t=0x%016" PRIx64 "\n", meta.actual_count,
        //        meta.timestamp);
        //fflush(stdout);
        status = onReceiveSamples(rxSamples, meta.actual_count);
    }
    return status;
}

int calibrate(struct bladerf* dev, bladerf_direction direction)
{
    int status;
    status = bladerf_calibrate_dc(dev, BLADERF_DC_CAL_LPF_TUNING);
    status = bladerf_calibrate_dc(dev, BLADERF_DC_CAL_TX_LPF);
    status = bladerf_calibrate_dc(dev, BLADERF_DC_CAL_RX_LPF);
    status = bladerf_calibrate_dc(dev, BLADERF_DC_CAL_RXVGA2);
    return 0;
}

