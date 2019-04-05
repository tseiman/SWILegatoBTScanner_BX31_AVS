
/*
 * BX31_ATServiceComponent.c
 *
 * This implements all interaction with the BX310x AT interface.
 * The BXModule is configured on initialization to be passive
 * (no own BT advertisement) WiFi of the BX31 module is disabled
 *
 * The ScanBLE function can be called and the BX310x module will
 * perform a BT scan. Once stations are found it will callback to
 * a previously provided function. Each found station is parsed,
 * the AT unsolicited scan message is binary packed and stored in
 * an allocated struct this struct is given as a parameter to the
 * callback function
 *
 * CAREFUL: the BTScanResult_t struct which is given to callback
 * function is allocated from pool - but _not_ freed here
 * - take care of freeing it outside of this module
 *
 *  This is part of the "BX31_ATService" Project
 *  Created on: Feb 21, 2019
 *      Author: Thomas Schmidt, SWI
 */

#include "BX31_ATServiceComponent.h"
#include "legato.h"
#include "interfaces.h"
#include <unistd.h>
#include <string.h>

le_atClient_DeviceRef_t DevRef;
le_atClient_UnsolicitedResponseHandlerRef_t UnsolCmtiRef;
static le_atClient_CmdRef_t cmdRef;
static int fd = 0;		                                                       // BX31 (Module UART1) serial device file descriptor


static void (*callback) (int, BTScanResult_t *) = NULL;	                       // this callback is called in case a BT scan was
                                                                               // received from AT CLI


le_mem_PoolRef_t scannedBTStationsPool;

/** ------------------------------------------------------------------------
 *
 * Is called to initialize the BT scanner
 *
 *  @param 	callback in case advertisement messages are collected, callback
 *			will be called for each advertisement and after ScanBLE was called.
 *			CAREFUL: the BTScanResult_t struct which is given as parameter to
 *			callback function is allocated from pool - but _not_ freed here
 *			- take care of freeing it outside of this module
 *
 *
 * -------------------------------------------------------------------------
 */

void initBLE(void (*callbackOnScan) (int, BTScanResult_t *))
{

	char buffer[LE_ATDEFS_RESPONSE_MAX_BYTES];
	memset(buffer, 0, LE_ATDEFS_RESPONSE_MAX_BYTES);

	LE_INFO("Initializing BX31 AT interface");

	LE_ASSERT((scannedBTStationsPool =
		   le_mem_CreatePool("BX31_ATService.station.scan",
				     sizeof(BTScanResult_t))) != NULL);
	le_mem_ExpandPool(scannedBTStationsPool,
			  MAX_SCANNED_STATION_MEM_POOL_SIZE);

	callback = callbackOnScan;

	fd = le_tty_Open("/dev/ttyHS0", O_RDWR | O_NDELAY
			                        | O_NOCTTY | O_NONBLOCK);	               // opening the UART2 - which is connected to the
	if (fd == -1) {		// IoT Board
		LE_ERROR("failed to open UART device");
		exit(EXIT_FAILURE);
	}

	LE_ASSERT(le_tty_SetBaudRate(fd, LE_TTY_SPEED_115200) == LE_OK);	       // assuming BX31 is on 115200
	LE_ASSERT(le_tty_SetFraming(fd, 'N', 8, 1) == LE_OK);	                   // set UART framing to 8bit, No Parity, 1 Stop bit
	LE_ASSERT(le_tty_SetRaw(fd, 10, 30000) == LE_OK);	                       // We need raw UART mode (the canonical echo's
                                                                               // back to peer)

	int newFd = dup(fd);	                                                   // we need to duplicate the file descriptor because
	DevRef = le_atClient_Start(fd);	                                           // we are checking two times if there is still an
	                                                                           // open atClient

	/* Try to stop the device */
	LE_ASSERT(le_atClient_Stop(DevRef) == LE_OK);	                           // checking twice
	LE_ASSERT(le_atClient_Stop(DevRef) == LE_FAULT);
	DevRef = le_atClient_Start(newFd);

	cmdRef = le_atClient_Create();	                                           // instantiate an AT interpreter client

	/* ======= Here first BX31 checks and initialization starts */

	/* --- Clean Up a may messed AT interface --- */
	LE_INFO("Cleaning AT interface");	                                       // Just send an empty AT command to get the interface clean

	LE_ASSERT(le_atClient_SetCommandAndSend
		  (&cmdRef, DevRef, "AT", "", "OK|ERROR|+CME ERROR",
		   LE_ATDEFS_COMMAND_DEFAULT_TIMEOUT) == LE_OK);

	LE_ASSERT(le_atClient_GetFinalResponse(cmdRef,
		   buffer, LE_ATDEFS_RESPONSE_MAX_BYTES) == LE_OK);	                   // We don't care about the result

	memset(buffer, 0, 50);

	/* --- check if we're really talking to BX31 --- */
	LE_INFO("Check BX310x presence");	                                       // We ask for device identification string

	LE_ASSERT(le_atClient_SetCommandAndSend
		  (&cmdRef, DevRef, "ATI", "", "OK|ERROR|+CME ERROR",
		   LE_ATDEFS_COMMAND_DEFAULT_TIMEOUT) == LE_OK);

	LE_ASSERT(le_atClient_GetFinalResponse
		  (cmdRef, buffer, LE_ATDEFS_RESPONSE_MAX_BYTES) == LE_OK);

	memset(buffer, 0, 50);

	LE_ASSERT(le_atClient_GetFirstIntermediateResponse
		  (cmdRef, buffer, LE_ATDEFS_RESPONSE_MAX_BYTES) == LE_OK);
	/*   LE_ASSERT(le_atClient_GetNextIntermediateResponse(cmdRef,buffer, LE_ATDEFS_RESPONSE_MAX_BYTES)
	   == LE_NOT_FOUND); */

	if (strncmp(buffer, "BX310", 4) != 0) {	                                   // and compare it with the expected
		LE_ERROR("This doesn seem to be a BX310x bluetooth module, "
				"exiting. Response was \"%s\"", buffer);	                   // if the BT module is not answering we better exit

		LE_ASSERT(le_atClient_Delete(cmdRef) == LE_OK);	                       // there might be a better strategy to recover eg.
		le_tty_Close(newFd);	                                               // an uninitialized Module
		exit(EXIT_FAILURE);
	}
	memset(buffer, 0, 50);

	/* --- Disable WiFi --- */
	LE_INFO("Disable WiFi on BX310x");	                                       // We use the BX just for BT scanning
	LE_ASSERT(le_atClient_SetCommandAndSend
		  (&cmdRef, DevRef, "AT+SRWCFG=0", "", "OK|ERROR|+CME ERROR",
		   LE_ATDEFS_COMMAND_DEFAULT_TIMEOUT) == LE_OK);

	LE_ASSERT(le_atClient_GetFinalResponse
		  (cmdRef, buffer, LE_ATDEFS_RESPONSE_MAX_BYTES) == LE_OK);

	memset(buffer, 0, 50);

	/* --- Enable BT --- */
	LE_DEBUG("Enable BT on BX310x");

	LE_ASSERT(le_atClient_SetCommandAndSend
		  (&cmdRef, DevRef, "AT+SRBTSYSTEM=1", "",
		   "OK|ERROR|+CME ERROR",
		   LE_ATDEFS_COMMAND_DEFAULT_TIMEOUT) == LE_OK);

	LE_ASSERT(le_atClient_GetFinalResponse
		  (cmdRef, buffer, LE_ATDEFS_RESPONSE_MAX_BYTES) == LE_OK);

	memset(buffer, 0, 50);

	/* --- Disable BT power save --- */
	LE_DEBUG("Disable BT power save for BX310x");

	LE_ASSERT(le_atClient_SetCommandAndSend
		  (&cmdRef, DevRef, "AT+SRBTPS=0", "", "OK|ERROR|+CME ERROR",
		   LE_ATDEFS_COMMAND_DEFAULT_TIMEOUT) == LE_OK);

	LE_ASSERT(le_atClient_GetFinalResponse
		  (cmdRef, buffer, LE_ATDEFS_RESPONSE_MAX_BYTES) == LE_OK);

	memset(buffer, 0, 50);

	/* --- No own advertising --- */
	LE_INFO("Disable own BT advertising on BX310x");

	le_atClient_SetCommandAndSend(&cmdRef, DevRef, "AT+SRBLEADV=1", "",
				      "OK|ERROR|+CME ERROR",
				      LE_ATDEFS_COMMAND_DEFAULT_TIMEOUT);

	//      LE_ASSERT(le_atClient_GetFinalResponse( cmdRef,buffer, LE_ATDEFS_RESPONSE_MAX_BYTES) == LE_OK);
	//      memset(buffer,0,50);

	LE_ASSERT(le_atClient_SetCommandAndSend
		  (&cmdRef, DevRef, "AT+SRBLEADV=0", "", "OK|ERROR|+CME ERROR",
		   LE_ATDEFS_COMMAND_DEFAULT_TIMEOUT) == LE_OK);

	LE_ASSERT(le_atClient_GetFinalResponse
		  (cmdRef, buffer, LE_ATDEFS_RESPONSE_MAX_BYTES) == LE_OK);

	memset(buffer, 0, 50);

}

/** ------------------------------------------------------------------------
 * Stops the BX31 AT Service, releases all resources
 * -------------------------------------------------------------------------
 */
void stopBLE()
{
	LE_INFO("Stopping BX_AT");
	LE_ASSERT(le_atClient_Delete(cmdRef) == LE_OK);
	le_tty_Close(fd);	                                                       // Close the serial file descriptor
}

/** ------------------------------------------------------------------------
 *
 * Getter for command reference this is required for timer
 *
 * -------------------------------------------------------------------------
 */
// FIXME timer and command reference could be better isolated

le_atClient_CmdRef_t getCmdRef()
{
	return cmdRef;
}

/** ------------------------------------------------------------------------
 *
 * Converts a Bluetooth address String (eventually in quotes)
 * into an integer representation
 *  @param *addrStr - pointer to address string
 *
 *  @return uint64 integer representation of the address string
 *
 * ------------------------------------------------------------------------
 */

uint64_t btAddrToInt(char *addrStr)
{

	uint64_t result = 0;

	if (addrStr == NULL) {
		LE_ERROR("NULL pointer given for addr string");
		return 0;
	}

	if (strnlen(addrStr, 20) < 19) {	                                       // addr str including quotes
		LE_ERROR("Problem parsing BT address to int Addr Str was: %s",
			 addrStr);
		return 0;
	}

	if (addrStr[0] == '\"') ++addrStr;	                                       // remove leading quote
	if (addrStr[17] == '\"') addrStr[17] = 0;	                               // remove tailing quote

	int oct[6];
	if (sscanf
	    (addrStr, "%x:%x:%x:%x:%x:%x", &oct[5], &oct[4], &oct[3], &oct[2],
	     &oct[1], &oct[0]) < 6) {

		LE_ERROR("Problem parsing BT address to int Addr Str was: %s",
			 addrStr);

		return 0;
	}

	for (int i = 5; i >= 0; i--) {
		result |= (uint64_t) oct[i] << (CHAR_BIT * i);
	}

	LE_DEBUG("Converted address string \"%s\" to int: \"%012llx\"", addrStr,
		 result);

	return result;
}

/** ------------------------------------------------------------------------
 *
 *  Converts the escaped BT advertisment string - e.g. "\1E\FF\06" to a
 *  binary representation 0x1e 0xff 0x06 ...
 *
 *  @param the escaped source string
 *  @param the buffer where to write to
 *
 *  @return length of converted string
 *
 * -------------------------------------------------------------------------
 */

int escapedAdvrtStr2Binary(char *escapedStrIn, char *dstBuffer)
{

	if (escapedStrIn == NULL) {
		LE_ERROR("NULL pointer given for escaped string ");
		return 0;
	}

	if (escapedStrIn[0] == '\"') ++escapedStrIn;                               // remove leading quote if present

	if (escapedStrIn
	    [strnlen(escapedStrIn, MAX_BT_DATA_STRING_SIZE * 3 + 1) - 1] ==
	    '\"')
		escapedStrIn[strnlen(escapedStrIn, MAX_BT_DATA_STRING_SIZE * 3 + 1) - 1] = 0;	// remove tailing quote if present

	LE_DEBUG("got ESC str: %s", escapedStrIn);

	LE_DEBUG("ESC strLen: %d",
		 strnlen(escapedStrIn, MAX_BT_DATA_STRING_SIZE * 3 + 1));

	int numberOfEscapedCharacters = 0;

	int escapedStrlen =
	    strnlen(escapedStrIn, MAX_BT_DATA_STRING_SIZE * 3 + 1);
	for (int i = 0; i <= escapedStrlen; ++i) {	                               // counting the number of escaped bytes in the
		                                                                       // advertisement message
		if (escapedStrIn[i] == '\\') {	                                       // MAX_BT_DATA_STRING_SIZE is the maximum size in
			                                                                   // bytes of a Advertisement message,
			++numberOfEscapedCharacters;	                                   // escaped it is the ASCII plus backslash = 3 ASCII
			                                                                   // characters per byte
			escapedStrIn[i] = 0;	                                           // to get the number of bytes in the string we count
			                                                                   // the number of backslashes
		}
	}

	for (int i = 0; i < numberOfEscapedCharacters; ++i) {	                   // read the escaped advertisement string and convert
		                                                                       // it to binary
		dstBuffer[i] =
		    (char)strtol(escapedStrIn + (i * 3) + 1, NULL, 16);
	}

#ifdef DEBUG
	char dbgBuffer[MAX_BT_DATA_STRING_SIZE * 5 + 1];
	memset(dbgBuffer, 0, MAX_BT_DATA_STRING_SIZE * 5 + 1);
	for (int i = 0; i < numberOfEscapedCharacters; ++i) {
		sprintf(dbgBuffer + (5 * i), " 0x%02x", dstBuffer[i]);
	}
	LE_DEBUG("packed buffer: %s", dbgBuffer);

#endif				/* DEBUG */

	LE_DEBUG("converted %d chars", numberOfEscapedCharacters);

	return numberOfEscapedCharacters;
}

/** ------------------------------------------------------------------------
 *
 * takes the full unsolicited +SRBLESCAN message from BX31 BT scan and
 * extracts the content
 *
 * the unsolicited message looks like:
 * +SRBLESCAN: "29:db:3c:cd:01:5a",   1,     -53,"\1E\FF\06\00\01\09\20\02\77\62\22\BC\EE\52\C2\BE\85\AB\73\AF\FB\A9\64\26\AE\EE\8D"
 * \----v----/ \-------v-------/      v       v   \--------------------------------------v-----------------------------------------/
 * AT preamble      BT addr       Addr Type  RSSI                                    Advert Data
 *
 * @return struct containing the content of unsolicited
 *  +SRBLESCAN message already binary packed
 *
 * -------------------------------------------------------------------------
 */
BTScanResult_t *tokenizeScanResult(char *buffer)
{

	if (strncmp(buffer, "+SRBLESCAN: ", 12) != 0) {
		LE_ERROR("The given buffer seems not to contain a BX31 scan result, "
		    	"it should start with \"+SRBLESCAN:\", given buffer was \"%s\".",
		     buffer);

		return NULL;
	}

	BTScanResult_t *scanResult;
	scanResult = le_mem_TryAlloc(scannedBTStationsPool);	                   // storage for the result
	if (scanResult == NULL) {

		LE_ERROR("Problem to allocate memory from Pool "
				"for tokenizeScanResult");

		return NULL;
	}

	char *parameter = strtok(buffer + 12, ",");	                               // get the first parameter (BT address) from the
	                                                                           // unsolicited string
	if ((scanResult->btStationAddress = btAddrToInt(parameter)) == 0) {

		LE_ERROR("Problem to tokenize BL Scan String , "
				"could not extract BL Address");

		le_mem_Release(scanResult);
		return NULL;
	}

	parameter = strtok(NULL, ",");	                                           // get the address type from the unsolicited string
	if (parameter == NULL) {

		LE_ERROR("Problem to tokenize BL Scan String , "
		    	"could not extract BL Address type");

		le_mem_Release(scanResult);
		return NULL;
	}

	scanResult->addrType = strtol(parameter, NULL, 10);
	if ((scanResult->addrType != BX31_BT_PUBLIC_ADDR)
	    && (scanResult->addrType != BX31_BT_PRIVATE_ADDR)) {

		LE_ERROR("Problem to tokenize BL Scan String , "
				"could not extract BL Address type");

		le_mem_Release(scanResult);
		return NULL;
	}

	parameter = strtok(NULL, ",");	                                           // get RSSI from the unsolicited string
	if (parameter == NULL) {

		LE_ERROR("Problem to tokenize BL Scan String , "
				"could not extract RSSI");

		le_mem_Release(scanResult);
		return NULL;
	}
	scanResult->rssi = strtol(parameter, NULL, 10);

	parameter = strtok(NULL, ",");	                                           // get BT advertisement data from the unsolicited
	                                                                           // string and pack it (string to binary)
	if (parameter == NULL) {

		LE_ERROR("Problem to tokenize BL Scan String , "
				"could not extract RSSI");

		le_mem_Release(scanResult);
		return NULL;
	}
	scanResult->data_len =
	    escapedAdvrtStr2Binary(parameter, scanResult->advertData);

	return scanResult;
}

/** ------------------------------------------------------------------------
 *
 * called by timer periodically to perform the BT scan
 * In case unsolicited messages with BT scan results are received
 * a callback is performed
 * CAREFUL: the BTScanResult_t struct which is given as parameter to
 * callback function is allocated from pool - but _not_ freed here
 * - take care of freeing it outside of this module
 *
 * @param reference to the calling timer  //FIXME this could be better isolated
 *
 * ------------------------------------------------------------------------
 */
void ScanBLE(le_timer_Ref_t timerRef)
{

	char buffer[LE_ATDEFS_RESPONSE_MAX_BYTES];
	memset(buffer, 0, LE_ATDEFS_RESPONSE_MAX_BYTES);

	le_atClient_CmdRef_t cmdRef =
	    (le_atClient_CmdRef_t) le_timer_GetContextPtr(timerRef);

	/* --- Run BT Scan  --- */
	LE_DEBUG("run the BT Scan");	                                           // We use the BX just for BT scanning

	LE_ASSERT(le_atClient_SetCommandAndSend
		  (&cmdRef, DevRef, "AT+SRBLESCAN=5,1", "",
		   "OK|ERROR|+CME ERROR",
		   LE_ATDEFS_COMMAND_DEFAULT_TIMEOUT) == LE_OK);

	LE_ASSERT(le_atClient_GetFinalResponse
		  (cmdRef, buffer, LE_ATDEFS_RESPONSE_MAX_BYTES) == LE_OK);

	LE_DEBUG("Final response after Scan: %s", buffer);

	if (strcmp(buffer, "OK") == 0) {
		memset(buffer, 0, 50);

		le_result_t res =
		    le_atClient_GetFirstIntermediateResponse(cmdRef, buffer,
							     LE_ATDEFS_RESPONSE_MAX_BYTES);

		int intNumber = 1;

		while (res == LE_OK) {
			if (callback != NULL) {
				BTScanResult_t *scanResult =
				    tokenizeScanResult(buffer);
				callback(intNumber, scanResult);
			} else {
				LE_WARN("BT Callback NOT SET - got BT Scan  %d: %s",
				     intNumber, buffer);
			}

			memset(buffer, 0, LE_ATDEFS_RESPONSE_MAX_BYTES);	               // FIXME - needs optimization
			intNumber++;

			res =
			    le_atClient_GetNextIntermediateResponse(cmdRef,
								    buffer,
								    LE_ATDEFS_RESPONSE_MAX_BYTES);
		}

	}
}
