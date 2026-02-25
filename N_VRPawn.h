//N_VRPawn.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"

#include "MotionControllerComponent.h"
#include "A_Projectile.h"
#include "InputActionValue.h"

#include "N_VRPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHPChanged, float, NewRatio);

class AN_IkaEnemy1;

UCLASS()
class MR_1_API AN_VRPawn : public APawn
{
	GENERATED_BODY()

public:
	AN_VRPawn();

protected:
	virtual void BeginPlay() override;

private:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UFUNCTION()
	void TakePlayerDamage(float Damage);

	//カメラ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	UCameraComponent* VRCamera;

	//弾を発射するポイント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class USceneComponent* LeftMuzzle;

	//弾のクラス
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<class AA_Projectile> ProjectileClass;

	//入力設定
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;

	//他クラスから「HPが減った」ことを知るための窓口
	UPROPERTY(BlueprintAssignable, Category = "Player|Events")
	FOnHPChanged OnHPChanged;

private:
	UPROPERTY(VisibleAnywhere)
	UCapsuleComponent* Capsule;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* VROrigin;

	//HP
	UPROPERTY(EditAnywhere, Category = "Player")
	float fPlayerHP;
	//MaxHP
	UPROPERTY(EditAnywhere, Category = "Player")
	float fMaxPlayerHP;

	//---無敵時間---
	UPROPERTY(EditAnywhere, Category = "Player")
	float fInvincibleTime;
	//無敵時間中かどうか
	bool bInvincible = false;
	//インターバルタイマー
	FTimerHandle InvincibleTimerHandle;
	//インターバルフラグを戻す関数
	void EndInvincible();

	//---ダメージフラッシュ---
	float fDamageFlashWeight;
	float fDamageFlashTarget;

	UPROPERTY(EditAnywhere, Category = "DamageFlash")
	float fDamageFlashSpeed;

	//リセット処理（タイトルに戻る）
	//ボタンが押されたときに実行する関数(メニューボタン)
	void OnResetTriggered();

public:
	//入力アクション（Reset用）を保持する変数
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* ResetAction;
};
