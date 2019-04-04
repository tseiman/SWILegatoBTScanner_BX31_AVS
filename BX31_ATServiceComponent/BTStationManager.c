/*
 * BTStationManager.c
 *
 *  This is part of the "BX31_ATService" Project
 *  Created on: Apr 4, 2019
 *      Author: Thomas Schmidt, SWI
 */

#include "BX31_ATServiceComponent.h"
#include "BTStationManager.h"


static le_hashmap_Ref_t stationHashMap = NULL;

static le_mem_PoolRef_t bTStationContainerPool = NULL;




/** ------------------------------------------------------------------------
 *
 * Compares 2 scan result structs
 *
 * @param scan result 1
 * @param scan result 2
 *
 * @return 0 if equal
 *
 * ------------------------------------------------------------------------
 */

int btScanCmp(BTScanResult_t *scanResult1, BTScanResult_t *scanResult2) {   						// FIXME not sure we should put this function here - could be also in BX31_ATServiceComponent.c
	if(scanResult1->btStationAddress != scanResult2->btStationAddress) return 1;
	if(scanResult1->addrType != scanResult2->addrType) return 2;
	if(scanResult1->data_len != scanResult2->data_len) return 3;
	/* if(scanResult1->rssi != scanResult2->rssi) return 4;       									We don't compare RSSI !! it changing permanently and it makes no sense to compare */

	for(int i = 0; i <= scanResult1->data_len; ++i) {												// Finally we check if the advertisement data differs
		if(scanResult1->advertData[i] != scanResult2->advertData[i]) return 4+i;					// before we checked that both scanReults have the same length
	}

	return 0;																						// if all tests failed - the both scan results are equal
}



void initBtManager() {

	LE_DEBUG("initializing BTStationManager");

	LE_ASSERT((
			stationHashMap = le_hashmap_Create(
			"BT Station Table",
			MAX_BT_STATION_HASHMAP_SIZE,
			le_hashmap_HashUInt64,
			le_hashmap_EqualsUInt64))
			!= NULL);

	le_hashmap_MakeTraceable(stationHashMap);

	bTStationContainerPool = le_mem_CreatePool("BX31_ATService.station.container", sizeof(BT_Station_Container_t));
	le_mem_ExpandPool(bTStationContainerPool, MAX_SCANNED_STATION_MEM_POOL_SIZE);



}

void updateList(BTScanResult_t *scanResult) {

	if(stationHashMap == NULL || bTStationContainerPool == NULL) initBtManager();

	if(le_hashmap_ContainsKey(stationHashMap,&scanResult->btStationAddress)) {
		BT_Station_Container_t *sCont = NULL;

		LE_ASSERT((sCont = le_hashmap_Get(stationHashMap, &scanResult->btStationAddress)) !=NULL);	// HasMap lookup for the key (should be there because we checked before)
		sCont->lastSeen = le_clk_GetRelativeTime();
		sCont->scanResult->rssi = scanResult->rssi; 		 										// we don't throw away the old scan result in case only the RSSI changed

		if(btScanCmp(sCont->scanResult, scanResult) == 0) {   										// in case the old and the new scan result are equal the new one is removed from memory
			LE_DEBUG("No update on scan result for addr: %012llx", scanResult->btStationAddress);
			le_mem_Release(scanResult);
		} else {
			LE_DEBUG("Scan result for addr: %012llx updated", scanResult->btStationAddress);
			le_mem_Release(sCont->scanResult);														// in case the scan result has changed for one BT address and they are not equal we remove the old
			sCont->scanResult = scanResult;															// and store the new in the BT_Station_Container_t
		}

	} else {

		LE_DEBUG("HashMap did not contain addr: %012llx - adding entry", scanResult->btStationAddress);

		BT_Station_Container_t *sCont;
		LE_ASSERT((sCont =  le_mem_ForceAlloc(bTStationContainerPool)) != NULL);			// storage for the BT station container (which contains some side information + the scan result
																									// FIXME - not sure force le_mem_ForceAlloc is good here ...
		sCont->btStationAddress = scanResult->btStationAddress;
		sCont->isDirty = false;
		sCont->lastSeen = le_clk_GetRelativeTime();											// storing the new scanned device to the HasMap
		sCont->scanResult = scanResult;

		le_hashmap_Put(stationHashMap, &sCont->btStationAddress, sCont);
	}


}

