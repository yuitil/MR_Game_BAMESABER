////N_MRWallManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "N_MRWallManager.generated.h"

UCLASS()
class MR_1_API AN_MRWallManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AN_MRWallManager();

protected:
	virtual void BeginPlay() override;

public:	
	//virtual void Tick(float DeltaTime) override;

	//スキャンデータがロードされた時に呼ばれる関数
	//UFUNCTION()
	//void OnSceneModelLoaded();

	//敵のクラスをエディタから指定できるようにする
	UPROPERTY(EditAnywhere, Category = "MR Settings")
	TSubclassOf<AActor> EnemyClass;

	//壁からどれくらい離して出すか(cm)
	UPROPERTY(EditAnywhere, Category = "MR Settings")
	float SpawnOffset = 10.f;

private:
	//実際に壁を探してスポーンさせる内部処理
	void FindWallAndSpawn();
};
