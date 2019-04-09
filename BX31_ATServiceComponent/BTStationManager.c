/*
 * BTStationManager.c
 *
 *  This is part of the "BX31_ATService" Project
 *  Created on: Apr 4, 2019
 *      Author: Thomas Schmidt, SWI
 */

#include "BX31_ATServiceComponent.h"
#include "BTStationManager.h"
#include "config_scanner.h"
#include "base64.h"


static le_hashmap_Ref_t stationHashMap = NULL;
static le_mem_PoolRef_t bTStationContainerPool = NULL;

static callbackOnAvsDataAdd_t avsDataAddCallback = NULL;
static callbackOnAvsDataPush_t avsDataPushCallback = NULL;

static unsigned int lastSeenStations = 0;

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
// FIXME not sure we should put this function here 
// - could be also in BX31_ATServiceComponent.c

int btmgr_ScanCmp (BTScanResult_t * scanResult1, BTScanResult_t * scanResult2)
{
        if (scanResult1->btStationAddress != scanResult2->btStationAddress)
                return 1;
        if (scanResult1->addrType != scanResult2->addrType)
                return 2;
        if (scanResult1->data_len != scanResult2->data_len)
                return 3;
        /* if(scanResult1->rssi != scanResult2->rssi) return 4; */                     // We don't compare RSSI !!
        // it changing permanently and it makes no sense to compare

        for (int i = 0; i <= scanResult1->data_len; ++i) {                      // Finally we check if the advertisement data differs  before
                // we checked that both scanReults have the same length
                if (scanResult1->advertData[i] != scanResult2->advertData[i])   // so it can be iterated and compared over the length
                        return 4 + i;

        }

        return 0;                                                               // if all tests failed - the both scan results are equal
}



/** ------------------------------------------------------------------------
 *
 * initializes the HashMap which contains information about the scanned
 * BT devices
 *
 * ------------------------------------------------------------------------
 */

void btmgr_init(callbackOnAvsDataAdd_t callbackOnAvsDataAdd, callbackOnAvsDataPush_t callbackOnAvsDataPush) {

        LE_DEBUG ("initializing BTStationManager");

        LE_ASSERT ((stationHashMap = le_hashmap_Create ("BX31_ATService.station.hashtable",
                        MAX_BT_STATION_HASHMAP_SIZE,
                        le_hashmap_HashUInt64,
                        le_hashmap_EqualsUInt64))
                        != NULL);

        le_hashmap_MakeTraceable (stationHashMap);

        bTStationContainerPool =
                        le_mem_CreatePool ("stationContainer",
                                        sizeof (BT_Station_Container_t));
        le_mem_ExpandPool (bTStationContainerPool,
                        MAX_SCANNED_STATION_MEM_POOL_SIZE);

        avsDataAddCallback = callbackOnAvsDataAdd;
        avsDataPushCallback = callbackOnAvsDataPush;
}

/** ------------------------------------------------------------------------
 *
 * Called for each scanned BT device. The given parameter contains a single
 * scanned station. The station is looked up based on it's BT address.
 * If the address is already known the last seen time and the RSSI is updated.
 * In case the advertisement packet was changed the complete data is updated
 *
 * @param scan result
 *
 * ------------------------------------------------------------------------
 */
void btmgr_updateList (BTScanResult_t * scanResult)
{

//        if (stationHashMap == NULL || bTStationContainerPool == NULL)
//                btmgr_init ();

        if (le_hashmap_ContainsKey(stationHashMap, 
                        &scanResult->btStationAddress)) {
                BT_Station_Container_t *sCont = NULL;

                LE_ASSERT ((sCont = le_hashmap_Get (stationHashMap, 
                                &scanResult->btStationAddress))
                                != NULL);                                             // HasMap lookup for the key (should be
                // there because we checked before)

                sCont->lastSeen = le_clk_GetAbsoluteTime ();
                sCont->scanResult->rssi = scanResult->rssi;                     // we don't throw away the old scan result
                // in case only the RSSI changed




                if (btmgr_ScanCmp (sCont->scanResult, scanResult) == 0) {           // in case the old and the new scan result
#ifdef DEBUG_BT
                        LE_DEBUG ("No update on scan result for addr: %012llx", // are equal the new one is removed from memory
                                        scanResult->btStationAddress);
#endif /* DEBUG_BT */
                        sCont->isDirty = false;
                        le_mem_Release (scanResult);
                } else {
#ifdef DEBUG_BT
                        LE_DEBUG ("Scan result for addr: %012llx updated",
                                        scanResult->btStationAddress);
#endif /* DEBUG_BT */
                        le_mem_Release (sCont->scanResult);                     // in case the scan result has changed for one BT
                        // address and they are not equal we remove the old

                        sCont->isDirty = true;
                        sCont->scanResult = scanResult;                         // and store the new in the BT_Station_Container_t
                }

        } else {

#ifdef DEBUG_BT
                LE_DEBUG("HashMap did not contain addr: %012llx - adding entry",
                                scanResult->btStationAddress);
#endif /* DEBUG_BT */

                BT_Station_Container_t *sCont;
                LE_ASSERT ((sCont = le_mem_ForceAlloc (bTStationContainerPool)) // storage for the BT station container (which contains
                                != NULL);                                       // some side information + the scan result
                                                                                // FIXME - not sure force le_mem_ForceAlloc is good here
                                                                                // may TryAlloc is better and handle a negative outcome
                                                                                // accordingly


                sCont->btStationAddress = scanResult->btStationAddress;
                sCont->isDirty = true;

                sCont->lastSeen = le_clk_GetAbsoluteTime ();                    // storing the new scanned device to the HasMap

                sCont->scanResult = scanResult;

                le_hashmap_Put (stationHashMap, &sCont->btStationAddress, sCont);
        }


}


/** ------------------------------------------------------------------------
 *
 *
 * ------------------------------------------------------------------------
 */

void btmgr_periodicalCheck()  {
        const uint64_t *nextKey;
        BT_Station_Container_t *nextVal;
        unsigned int stationCount = 0, removedStations = 0;
        char pathBuffer[MAX_PATH_BUFFER_LEN];
        char encodedStringBuffer[LE_BASE64_ENCODED_SIZE(MAX_BT_DATA_STRING_SIZE) + 1];


        LE_INFO("checking periodically BT station List");

        le_clk_Time_t now = le_clk_GetAbsoluteTime();
        le_clk_Time_t diffTime;
        diffTime.sec = MAX_BT_STATION_AGE;
        diffTime.usec = 0;

        le_hashmap_It_Ref_t hashMapIterator = le_hashmap_GetIterator(stationHashMap);

        while (LE_OK == le_hashmap_NextNode(hashMapIterator)) {
                nextKey = le_hashmap_GetKey(hashMapIterator);
                nextVal = le_hashmap_GetValue(hashMapIterator);


                if(le_clk_GreaterThan(now,le_clk_Add(nextVal->lastSeen, diffTime))) {
                        LE_DEBUG("free BT station from HashMap %012llx", *nextKey);
                        le_mem_Release(nextVal->scanResult);
                        le_mem_Release(nextVal);
                        le_hashmap_Remove(stationHashMap, nextKey);
                        ++removedStations;
                } else  {
                        snprintf(pathBuffer, MAX_PATH_BUFFER_LEN, AVS_STATION_PATH ".%012llx.lastseen", nextVal->btStationAddress);
                        avsDataAddCallback(pathBuffer, &nextVal->lastSeen, INT);

                        snprintf(pathBuffer, MAX_PATH_BUFFER_LEN, AVS_STATION_PATH ".%012llx.rssi", nextVal->btStationAddress);
                        avsDataAddCallback(pathBuffer, &nextVal->scanResult->rssi, INT);

                        if(nextVal->isDirty) {
                                snprintf(pathBuffer, MAX_PATH_BUFFER_LEN, AVS_STATION_PATH ".%012llx.addrType", nextVal->btStationAddress);
                                avsDataAddCallback(pathBuffer, &nextVal->scanResult->addrType, INT);

                                snprintf(pathBuffer, MAX_PATH_BUFFER_LEN, AVS_STATION_PATH ".%012llx.dataLen", nextVal->btStationAddress);
                                avsDataAddCallback(pathBuffer, &nextVal->scanResult->data_len, INT);


                                unsigned int len = LE_BASE64_ENCODED_SIZE(MAX_BT_DATA_STRING_SIZE) + 1;
                                memset(encodedStringBuffer,0,len);

                                le_result_t b64result = le_base64_Encode((uint8_t * ) nextVal->scanResult->advertData ,nextVal->scanResult->data_len,encodedStringBuffer,&len);

                                if(b64result == LE_OK) {
                                        snprintf(pathBuffer, MAX_PATH_BUFFER_LEN, AVS_STATION_PATH ".%012llx.data", nextVal->btStationAddress);
                                        avsDataAddCallback(pathBuffer, encodedStringBuffer, STRING);

                                } else {
                                        LE_WARN("could not convert binary to base64: %d", b64result);
                                }

                        }

                }


                ++stationCount;
        }

        unsigned int stationsAfterCleanup =  stationCount-removedStations;
        unsigned int stationsAdded =  ( stationCount - lastSeenStations) > 0 ? // are there stations added ? then print the
                        stationCount - lastSeenStations : 0;                    // number - otherwise we don't print negative number

        LE_INFO("BTstat: Stations in list=%u; "
                        "After cleanup=%u; removed Stations=%u; added stations=%u; last seen=%u",
                        stationCount,  stationsAfterCleanup, removedStations,
                        stationsAdded, lastSeenStations);

// TODO - put here the Update to AVS !!!!

        if(avsDataAddCallback != NULL) {
                avsDataAddCallback(AVS_STATISTICS_PATH ".stations.count", &stationCount, INT);
                avsDataAddCallback(AVS_STATISTICS_PATH ".stations.countAfterCleanup", &stationsAfterCleanup, INT);
                avsDataAddCallback(AVS_STATISTICS_PATH ".stations.removed", &removedStations, INT);
                avsDataAddCallback(AVS_STATISTICS_PATH ".stations.added", &stationsAdded, INT);
                avsDataPushCallback();
        } else  {
                LE_WARN("callback not set, can't record data: %s", AVS_STATISTICS_PATH ".*" );
        }

        lastSeenStations = stationsAfterCleanup;

}

/** ------------------------------------------------------------------------
 *
 * destroys the allocated data from the HashMap inclusive content
 * to release memory typically on process exit
 *
 * ------------------------------------------------------------------------
 */
void btmgr_destroy() {
        const uint64_t *nextKey;
        BT_Station_Container_t *nextVal;

        LE_INFO("free BT stations from HashMap");
        le_hashmap_It_Ref_t hashMapIterator = le_hashmap_GetIterator(stationHashMap);

        while (LE_OK == le_hashmap_NextNode(hashMapIterator)) {
                nextKey = le_hashmap_GetKey(hashMapIterator);
                nextVal = le_hashmap_GetValue(hashMapIterator);
                LE_DEBUG("free BT station from HashMap %012llx", *nextKey);

                le_mem_Release(nextVal->scanResult);
                le_mem_Release(nextVal);

        }
        le_hashmap_RemoveAll(stationHashMap);
        avsDataAddCallback = NULL;
}

