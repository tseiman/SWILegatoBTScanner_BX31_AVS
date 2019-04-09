/*
 * AVSInterface.h
 *
 *  This is part of the "BX31_ATService" Project
 *  Created on: Apr 8, 2019
 *      Author: Thomas Schmidt, SWI
 */

#include "legato.h"
#include "interfaces.h"

#ifndef AVSINTERFACE_H_
#define AVSINTERFACE_H_


typedef enum {
        INT,
        FLOAT,
        BOOL,
        STRING
} avsService_DataType_t;


le_result_t avsService_init();
le_result_t avsService_recordData(char *path, void *data, avsService_DataType_t type);
le_result_t avsService_pushData();
void avsService_detroy();

#endif /* AVSINTERFACE_H_ */
