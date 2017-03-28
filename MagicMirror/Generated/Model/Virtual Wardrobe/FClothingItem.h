/**
 * Project Untitled
 */


#ifndef _FCLOTHINGITEM_H
#define _FCLOTHINGITEM_H

#include "FFromTable.h"


class FClothingItem: public FFromTable {
public: 
    FCategory category;
    UTexture2D texture;
};

#endif //_FCLOTHINGITEM_H