

/*
 * ====================== WARNING ======================
 *
 * THE CONTENTS OF THIS FILE HAVE BEEN AUTO-GENERATED.
 * DO NOT MODIFY IN ANY WAY.
 *
 * ====================== WARNING ======================
 */

/**
 * @file le_atDefs_interface.h
 *
 * Definition file common to multiple AT commands related services.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef LE_ATDEFS_INTERFACE_H_INCLUDE_GUARD
#define LE_ATDEFS_INTERFACE_H_INCLUDE_GUARD


#include "legato.h"


//--------------------------------------------------------------------------------------------------
/**
 * Type for handler called when a server disconnects.
 */
//--------------------------------------------------------------------------------------------------
typedef void (*le_atDefs_DisconnectHandler_t)(void *);

//--------------------------------------------------------------------------------------------------
/**
 *
 * Connect the current client thread to the service providing this API. Block until the service is
 * available.
 *
 * For each thread that wants to use this API, either ConnectService or TryConnectService must be
 * called before any other functions in this API.  Normally, ConnectService is automatically called
 * for the main thread, but not for any other thread. For details, see @ref apiFilesC_client.
 *
 * This function is created automatically.
 */
//--------------------------------------------------------------------------------------------------
void le_atDefs_ConnectService
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 *
 * Try to connect the current client thread to the service providing this API. Return with an error
 * if the service is not available.
 *
 * For each thread that wants to use this API, either ConnectService or TryConnectService must be
 * called before any other functions in this API.  Normally, ConnectService is automatically called
 * for the main thread, but not for any other thread. For details, see @ref apiFilesC_client.
 *
 * This function is created automatically.
 *
 * @return
 *  - LE_OK if the client connected successfully to the service.
 *  - LE_UNAVAILABLE if the server is not currently offering the service to which the client is
 *    bound.
 *  - LE_NOT_PERMITTED if the client interface is not bound to any service (doesn't have a binding).
 *  - LE_COMM_ERROR if the Service Directory cannot be reached.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_atDefs_TryConnectService
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Set handler called when server disconnection is detected.
 *
 * When a server connection is lost, call this handler then exit with LE_FATAL.  If a program wants
 * to continue without exiting, it should call longjmp() from inside the handler.
 */
//--------------------------------------------------------------------------------------------------
void le_atDefs_SetServerDisconnectHandler
(
    le_atDefs_DisconnectHandler_t disconnectHandler,
    void *contextPtr
);

//--------------------------------------------------------------------------------------------------
/**
 *
 * Disconnect the current client thread from the service providing this API.
 *
 * Normally, this function doesn't need to be called. After this function is called, there's no
 * longer a connection to the service, and the functions in this API can't be used. For details, see
 * @ref apiFilesC_client.
 *
 * This function is created automatically.
 */
//--------------------------------------------------------------------------------------------------
void le_atDefs_DisconnectService
(
    void
);


//--------------------------------------------------------------------------------------------------
/**
 * Command string maximum length.
 *
 * @warning Special characters cost more than one byte
 *
 */
//--------------------------------------------------------------------------------------------------
#define LE_ATDEFS_COMMAND_MAX_LEN 511

//--------------------------------------------------------------------------------------------------
/**
 * Command string maximum length.
 * One extra byte is added for the null character.
 */
//--------------------------------------------------------------------------------------------------
#define LE_ATDEFS_COMMAND_MAX_BYTES 512

//--------------------------------------------------------------------------------------------------
/**
 * Parameter string maximum length.
 *
 * @warning Special characters cost more than one byte
 *
 */
//--------------------------------------------------------------------------------------------------
#define LE_ATDEFS_PARAMETER_MAX_LEN 127

//--------------------------------------------------------------------------------------------------
/**
 * Parameter string maximum length.
 * One extra byte is added for the null character.
 */
//--------------------------------------------------------------------------------------------------
#define LE_ATDEFS_PARAMETER_MAX_BYTES 128

//--------------------------------------------------------------------------------------------------
/**
 * AT command response maximum length.
 *
 * @warning Special characters cost more than one byte
 *
 */
//--------------------------------------------------------------------------------------------------
#define LE_ATDEFS_RESPONSE_MAX_LEN 512

//--------------------------------------------------------------------------------------------------
/**
 * AT command response maximum length.
 * One extra byte is added for the null character.
 */
//--------------------------------------------------------------------------------------------------
#define LE_ATDEFS_RESPONSE_MAX_BYTES 513

//--------------------------------------------------------------------------------------------------
/**
 * Maximum number of bytes in an unsolicited response (not including the null-terminator).
 *
 * @warning Special characters cost more than one byte
 *
 */
//--------------------------------------------------------------------------------------------------
#define LE_ATDEFS_UNSOLICITED_MAX_LEN 256

//--------------------------------------------------------------------------------------------------
/**
 * Maximum number of bytes in an unsolicited response (including the null-terminator).
 */
//--------------------------------------------------------------------------------------------------
#define LE_ATDEFS_UNSOLICITED_MAX_BYTES 257

//--------------------------------------------------------------------------------------------------
/**
 * Maximum number of bytes in a text message (not including the null-terminator).
 *
 * @warning Special characters cost more than one byte
 *
 */
//--------------------------------------------------------------------------------------------------
#define LE_ATDEFS_TEXT_MAX_LEN 4095

//--------------------------------------------------------------------------------------------------
/**
 * Maximum number of bytes in an text message (including the null-terminator).
 */
//--------------------------------------------------------------------------------------------------
#define LE_ATDEFS_TEXT_MAX_BYTES 4096

//--------------------------------------------------------------------------------------------------
/**
 * Command default timeout (in ms)
 */
//--------------------------------------------------------------------------------------------------
#define LE_ATDEFS_COMMAND_DEFAULT_TIMEOUT 30000

//--------------------------------------------------------------------------------------------------
/**
 *  CME error pattern
 */
//--------------------------------------------------------------------------------------------------
#define LE_ATDEFS_CME_ERROR "+CME ERROR: "

//--------------------------------------------------------------------------------------------------
/**
 *  CMS error pattern
 */
//--------------------------------------------------------------------------------------------------
#define LE_ATDEFS_CMS_ERROR "+CMS ERROR: "

#endif // LE_ATDEFS_INTERFACE_H_INCLUDE_GUARD