//N_VRPawn.cpp

#include "N_VRPawn.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"

#include "N_AGameMode_Main.h"

#include "IXRTrackingSystem.h" //XRの標準的なリセット関数

#include "DrawDebugHelpers.h"//画面デバッグ用
//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Damage"));

AN_VRPawn::AN_VRPawn() :
	fPlayerHP(100),
	fMaxPlayerHP(100),
	fInvincibleTime(0.3),
	fDamageFlashWeight(0.0),
	fDamageFlashTarget(0.0),
	fDamageFlashSpeed(0.8)
{
	PrimaryActorTick.bCanEverTick = true;

	//カプセルを親にする
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	RootComponent = Capsule;

	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	VROrigin->SetupAttachment(RootComponent);

	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(VROrigin);

	//発射口を作成
	LeftMuzzle = CreateDefaultSubobject<USceneComponent>(TEXT("LeftMuzzle"));
	LeftMuzzle->SetupAttachment(RootComponent);
}

void AN_VRPawn::BeginPlay()
{
	Super::BeginPlay();

	//解像度を上げる
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		//1.fが標準
		PC->ConsoleCommand(TEXT("vr.PixelDensity 1.0"));
	}

	//デバイスの向きと位置のリセットし、現在の向きを「正面」にする
	if (GEngine && GEngine->XRSystem.IsValid())
	{
		// 向き(Orientation)と位置(Position)をリセット
		GEngine->XRSystem->ResetOrientationAndPosition();
	}

	// PlayerStartの回転を反映させるための処理
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetControlRotation(GetActorRotation());
	}

	//Enhanced Inputを有効化する
	if (APlayerController* PC = Cast<APlayerController>(GetController())) {
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer())) {
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (!VRCamera) return;

	//ダメージフラッシュ
	VRCamera->PostProcessSettings.bOverride_SceneColorTint = true;
	VRCamera->PostProcessSettings.SceneColorTint = FLinearColor::Red;
	VRCamera->PostProcessBlendWeight = 0.f;

	VRCamera->PostProcessSettings.bOverride_VignetteIntensity = true;
	VRCamera->PostProcessSettings.VignetteIntensity = 0.6f;
}

void AN_VRPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!VRCamera)return;

	//ダメージフラッシュ
	if (fDamageFlashWeight > fDamageFlashTarget)
	{
		fDamageFlashWeight = FMath::FInterpTo(
			fDamageFlashWeight,
			fDamageFlashTarget,
			DeltaTime,
			fDamageFlashSpeed
		);
	}
	else if (fDamageFlashTarget > 0.f)
	{
		fDamageFlashTarget = 0.f;
	}
	else
	{
		fDamageFlashWeight = FMath::FInterpTo(
			fDamageFlashWeight,
			0.f,
			DeltaTime,
			fDamageFlashSpeed
		);
	}

	VRCamera->PostProcessBlendWeight = fDamageFlashWeight;
}

//入力
void AN_VRPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		//作成済みのResetアクションを関数にバインド
		if (ResetAction)
		{
			EIC->BindAction(ResetAction, ETriggerEvent::Triggered, this, &AN_VRPawn::OnResetTriggered);
		}
	}
}

//ダメージ関数
void AN_VRPawn::TakePlayerDamage(float Damage)
{
	if (bInvincible) return;

	fPlayerHP = FMath::Clamp(fPlayerHP - Damage, 0.f, fMaxPlayerHP);

	//デリゲートを実行
	if (OnHPChanged.IsBound())
	{
		OnHPChanged.Broadcast(fPlayerHP / fMaxPlayerHP);
	}

	//ダメージフラッシュ開始
	fDamageFlashWeight = 1.f;
	fDamageFlashTarget = 0.3f;
	if (VRCamera)
	{
		VRCamera->PostProcessBlendWeight = fDamageFlashWeight;
	}

	//無敵時間開始
	bInvincible = true;

	GetWorldTimerManager().SetTimer(
		InvincibleTimerHandle,
		this,
		&AN_VRPawn::EndInvincible,
		0.3f,
		false
	);

	//フラッシュ解除
	FTimerHandle FlashTimer;
	GetWorldTimerManager().SetTimer(
		FlashTimer,
		[this]()
		{
			if (VRCamera)
			{
				VRCamera->PostProcessBlendWeight = 0.f;
			}
		},
		0.1f,
		false
	);

	//HPがゼロになったらフィニッシュUIを表示させる
	if (fPlayerHP <= 0)
	{
		AN_AGameMode_Main* GM = Cast<AN_AGameMode_Main>(UGameplayStatics::GetGameMode(this));
		if (GM)
		{
			GM->OnGameFinish();
		}
	}
}

//無敵時間間隔に使うフラグチェンジ
void AN_VRPawn::EndInvincible()
{
	bInvincible = false;
}

void AN_VRPawn::OnResetTriggered()
{
	//強制的にタイトルレベルへ移動
	UGameplayStatics::OpenLevel(this, FName("TitleLevel"));
}
