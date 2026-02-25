//N_MyGameInstance.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "N_MyGameInstance.generated.h"

UCLASS()
class MR_1_API UN_MyGameInstance : public UGameInstance
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, Category = "GameData")
    int32 FinalKillCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "GameData")
    int32 TotalPossibleEnemies = 50; // �ő吔
};