/*
 * Scheduler.h
 *
 *  Created on: Apr 11, 2019
 *      Author: Ilker GURCAN, Fehime Betul CAVDARLI, Umay Ezgi KADAN
 *
 */

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <iostream>
#include <cstring>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>  //Read/Write to/from fd
#include <boost/thread.hpp>
#include <boost/atomic.hpp>

#include "TxRx.h"

void initScheduler(int numTrans, TxRx* txRx);

void startScheduler();

void stopScheduler();

#endif

