//N_IkaEnemy1.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"

#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

#include "N_IkaEnemy1.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class USplineComponent;
class AA_EnemySpline;

UCLASS()
class MR_1_API AN_IkaEnemy1 : public AActor
{
	GENERATED_BODY()

public:
	AN_IkaEnemy1();

protected:
	virtual void BeginPlay() override;

private:
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION()
	void TakeEnemyDamage(float Damage);

private:
	//剣が当たった時の判定
	void NotifyActorBeginOverlap(AActor* OtherActor);

	//発射処理
	void FireProjectile();

	// 発射間隔
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy",
		meta = (AllowPrivateAccess = "true"))
	float fFireInterval;

	FTimerHandle FireTimerHandle;

	FTimerHandle AttackTimerHandle;

	//攻撃予備動作（レーザー）
	void StartAnticipation();
	//発射
	void ExecuteFire();
	//サイクルリセット
	void ResetAttackCycle();
	//攻撃権の確認
	void TryFireProjectile();
	//レーザーの向き更新
	void UpdateLaserTarget(float DeltaTime);

	//レーザーの長さ
	float CurrentLaserLength = 0.0f;

	//伸びる速さ
	UPROPERTY(EditAnywhere, Category = "Enemy|Attack")
	float LaserExtendSpeed = 2500.0f;


	//コンポーネントの宣言
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere)
	UCapsuleComponent* Capsule;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* FirePoint;

	UPROPERTY(VisibleAnywhere, Category = "Effects")
	USceneComponent* NewLaserPivot;



	//弾のクラス
	UPROPERTY(EditAnywhere, Category = "Enemy")
	TSubclassOf<AActor> EnemyProjectile;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, 
		Category = "Enemy", meta = (AllowPrivateAccess = "true"))
	float fEnemyHP;

	
	bool bIsMove = true;
	
	//移動速度をゲームモードでフェーズごとに切り替える
	UPROPERTY(EditAnywhere, Category = "Enemy|Movement",
		meta = (AllowPrivateAccess = "true"))
	float fMoveSpeed = 70.f;

	//出現中フラグ
	bool bIsSpawning = false;

	//登場演出が終わったあとに呼ばれる関数
	void FinishSpawnTransition();

	//演出終了後に消滅させるための関数
	void ExecuteDestroy();


	//発光処理(ダメージくらった時)
	UFUNCTION()
	void FlashRed();
	UPROPERTY()
	UMaterialInstanceDynamic* DynMat;

	//レーザーの音
	UPROPERTY(EditAnywhere, Category = "Effects")
	class USoundBase* LaserChargeSound;

	//音の再生状態を管理するコンポーネント
	UPROPERTY()
	class UAudioComponent* LaserAudioComp;

	//ヒット音アセット
	UPROPERTY(EditAnywhere, Category = "Effects")
	class USoundBase* EnemyHitSound;

	//やられ音アセット
	UPROPERTY(EditAnywhere, Category = "Effects")
	class USoundBase* EnemyDeathSound;


	//足元が光るエフェクト（登場）
	UPROPERTY(EditAnywhere, Category = "Effects")
	UNiagaraSystem* GroundEffect;

	//上に伸びる光のエフェクト（退場）
	UPROPERTY(EditAnywhere, Category = "Effects")
	UNiagaraSystem* PillarEffect;

	//爆発エフェクト（やられた時）
	UPROPERTY(EditAnywhere, Category = "Effects")
	UNiagaraSystem* DeathEffect;

public:
	//出現・退場用の共通関数
	UFUNCTION(BlueprintCallable)
	void PlaySpawnTransition(bool bIsAppearing);

	// スプラインの参照をエディタから設定できるようにする
	UPROPERTY(EditAnywhere, Category = "Enemy|Movement")
	bool bMovingForward;

	class AA_EnemySpline* TargetSpline;

	USplineComponent* SplineComponent;

	float CurrentSplineDistance = 0.0f;

	//レーザーサイト用のMesh
	UPROPERTY(VisibleAnywhere, Category = "Effects")
	UStaticMeshComponent* LaserMesh;


	//ゲームモードでフェーズごとに移動フラグを切り替える
	UFUNCTION(BlueprintCallable)
	void SetIsMove(bool IsMove);

	//ゲームモードでフェーズごとに移動速度を切り替える
	UFUNCTION(BlueprintCallable)
	void SetMoveSpeed(float NewSpeed);

	//ゲームモードでフェーズごとに発射間隔を切り替える
	UFUNCTION(BlueprintCallable)
	void SetFireInterval(float NewInterval);
};
