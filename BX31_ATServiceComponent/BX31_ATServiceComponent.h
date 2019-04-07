/*
 * BX31_ATServiceComponent.h
 *
 *  Created on: Apr 3, 2019
 *      Author: tseiman
 */
#include "legato.h"
#include "interfaces.h"
#include <stdint.h>

#ifndef BX31_ATSERVICECOMPONENT_H_
#define BX31_ATSERVICECOMPONENT_H_

#define MAX_SCANNED_STATION_MEM_POOL_SIZE 1024
#define MAX_BT_DATA_STRING_SIZE 31				// BT Advert Packet is not longer than 31 bytes

#define BX31_BT_PUBLIC_ADDR 0
#define BX31_BT_PRIVATE_ADDR 1


typedef struct  {
	uint64_t btStationAddress;
	uint8_t addrType;
	int rssi;
	int data_len;
	char advertData[MAX_BT_DATA_STRING_SIZE];  // Fixed buffer no allocation for simplicity -
                                               // need to take care it is not copied more than MAX_BT_DATA_STRING_SIZE
} BTScanResult_t;


void bx31at_initBLE(void (*callbackOnScan)(int, BTScanResult_t*));
void bx31at_stopBLE();
le_atClient_CmdRef_t bx31at_getCmdRef();
void bx31at_ScanBLE(le_timer_Ref_t timerRef);

#endif /* BX31_ATSERVICECOMPONENT_H_ */

