//N_Saber.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "N_Saber.generated.h"

//スイングの状態を定義
UENUM(BlueprintType)
enum class ESwingState : uint8
{
	Idle,      //停止
	LowSpeed,  //低速
	HighSpeed  //高速
};

UCLASS()
class MR_1_API AN_Saber : public AActor
{
	GENERATED_BODY()
	
public:	
	AN_Saber();
	virtual void Tick(float DeltaTime) override;

	//弾側から参照する関数
	UFUNCTION(BlueprintCallable)
	ESwingState GetCurrentSwingState() const { return CurrentState; }

	float GetSaberSpeed() const { return CurrentSpeed; }

	UFUNCTION(BlueprintCallable)
	UStaticMeshComponent* GetSaberMesh() const { return Mesh; }

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	FVector LastTipLocation;
	float CurrentSpeed;
	ESwingState CurrentState;

	UPROPERTY(EditAnywhere, Category = "SaberSettings")
	float HighSpeedThreshold = 240.f;
	UPROPERTY(EditAnywhere, Category = "SaberSettings")
	float LowSpeedThreshold = 50.f;
};
