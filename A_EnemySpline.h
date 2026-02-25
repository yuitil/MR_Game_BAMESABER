//A_EnemySpline.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MR_1/N_IkaEnemy1.h"
#include "Components/SplineComponent.h"

#include "A_EnemySpline.generated.h"

class USplineComponent;
UCLASS()
class MR_1_API AA_EnemySpline : public AActor
{
	GENERATED_BODY()
	
public:	
	AA_EnemySpline();

protected:
	virtual void BeginPlay() override;

public:	
	//virtual void Tick(float DeltaTime) override;
	//移動速度
	UPROPERTY(EditAnywhere,Category = "Movement")
	float MoveSpeed = 1000.f;
	//現在の距離
	float CurrentDistance = 0.f;
	//スプラインの長さ
	float SplineLength = 0.f;
	//距離越えイベント
	void ToggleMoveDirection();
	UPROPERTY(EditAnywhere,Category = "Movement")
	float MoveDistance = 300.f;
	UPROPERTY(EditAnywhere,Category ="Movement")
	float MoveInterval = 3.f;

	FVector StartLocation;
	int32 Direction = 1;
	FTimerHandle MoveTimerHandle;

public:
	//スプラインコンポーネントを外部から取得するための関数
	FORCEINLINE class USplineComponent* GetSplineComponent() const { return SplineComponent; }

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline", meta = (AllowPrivateAccess = "true"))
	class USplineComponent* SplineComponent;
};
