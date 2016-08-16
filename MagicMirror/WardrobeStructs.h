/*#pragma once
#ifndef STRUCTS_H
#define STRUCTS_H


#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>

#include "WardrobeStructs.generated.h"

USTRUCT(BlueprintType)
struct FCategory{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Structs")
		FString fullname;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Structs")
		int32 id;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Structs")
		FString name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Structs")
		bool isTrousers;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Structs")
		int32 layer;

	FCategory(){
		fullname = FString("");
		id = -1;
		name = FString("");
		isTrousers = false;
		layer = 0;
	}

	FCategory(FString fullname, int32 id, FString name, bool isTrousers, int32 layer){
		fullname = fullname;
		id = id;
		name = name;
		isTrousers = isTrousers;
		layer = layer;
	}

	FCategory(SQLite::Statement *query){
		fullname = FString(query->getColumn("fullname").getText());
		id = query->getColumn("id");
		name = FString(query->getColumn("name").getText());
		isTrousers = ((int)query->getColumn("istrousers") != 0);
		layer = query->getColumn("layer");
	}
};


USTRUCT(BlueprintType)
struct FClothingItem{
	GENERATED_BODY()
		UPROPERTY(BlueprintReadOnly)
		int32 id;
	UPROPERTY(BlueprintReadOnly)
		FCategory category;
	UPROPERTY(BlueprintReadOnly)
		UTexture2D* texture;

	FClothingItem();

	FClothingItem(SQLite::Statement *query);
};

#endif // !STRUCTS_H*/