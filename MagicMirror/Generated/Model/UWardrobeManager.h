/**
 * Project Untitled
 */


#ifndef _UWARDROBEMANAGER_H
#define _UWARDROBEMANAGER_H

class UWardrobeManager {
public: 
    static FString documentsPath;
    static FString texturePath;
    EWardrobeMode mode;
    FCategory scanningCategory;
    TArray<FClothingItem> items;
    int32 currentItemPos;
    static TArray<FCategories> categories;
    static float scanInterval;
    float lastAction;
    float timeSinceSecondScan;
    /**
     * UPROPERTY(BlueprintAssignable, Category = "Wardrobe")
     * DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTshirtProcessedDelegate, UTexture2D*, texture);
     */
    FItemProcessedDelegate ItemProcessed;
    /**
     * DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FScanningStatusUpdate, FString, newStatus);
     */
    FScanningStatusUpdateDelegate ScanningStatusUpdate;
    FClothingItem currentClothingItem;
    FCategory currentFilter;
    static Database database;
    float currentTime;
    
    /**
     * @param startingMode
     */
    void StartWardrobeManager(EWardrobeMode startingMode);
    
    /**
     * @param float deltaTime
     */
    void Tick(void float deltaTime);
    
    /**
     * @param category
     */
    void StartScanning(FCategory category);
    
    TArray<FCategory> GetCategories();
    
    /**
     * @param id
     */
    FCategory GetCategory(int32 id);
    
    FClothingItem NextClothingItem();
    
    FClothingItem PreviousClothingItem();
    
    /**
     * @param category
     * @param recommendedItem
     */
    FClothingItem FilterClothingItem(FCategory category, FClothingItem recommendedItem);
    
    void GetClothesFromDB();
};

#endif //_UWARDROBEMANAGER_H