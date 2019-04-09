/*
 * main.c
 *
 *  Created on: Apr 3, 2019
 *      Author: Thomas Schmidt, SWI
 */

#include "legato.h"
#include "interfaces.h"
#include <unistd.h>
#include <signal.h>

#include "BX31_ATServiceComponent.h"
#include "BTStationManager.h"
#include "AVSInterface.h"

static le_timer_Ref_t scanTimer = NULL;
static le_timer_Ref_t btStationJanitorTimer = NULL;


/** ------------------------------------------------------------------------
 *
 * callback - called data set should be queued for AirVantage
 *
 * ------------------------------------------------------------------------
 */

void main_addDataToAvsCallback(char *path, void *data, avsService_DataType_t type) {

        switch(type)  {
        case INT:  LE_INFO("got AVS callback path=%s; data=%d", path, *(int*) data); break;
        case FLOAT: LE_INFO("got AVS callback path=%s; data=%f", path, *(double*) data); break;
        case BOOL:  LE_INFO("got AVS callback path=%s; data=%s", path, (((*(bool*) data)) == true) ? "true" : "false") ; break;
        case STRING:  LE_INFO("got AVS callback path=%s; data=%s", path, (char *) data); break;
        default: LE_INFO("got AVS callback path=%s; unknown data ", path); break;
        }

        avsService_recordData(path, data, type);

}


/** ------------------------------------------------------------------------
 *
 * callback - called data set should be send to AirVantage
 *
 * ------------------------------------------------------------------------
 */

void main_pushDataToAvsCallback() {
#ifndef TEST_DRYRUN
        avsService_pushData();
#endif
}




/** ------------------------------------------------------------------------
 *
 * callback - called for each BT scan result
 *
 * ------------------------------------------------------------------------
 */

void main_scanCallback(int index, BTScanResult_t * scanResult) {

        if (scanResult->data_len == 0) return;

#ifdef DEBUG_MAIN
        char buffer[MAX_BT_DATA_STRING_SIZE * 3 + 1];
        memset(buffer, 0, MAX_BT_DATA_STRING_SIZE * 3 + 1);

        for (int i = 0; i < scanResult->data_len; ++i) {
                sprintf(buffer + (3 * i), "%02x ", scanResult->advertData[i]);
        }

        LE_INFO("Got BT Scan  %d: addr=%012llx, addrType=%d, rssi=%d, data_len=%d, data:",
             index, scanResult->btStationAddress, scanResult->addrType,
             scanResult->rssi, scanResult->data_len);

        LE_INFO("%s", buffer);
#endif                                /* DEBUG */

        btmgr_updateList(scanResult);

}

/** ------------------------------------------------------------------------
 *
 *   Handle System Signals on termination of the legato application
 *
 * ------------------------------------------------------------------------
 */

static void main_SigHandler(int signal) {
        LE_INFO("Terminating BX31_ATService");
        if(scanTimer != NULL) le_timer_Stop(scanTimer);
        if(btStationJanitorTimer != NULL) le_timer_Stop(btStationJanitorTimer);
        bx31at_stopBLE();
        btmgr_destroy();
        avsService_detroy();
}


/** ------------------------------------------------------------------------
 *
 * Initialization ("main") routine
 *
 * ------------------------------------------------------------------------
 */
COMPONENT_INIT {
        char buffer[LE_ATDEFS_RESPONSE_MAX_BYTES];
        memset(buffer, 0, LE_ATDEFS_RESPONSE_MAX_BYTES);


        LE_INFO("Start BX31_ATService");

        le_sig_Block(SIGINT);                                                   // catch the termination of the Application
        le_sig_SetEventHandler(SIGINT, main_SigHandler);                        // to clean up allocated resources
        le_sig_Block(SIGTERM);                                                   // catch the termination of the Application
        le_sig_SetEventHandler(SIGTERM, main_SigHandler);                        // to clean up allocated resources

        avsService_init();

        btmgr_init(main_addDataToAvsCallback, main_pushDataToAvsCallback);

        bx31at_initBLE(main_scanCallback);                                      // initialize the BX31 Module for BT scanning,
                                                                                // callback is called on scan events
        le_atClient_CmdRef_t cmdRef = bx31at_getCmdRef();                       // the timer needs the reference to the command
                                                                                // it needs to be executed


        scanTimer = le_timer_Create("scanBleTimer");                            // set up a Timer to scan BT
        le_timer_SetContextPtr(scanTimer, cmdRef);
        le_timer_SetHandler(scanTimer, bx31at_ScanBLE);
        le_timer_SetRepeat(scanTimer, 0);
        le_timer_SetMsInterval(scanTimer, 10000);
        le_timer_Start(scanTimer);


        btStationJanitorTimer = le_timer_Create("cleanBtStationsTimer");        // set up Timer to clean the BT station Database
   //     le_timer_SetContextPtr(btStationJanitorTimer, ?????cmdRef);           // and to find stations have not been seen
        le_timer_SetHandler(btStationJanitorTimer, btmgr_periodicalCheck);      // for a period of time
        le_timer_SetRepeat(btStationJanitorTimer, 0);                           // on each cleanup changes are as well reported
        le_timer_SetMsInterval(btStationJanitorTimer, MAX_BT_STATION_AGE * 1000);  // to AirVantage
        le_timer_Start(btStationJanitorTimer);


}


