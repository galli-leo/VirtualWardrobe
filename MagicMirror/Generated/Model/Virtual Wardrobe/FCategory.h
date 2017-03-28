/**
 * Project Untitled
 */


#ifndef _FCATEGORY_H
#define _FCATEGORY_H

#include "FFromTable.h"


class FCategory: public FFromTable {
public: 
    FString fullName;
    FString name;
    bool bIsTrousers;
    int32 layer;
};

#endif //_FCATEGORY_H