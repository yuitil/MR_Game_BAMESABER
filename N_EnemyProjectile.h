//N_EnemyProjectile.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Haptics/HapticFeedbackEffect_Base.h"

#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

#include "N_EnemyProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class AN_IkaEnemy1;
class AN_VRPawn;
class AN_Saber;

UCLASS()
class MR_1_API AN_EnemyProjectile : public AActor
{
	GENERATED_BODY()

public:
	AN_EnemyProjectile();

protected:
	virtual void BeginPlay() override;

	//振動アセットのセット
	UPROPERTY(EditAnywhere, Category = "Effects")
	class UHapticFeedbackEffect_Base* SaberHitHaptic;

	//反射音アセットのセット
	UPROPERTY(EditAnywhere, Category = "Effects")
	class USoundBase* SaberHitSound;

	//	消滅時の音
	UPROPERTY(EditAnywhere, Category = "Effects")
	class USoundBase* ProjectileDestroySound;

	//火花エフェクト
	UPROPERTY(EditAnywhere, Category = "Effects")
	UNiagaraSystem* SparkEffect;

private:
	//ヒット処理
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere)
	USphereComponent* Collision;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovement;

	//弾の色変え用
	UPROPERTY()
	UMaterialInstanceDynamic* DynMat;


	//反射許容角度
	UPROPERTY(VisibleAnywhere)
	float fAssistAngleDeg;

	UPROPERTY(VisibleAnywhere)
	float fMaxAssistDist;

	//反射時のダメージ保持用
	float CurrentReflectDamage = 1.0f;

	//反射したかどうか
	UPROPERTY()
	bool bReflected = false;

	//スイングスピード、これ以上なら速い
	UPROPERTY(EditAnywhere, Category = "ReflectSettings")
	float MinSwingSpeed = 500.f;

	//スウィートスポット、剣の特定の点から何センチならスポット
	UPROPERTY(EditAnywhere, Category = "ReflectSettings")
	float SweetSpotRang = 14.f;

	//追従の強さ
	UPROPERTY(EditAnywhere, Category = "ReflectSettings")
	float HomingStrength = 20000.f;

	FVector LastSaberLocation;
	bool bIsFirstHit = true;

	//スピード判定
	bool bIsStrongHit = false;
	//タイミング判定
	bool bIsPerfectTiming = false;
	//場所判定
	bool bIsSweetSpot = false;
};
