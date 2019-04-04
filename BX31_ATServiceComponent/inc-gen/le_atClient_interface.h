

/*
 * ====================== WARNING ======================
 *
 * THE CONTENTS OF THIS FILE HAVE BEEN AUTO-GENERATED.
 * DO NOT MODIFY IN ANY WAY.
 *
 * ====================== WARNING ======================
 */

/**
 * @page c_atClient AT Commands Client
 *
 * @ref le_atClient_interface.h "API Reference"
 *
 * @warning Some AT commands may conflict with Legato APIs; using both may cause problems that can
 * be difficult to diagnose. AT commands should be avoided whenever possible, and should only be
 * used with great care.
 *
 * The AT Client Service handles the AT commands sent to the modem on a specified
 * serial device. It also supports getting AT command responses (intermediate, final or
 * unsolicited responses).
 * This service can be subscribed to by several apps simultaneously.
 *
 * @section le_atClient_binding IPC interfaces binding
 *
 * All the functions of this API are provided by the @b atService.
 *
 * Here's a code sample binding to the AT commands server:
 * @verbatim
   bindings:
   {
      atClientTest.atClientTestComp.le_atClient -> atService.le_atClient
   }
   @endverbatim
 *
 * @section atClient_binding Device Binding
 *
 * le_atClient_Start() must be called to bind a specific device with the ATClient.
 * Multiple devices can be bound. The app has to configured the device before using it with
 * the ATClient.
 * A device can be unbound using le_atClient_Stop().
 *
 * @section atClient_statement Statement
 *
 * An AT command statement is requested before sending it. The following steps have to be done for
 * its declaration:
 *
 * - create an AT command reference using le_atClient_Create().
 *
 * - use le_atClient_SetCommand() to set the AT command to be sent to the modem.
 *
 * - set the sending port to use le_atClient_SetDevice().
 *
 * - can set a timeout value using le_atClient_SetTimeout(); default value is @c 30s.
 *
 * - request expected final responses and set using le_atClient_SetFinalResponse().The final
 * response is mandatory to detect
 * the end of the AT command execution. If an AT command answers with a final response that
 * doesn't belong to the final responses set, the AT command execution will end by timeout.
 *
 * - can call le_atClient_SetIntermediateResponse() to set the intermediate responses
 * filters. This is optional.
 *
 * Command responses given in le_atClient_SetIntermediateResponse() and
 * le_atClient_SetFinalResponse() are the first characters of the response lines. They are used as a
 * filter of the received lines (a line are the characters received between receiving a
 * <CR> and an <LF>). Other lines are dropped.
 *
 * - text can be set using le_atClient_SetText(). This is used for commands that answers a
 * '>' character to receive additional information. The given text is sent to the modem when '>' is
 * detected. The character @c CTRL-Z is automatically sent.
 *
 *
 * @section atClient_send Sending
 *
 * When the AT command declaration is complete, it can be sent using le_atClient_Send(). This API is
 * synchronous (blocking until final response is detected, or timeout reached).
 *
 * le_atClient_SetCommandAndSend() is equivalent to le_atClient_Create(), le_atClient_SetCommand(),
 * le_atClient_SetDevice(), le_atClient_SetTimeout(), le_atClient_SetIntermediateResponse() and
 * le_atClient_SetFinalResponse(), in case of an Error le_atClient_Delete(),
 * in one API call.
 * The AT command reference is created and returned by this API. When an error
 * occurs the command reference is deleted and is not a valid reference anymore
 *
 * @section atClient_responses Responses
 *
 * When the AT command has been sent correctly (i.e., le_atClient_Send() or
 * le_atClient_SetCommandAndSend() execution is successful), the app gets these AT command
 * responses:
 * - le_atClient_GetFinalResponse() is used to get the final responses
 * - le_atClient_GetFirstIntermediateResponse() is used to get the first intermediate result code.
 * Other intermediate result codes can be obtained by calling
 * le_atClient_GetNextIntermediateResponse().Returns LE_NOT_FOUND when there are no further results.
 *
 * When a response has been set in the AT command declaration, the AT command response returned by
 * these APIs start with the given pattern, and ends when a <CR><LF> is detected.
 *
 * @section atClient__delete Deleting
 *
 * When the AT command is over, the reference has to be deleted by calling le_atClient_Delete().
 *
 * @section atClient_unsolicited Unsolicited Responses
 *
 * An app can subscribe to a specific, unsolicited response using
 * le_atClient_AddUnsolicitedResponseHandler(), and can be removed using
 * le_atClient_RemoveUnsolicitedResponseHandler(). The subscribed handler is called when the given
 * pattern is detected. The handler receives a parameter with the complete line of the unsolicited
 * response.
 * The parameter @c lineCount is used to set the unsolicited lines number.
 * For example, @c +CMT unsolicited response has the following syntax:
 * @code
 * +CMT: ...<CR><LF>
 * <sms text>
 * @endcode
 * In this case, @c lineCount has to be set to @c 2 to receive both lines into the handler.
 * @c +CREG unsolicited response is sent only one line, so @c lineCount is set to @c 1.
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 */
/**
 * @file le_atClient_interface.h
 *
 * Legato @ref c_atClient include file.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef LE_ATCLIENT_INTERFACE_H_INCLUDE_GUARD
#define LE_ATCLIENT_INTERFACE_H_INCLUDE_GUARD


#include "legato.h"

// Interface specific includes
#include "le_atDefs_interface.h"


//--------------------------------------------------------------------------------------------------
/**
 * Type for handler called when a server disconnects.
 */
//--------------------------------------------------------------------------------------------------
typedef void (*le_atClient_DisconnectHandler_t)(void *);

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
void le_atClient_ConnectService
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
le_result_t le_atClient_TryConnectService
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
void le_atClient_SetServerDisconnectHandler
(
    le_atClient_DisconnectHandler_t disconnectHandler,
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
void le_atClient_DisconnectService
(
    void
);


//--------------------------------------------------------------------------------------------------
/**
 */
//--------------------------------------------------------------------------------------------------
typedef struct le_atClient_Cmd* le_atClient_CmdRef_t;


//--------------------------------------------------------------------------------------------------
/**
 */
//--------------------------------------------------------------------------------------------------
typedef struct le_atClient_Device* le_atClient_DeviceRef_t;


//--------------------------------------------------------------------------------------------------
/**
 * Reference type used by Add/Remove functions for EVENT 'le_atClient_UnsolicitedResponse'
 */
//--------------------------------------------------------------------------------------------------
typedef struct le_atClient_UnsolicitedResponseHandler* le_atClient_UnsolicitedResponseHandlerRef_t;


//--------------------------------------------------------------------------------------------------
/**
 * Handler for unsolicited response reception.
 *
 */
//--------------------------------------------------------------------------------------------------
typedef void (*le_atClient_UnsolicitedResponseHandlerFunc_t)
(
    const char* LE_NONNULL unsolicitedRsp,
        ///< The call reference.
    void* contextPtr
        ///<
);

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to start a ATClient session on a specified device.
 *
 * @return reference on a device context
 */
//--------------------------------------------------------------------------------------------------
le_atClient_DeviceRef_t le_atClient_Start
(
    int fd
        ///< [IN] File descriptor.
);

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to stop the ATClient session on the specified device.
 *
 * @return
 *      - LE_FAULT when function failed
 *      - LE_OK when function succeed
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_atClient_Stop
(
    le_atClient_DeviceRef_t device
        ///< [IN] Device reference
);

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to create a new AT command.
 *
 * @return pointer to the new AT Command reference
 */
//--------------------------------------------------------------------------------------------------
le_atClient_CmdRef_t le_atClient_Create
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to delete an AT command reference.
 *
 * @return
 *      - LE_OK when function succeed
 *
 * @note If the AT Command reference is invalid, a fatal error occurs,
 *       the function won't return.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_atClient_Delete
(
    le_atClient_CmdRef_t cmdRef
        ///< [IN] AT Command
);

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the AT command string to be sent.
 *
 * @return
 *      - LE_OK when function succeed
 *
 * @note If the AT Command reference is invalid, a fatal error occurs,
 *       the function won't return.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_atClient_SetCommand
(
    le_atClient_CmdRef_t cmdRef,
        ///< [IN] AT Command
    const char* LE_NONNULL command
        ///< [IN] Set Command
);

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the waiting intermediate responses.
 * Several intermediate responses can be specified separated by a '|' character into the string
 * given in parameter.
 *
 * @return
 *      - LE_FAULT when function failed
 *      - LE_OK when function succeed
 *
 * @note If the AT Command reference or set intermediate response is invalid, a fatal error occurs,
 *       the function won't return.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_atClient_SetIntermediateResponse
(
    le_atClient_CmdRef_t cmdRef,
        ///< [IN] AT Command
    const char* LE_NONNULL intermediate
        ///< [IN] Set Intermediate
);

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the final response(s) of the AT command execution.
 * Several final responses can be specified separated by a '|' character into the string given in
 * parameter.
 *
 * @return
 *      - LE_FAULT when function failed
 *      - LE_OK when function succeed
 *
 * @note If the AT Command reference or set response is invalid, a fatal error occurs,
 *       the function won't return.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_atClient_SetFinalResponse
(
    le_atClient_CmdRef_t cmdRef,
        ///< [IN] AT Command
    const char* LE_NONNULL response
        ///< [IN] Set Response
);

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the text when the prompt is expected.
 *
 * @return
 *      - LE_FAULT when function failed
 *      - LE_OK when function succeed
 *
 * @note If the AT Command reference is invalid, a fatal error occurs,
 *       the function won't return.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_atClient_SetText
(
    le_atClient_CmdRef_t cmdRef,
        ///< [IN] AT Command
    const char* LE_NONNULL text
        ///< [IN] The AT Data to send
);

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the timeout of the AT command execution.
 *
 * @return
 *      - LE_OK when function succeed
 *
 * @note If the AT Command reference is invalid, a fatal error occurs,
 *       the function won't return.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_atClient_SetTimeout
(
    le_atClient_CmdRef_t cmdRef,
        ///< [IN] AT Command
    uint32_t timer
        ///< [IN] The timeout value in milliseconds.
);

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the device where the AT command will be sent.
 *
 * @return
 *      - LE_FAULT when function failed
 *      - LE_OK when function succeed
 *
 * @note If the AT Command reference is invalid, a fatal error occurs,
 *       the function won't return.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_atClient_SetDevice
(
    le_atClient_CmdRef_t cmdRef,
        ///< [IN] AT Command
    le_atClient_DeviceRef_t devRef
        ///< [IN] Device where the AT command has to be sent
);

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to send an AT Command and wait for response.
 *
 * @return
 *      - LE_FAULT when function failed
 *      - LE_TIMEOUT when a timeout occur
 *      - LE_OK when function succeed
 *
 * @note If the AT Command reference is invalid, a fatal error occurs,
 *       the function won't return.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_atClient_Send
(
    le_atClient_CmdRef_t cmdRef
        ///< [IN] AT Command
);

//--------------------------------------------------------------------------------------------------
/**
 * This function is used to get the first intermediate response.
 *
 * @return
 *      - LE_FAULT when function failed
 *      - LE_OK when function succeed
 *
 * @note If the AT Command reference is invalid, a fatal error occurs,
 *       the function won't return.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_atClient_GetFirstIntermediateResponse
(
    le_atClient_CmdRef_t cmdRef,
        ///< [IN] AT Command
    char* intermediateRsp,
        ///< [OUT] First intermediate result code
    size_t intermediateRspSize
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * This function is used to get the next intermediate response.
 *
 * @return
 *      - LE_NOT_FOUND when there are no further results
 *      - LE_OK when function succeed
 *
 * @note If the AT Command reference is invalid, a fatal error occurs,
 *       the function won't return.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_atClient_GetNextIntermediateResponse
(
    le_atClient_CmdRef_t cmdRef,
        ///< [IN] AT Command
    char* intermediateRsp,
        ///< [OUT] Get Next intermediate result
        ///< code.
    size_t intermediateRspSize
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * This function is used to get the final response
 *
 * @return
 *      - LE_FAULT when function failed
 *      - LE_OK when function succeed
 *
 * @note If the AT Command reference is invalid, a fatal error occurs,
 *       the function won't return.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_atClient_GetFinalResponse
(
    le_atClient_CmdRef_t cmdRef,
        ///< [IN] AT Command
    char* finalRsp,
        ///< [OUT] Get Final Line
    size_t finalRspSize
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to automatically set and send an AT Command.
 *
 * @return
 *      - LE_FAULT when function failed
 *      - LE_TIMEOUT when a timeout occur
 *      - LE_OK when function succeed
 *
 * @note This command creates a command reference when called
 *
 * @note In case of an Error the command reference will be deleted and though
 *       not usable. Make sure to test the return code and not use the reference
 *       in other functions.
 *
 * @note If the AT command is invalid, a fatal error occurs,
 *        the function won't return.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_atClient_SetCommandAndSend
(
    le_atClient_CmdRef_t* cmdRefPtr,
        ///< [OUT] Cmd reference
    le_atClient_DeviceRef_t devRef,
        ///< [IN] Dev reference
    const char* LE_NONNULL command,
        ///< [IN] AT Command
    const char* LE_NONNULL interResp,
        ///< [IN] Expected intermediate response
    const char* LE_NONNULL finalResp,
        ///< [IN] Expected final response
    uint32_t timeout
        ///< [IN] Timeout value in milliseconds.
);

//--------------------------------------------------------------------------------------------------
/**
 * Add handler function for EVENT 'le_atClient_UnsolicitedResponse'
 *
 * This event provides information on a subscribed unsolicited response when this unsolicited
 * response is received.
 *
 */
//--------------------------------------------------------------------------------------------------
le_atClient_UnsolicitedResponseHandlerRef_t le_atClient_AddUnsolicitedResponseHandler
(
    const char* LE_NONNULL unsolRsp,
        ///< [IN] Pattern to match
    le_atClient_DeviceRef_t devRef,
        ///< [IN] Device to listen
    le_atClient_UnsolicitedResponseHandlerFunc_t handlerPtr,
        ///< [IN] unsolicited handler
    void* contextPtr,
        ///< [IN]
    uint32_t lineCount
        ///< [IN] Indicate the number of line of
        ///< the unsolicited
);

//--------------------------------------------------------------------------------------------------
/**
 * Remove handler function for EVENT 'le_atClient_UnsolicitedResponse'
 */
//--------------------------------------------------------------------------------------------------
void le_atClient_RemoveUnsolicitedResponseHandler
(
    le_atClient_UnsolicitedResponseHandlerRef_t handlerRef
        ///< [IN]
);

#endif // LE_ATCLIENT_INTERFACE_H_INCLUDE_GUARD