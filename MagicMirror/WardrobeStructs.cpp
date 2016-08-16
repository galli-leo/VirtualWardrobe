/*#include "MagicMirror.h"
#include "WardrobeManager.h"


FClothingItem::FClothingItem(){
	id = -1;
	category = FCategory();
	texture = UTexture2D::CreateTransient(1024, 1024);
}

FClothingItem::FClothingItem(SQLite::Statement *query){
	id = query->getColumn("id");
	uint32 catID = query->getColumn("category");
	texture = UTexture2D::CreateTransient(1024, 1024);
	for (FCategory cat : UWardrobeManager::categories)
	{
		if (cat.id == catID)
		{
			category = cat;
		}
	}
}*/