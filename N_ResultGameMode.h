//N_ResultGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "N_ResultGameMode.generated.h"

UCLASS()
class MR_1_API AN_ResultGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void TitleBack();
};
