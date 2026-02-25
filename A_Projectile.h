//2025/12/19	銃弾　作成
//A_Projectile.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "A_Projectile.generated.h"

UCLASS()
class MR_1_API AA_Projectile : public AActor
{
	GENERATED_BODY()
	
public:	
	//コンストラクタ
	AA_Projectile();

protected:
	//初期化
	virtual void BeginPlay() override;

	//見た目用のメッシュコンポーネント
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	UStaticMeshComponent* ProjectileMesh;

public:
	//更新
	virtual void Tick(float DeltaTime) override;
	//弾スピード
	UPROPERTY(VisibleAnywhere,Category = "Movement")
	UProjectileMovementComponent* ProjectileMovementComponent;

private	:
	FVector StartLocation; //発射時の位置

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float MaxDistance = 2000.0f; //最大飛距離

protected:
	//弾コリジョン
	UPROPERTY(VisibleDefaultsOnly,Category = "Projectile")
	USphereComponent* CollisionComponent;

	//弾ヒット判定
	UFUNCTION() 
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp,FVector NormalImpulse, const FHitResult& Hit);
};
