//2025/12/19	プレイヤーキャラクター　作成
//A_KeybordCharacter.cpp

#include "A_KeyboadCharacter.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"

//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("ChargeCompleted!"));




//コンストラクタ
AA_KeyboadCharacter::AA_KeyboadCharacter()
	: bRButtonPressed(false)
	, bLButtonPressed(false)
	, bIsCharging(false)
	, ChargeTime(0.0f)
{
	//更新するかどうかのフラグ
	PrimaryActorTick.bCanEverTick = true;

	FPCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FPCameraComponent->SetupAttachment(GetCapsuleComponent());
	FPCameraComponent->bUsePawnControlRotation = true;

}

//初期化
void AA_KeyboadCharacter::BeginPlay()
{
	Super::BeginPlay();

	//InputMappingContext登録
	if (APlayerController* PC = Cast<APlayerController>(Controller)) {
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = PC->GetLocalPlayer()
			->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()) {
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}


}

//更新
void AA_KeyboadCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsCharging) {
		ChargeTime += DeltaTime;
	}

	//UE_LOG(LogTemp, Warning, TEXT("Fired with charge amount:%f"), ChargeTime);


}


//バインド
void AA_KeyboadCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);


	//EnhancedInputの入力バインド実装
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		if (IA_Action) {
			EIC->BindAction(IA_Action, ETriggerEvent::Started, this, &AA_KeyboadCharacter::StartCharge);		//チャージ開始
			EIC->BindAction(IA_Action, ETriggerEvent::Completed, this, &AA_KeyboadCharacter::ReleaseCharge);	//チャージ終了
			EIC->BindAction(IA_Action, ETriggerEvent::Canceled, this, &AA_KeyboadCharacter::ReleaseCharge);		//チャージキャンセル
			EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AA_KeyboadCharacter::Look);			//カメラ
		}
	}

}

//カメラ
void AA_KeyboadCharacter::Look(const FInputActionValue& Value) {
	FVector2D AxisValue = Value.Get<FVector2D>();
	AddControllerYawInput(AxisValue.X);
	AddControllerPitchInput(-AxisValue.Y);
}


//弾発射
void AA_KeyboadCharacter::OnMouseClick() {
	UE_LOG(LogTemp, Warning, TEXT("Left Mouse Button Clicked"));

	if (!ProjectileClass) return;

	UWorld* World = GetWorld();
	if (!World)return;

	//スポーン位置
	FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 100.f
		+ FVector(0, 0, 50.f);

	//スポーン時の向き
	FRotator SpawnRotation = GetControlRotation();

	//
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	//
	AA_Projectile* Projectile = World->SpawnActor<AA_Projectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (Projectile) {
		FVector LaunchDirection = SpawnRotation.Vector();
		Projectile->ProjectileMovementComponent->Velocity
			= LaunchDirection * Projectile->ProjectileMovementComponent->InitialSpeed;
	}


}

//チャージ開始
void AA_KeyboadCharacter::StartCharge(const FInputActionValue& Value) {
	bIsCharging = true;
	ChargeTime = 0.0f;
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("ChargeStart!"));
}

//チャージ終了
void AA_KeyboadCharacter::ReleaseCharge(const FInputActionValue& Value) {
	if (bIsCharging) {
		bIsCharging = false;
		ChargeShoot(ChargeTime);
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("ChargeCompleted!"));
	}
}

//チャージショット
void AA_KeyboadCharacter::ChargeShoot(float FinalCharge) {

	//一段階目
	if (FinalCharge > 0.0f && FinalCharge < 1.0f) {
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("FirstChargeShoot!"));
		OnMouseClick();
	}

	//二段階目
	if (FinalCharge > 1.0f && FinalCharge < 2.0f) {
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("SecondChargeShoot!"));
		OnMouseClick();
	}

	//三段階目
	if (FinalCharge > 2.0f) {
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("MaxChargeShoot!"));
		OnMouseClick();
	}


	UE_LOG(LogTemp, Warning, TEXT("Fired with charge amount:"), FinalCharge);
}