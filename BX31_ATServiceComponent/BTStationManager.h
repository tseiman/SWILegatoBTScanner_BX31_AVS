/*
 * BTStationManager.h
 *
 *  This is part of the "BX31_ATService" Project
 *  Created on: Apr 4, 2019
 *      Author: Thomas Schmidt, SWI
 */


#include "BX31_ATServiceComponent.h"

#ifndef BTSTATIONMANAGER_H_
#define BTSTATIONMANAGER_H_

#define MAX_BT_STATION_HASHMAP_SIZE 600

typedef struct  {
	uint64_t btStationAddress;		// redundant storage of the btStationAddress (here and in scanResult)
									// - because the hasmap uses the reference to the address as key, scanResult
									// might be freed on update and the key reference would be destroyed
	le_clk_Time_t lastSeen;			// here the relative time stamp is set - in case the station was seen
	bool isDirty;					// in case the BT advertisement data has changed - this is set to true
	BTScanResult_t *scanResult;		// here the BT Scan result pointer is stored
} BT_Station_Container_t;

void initBtManager();
void updateList(BTScanResult_t *scanResult);

#endif /* BTSTATIONMANAGER_H_ */
