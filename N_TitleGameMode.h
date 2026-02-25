//N_TitleGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "N_TitleGameMode.generated.h"

UCLASS()
class MR_1_API AN_TitleGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void StartInGame2();
};
