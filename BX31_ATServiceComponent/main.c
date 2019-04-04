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

static le_timer_Ref_t scanTimer = NULL;


void scanCallback(int index,  BTScanResult_t *scanResult) {

	if(scanResult->data_len == 0) return;

#ifdef DEBUG
	char buffer[MAX_BT_DATA_STRING_SIZE *3 +1];
	memset(buffer,0,MAX_BT_DATA_STRING_SIZE *3 +1);
	for(int i = 0; i< scanResult->data_len; ++i) {
		sprintf(buffer + (3*i), "%02x ", scanResult->advertData[i]);
	}
	LE_INFO("Got BT Scan  %d: addr=%012llx, addrType=%d, rssi=%d, data_len=%d, data:", index, scanResult->addressInt, scanResult->addrType, scanResult->rssi, scanResult->data_len);
	LE_INFO("%s", buffer);
#endif /* DEBUG */

	updateList(scanResult);


}

/**
 *
 *   Handle System Signals on termination of the legato application
 *
 */

static void SigHandler(int signal) {
	LE_INFO("Terminating BX31_ATService");
	le_timer_Stop(scanTimer);
	stopBLE();
}

/**
 *
 * Initialization ("main") routine
 *
 *
 */
COMPONENT_INIT
{
	char buffer[LE_ATDEFS_RESPONSE_MAX_BYTES];
	memset(buffer,0,LE_ATDEFS_RESPONSE_MAX_BYTES);


	LE_INFO("Start BX31_ATService");

	le_sig_Block(SIGINT);								// catch the termination of the Application
	le_sig_SetEventHandler(SIGINT, SigHandler);			// to clean up allocated resources

	initBLE(scanCallback);								// initialize the BX31 Module for BT scanning, callback is called on scan events
	le_atClient_CmdRef_t cmdRef = getCmdRef();			// the timer needs the reference to the command it needs to be executed



	scanTimer = le_timer_Create("scanBleTimer");		// set up a Legato Timer
	le_timer_SetContextPtr(scanTimer, cmdRef);
    le_timer_SetHandler(scanTimer, ScanBLE);
    le_timer_SetRepeat(scanTimer, 0);
    le_timer_SetMsInterval(scanTimer, 10000);
    le_timer_Start(scanTimer);

}
