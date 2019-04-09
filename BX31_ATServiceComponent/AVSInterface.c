
/*
 * AVSInterface.c
 *
 *  This is part of the "BX31_ATService" Project
 *  Created on: Apr 8, 2019
 *      Author: Thomas Schmidt, SWI
 */

#include "legato.h"
#include "interfaces.h"
#include "AVSInterface.h"



static le_avdata_RequestSessionObjRef_t avsSession = NULL;
static le_avdata_RecordRef_t avsRecordRef = NULL;

le_result_t avsService_init() {
        avsSession = le_avdata_RequestSession();

        if (NULL == avsSession) {
                LE_ERROR("AirVantage Connection Controller does not start.");
                return LE_UNAVAILABLE;
        }

        LE_ASSERT(le_avdata_SetNamespace(LE_AVDATA_NAMESPACE_GLOBAL) == LE_OK);


        return LE_OK;
}


void PushRecordCallbackHandler(le_avdata_PushStatus_t status, void* contextPtr) {
        if (status == LE_AVDATA_PUSH_SUCCESS) {
                LE_INFO("Push Timeserie OK");
        } else {
                LE_INFO("Failed to push Timeserie");
        }
}


le_result_t avsService_recordData(char *path, void *data, avsService_DataType_t type) {
        struct timeval  tv;
        gettimeofday(&tv, NULL);
        uint64_t utcMilliSec = (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;

        if(avsRecordRef ==NULL) {
                LE_ASSERT( (avsRecordRef = le_avdata_CreateRecord()) != NULL);    // a record is to collect a series of events over
                                                                                  // time and push the series later. We use the
                                                                                  // record to keep track even if we have not
                                                                                  // been able to push it now, because of coverage
        }                                                                         // etc.


        le_result_t recordResult;

        switch(type)  {
        case INT:       recordResult = le_avdata_RecordInt   (avsRecordRef, path, *((int32_t *) data), utcMilliSec); break;
        case FLOAT:     recordResult = le_avdata_RecordFloat (avsRecordRef, path, *((double  *) data), utcMilliSec); break;
        case BOOL:      recordResult = le_avdata_RecordBool  (avsRecordRef, path, *((bool    *) data), utcMilliSec); break;
        case STRING:    recordResult = le_avdata_RecordString(avsRecordRef, path,  ((char    *) data), utcMilliSec); break;
        default:  LE_ERROR("Invalid Data Type"); return LE_FAULT; break;
        }

        if(recordResult != LE_OK) {
                LE_WARN("Failed to push data to record result was : %d", recordResult);
                return recordResult;
        }




        return LE_OK;
}

le_result_t avsService_pushData() {
        le_result_t result = LE_OK;
        if ((result= le_avdata_PushRecord(avsRecordRef, PushRecordCallbackHandler, NULL)) == LE_OK) {
                le_avdata_DeleteRecord(avsRecordRef);
                avsRecordRef = NULL;
        } else {
                LE_WARN("Failed pushing time series, will retry next time");

        }
        return result;
}


void avsService_detroy() {
        if(avsRecordRef) le_avdata_DeleteRecord(avsRecordRef);
        if (avsSession) le_avdata_ReleaseSession(avsSession);
}
