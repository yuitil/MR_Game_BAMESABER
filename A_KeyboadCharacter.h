//2025/12/19	プレイヤーキャラクター　作成
//A_KeybordCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "A_Projectile.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Blueprint/UserWidget.h"
#include "A_KeyboadCharacter.generated.h"



//前方宣言
class UCameraComponent;

UCLASS()
class MR_1_API AA_KeyboadCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	//コンストラクタ
	AA_KeyboadCharacter();

protected:
	//初期化
	virtual void BeginPlay() override;
	//弾クラス定義
	UPROPERTY(EditDefaultsOnly,Category = "Projectile")
	TSubclassOf<AA_Projectile> ProjectileClass;
	//コンテキスト
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	//マウスクリック
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Input")
	TObjectPtr<UInputAction> IA_Action;
	//視点移動
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Input")
	TObjectPtr<UInputAction> LookAction;
	//カメラ
	UPROPERTY(VisibleAnywhere,Category = "Camera")
	UCameraComponent* FPCameraComponent;


public:	
	//更新
	virtual void Tick(float DeltaTime) override;

	//入力メソッドのバインド処理
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


private:
	//ボタンが押されているかのフラグ
	bool bLButtonPressed;
	bool bRButtonPressed;
	//チャージ時間
	float ChargeTime;
	//チャージ中かどうか
	bool bIsCharging;

	//チャージ開始
	void StartCharge(const FInputActionValue& Value);
	//チャージ終了
	void ReleaseCharge(const FInputActionValue& Value);
	//チャージ処理
	void ChargeShoot(float FinalCharge);

	UFUNCTION()
	void Look(const FInputActionValue& Value);
	//弾発射
	void OnMouseClick();
};
