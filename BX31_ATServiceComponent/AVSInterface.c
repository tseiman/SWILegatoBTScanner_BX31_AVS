
/*
 * AVSInterface.c
 *
 *  This is part of the "BX31_ATService" Project
 *  Created on: Apr 8, 2019
 *      Author: Thomas Schmidt, SWI
 */

#include "legato.h"
#include "interfaces.h"



static le_avdata_RequestSessionObjRef_t avsSession = NULL;

void initAvsService() {
        avsSession = le_avdata_RequestSession();
        LE_ASSERT(le_avdata_SetNamespace(LE_AVDATA_NAMESPACE_GLOBAL) == LE_OK);
}
