/*
 * BladeRFOps.h
 *
 *  Created on: Mar 17, 2019
 *      Author: Ilker GURCAN, Fehime Betul CAVDARLI, Umay Ezgi KADAN
 *
 */

#ifndef _BLADERF_OPS_H_
#define _BLADERF_OPS_H_

#include <unistd.h>  //Read/Write to/from fd
#include <iostream>
#include <string>
#include <stdio.h>
#include <libbladeRF.h>
#include <math.h>
#include <complex>
#include <liquid/liquid.h>

#define FPGA_FILE "./hostedx115-latest.rbf"
#define NUMBER_OF_BUFFERS 16
#define NUMBER_OF_TRANSFERS 8
#define TIMEOUT_IN_MS 5000
#define SAMPLE_SET_SIZE 8192
#define BUFFER_SIZE SAMPLE_SET_SIZE*sizeof(int32_t)

#define M 64          // number of subcarriers
#define CP_LEN 16     // cyclic prefix length
#define TAPER_LEN 4   // taper length
#define PAYLOAD_LEN 120

typedef struct module_config {
    bladerf_channel channel;
    unsigned int frequency;
    unsigned int bandwidth;
    unsigned int samplerate;
    int gain;
}ModuleConfig;

int initBladeRF(
    std::string&& serial, 
    struct bladerf_devinfo& devInfo,
    struct bladerf** dev
);

int configModule(struct bladerf* dev, ModuleConfig* c);

int initSync(
    struct bladerf* dev, 
    bladerf_channel_layout layout,
    bladerf_direction direction
);

int16_t* createTXSamples(
    unsigned char* payload, 
    unsigned int payloadLen, 
    unsigned int& symbLen
);

int16_t* convertComplexfloatToSC16Q11(
    std::complex<float>* in, 
    unsigned int inlen
);

std::complex<float>* convertSC16Q11ToComplexfloat(int16_t* in, int16_t inlen);

int syncTX(
    struct bladerf *dev, 
    int16_t* txSamples, 
    unsigned int symbLen
);

int syncRX(
    struct bladerf *dev, 
    int16_t *rxSamples,
    int (*onReceiveSamples)(int16_t*, unsigned int)
);

int calibrate(struct bladerf* dev, bladerf_direction direction);

#endif

