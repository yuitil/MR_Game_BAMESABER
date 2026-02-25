//N_IkaEnemy1.cpp

#include "N_IkaEnemy1.h"
#include "N_AGameMode_Main.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "A_EnemySpline.h"
#include "Components/SplineComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/AudioComponent.h"

#include "DrawDebugHelpers.h"//画面デバッグ用

AN_IkaEnemy1::AN_IkaEnemy1() :
	fEnemyHP(5.f),
	fFireInterval(3.0f)
{
	PrimaryActorTick.bCanEverTick = true;

	//当たり判定
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(Root);
	Capsule->InitCapsuleSize(40.f, 80.f);

	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Capsule->SetCollisionObjectType(ECC_GameTraceChannel4);
	Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	Capsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	Capsule->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore); // EnemyProjectile
	Capsule->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block); // ReflectedProjectile
	Capsule->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap); // セイバー

	Capsule->SetGenerateOverlapEvents(true);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Capsule);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FirePoint = CreateDefaultSubobject<USceneComponent>(TEXT("FirePoint"));
	FirePoint->SetupAttachment(Root);

	//レーザーサイトの作成
	//まずピポッドをファイヤーポイントにつける
	NewLaserPivot = CreateDefaultSubobject<USceneComponent>(TEXT("LaserPivot"));
	NewLaserPivot->SetupAttachment(FirePoint);

	//メッシュをピポッドの子にする
	LaserMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaserMesh"));
	LaserMesh->SetupAttachment(NewLaserPivot);

	//メッシュを半分ずらしてPivotが底に来るようにする
	LaserMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

	LaserMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LaserMesh->SetHiddenInGame(true);	//レーザーを最初は隠しておく
}

void AN_IkaEnemy1::BeginPlay()
{
	Super::BeginPlay();

	if (Mesh) Mesh->SetHiddenInGame(true);				// メッシュを隠す
	if (LaserMesh) LaserMesh->SetHiddenInGame(true);	// レーザーも隠す
	bIsMove = false;

	// 自動スプライン取得ロジック
	if (ActorHasTag(FName("IsMover"))) {
		TArray<AActor*> FoundSplines;
		//"Move"タグを持つスプラインをすべて探す
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Move"), FoundSplines);

		float NearestDist = 999999.f;
		AActor* NearestActor = nullptr;

		for (AActor* SplineActor : FoundSplines) {
			float Dist = FVector::Dist(GetActorLocation(), SplineActor->GetActorLocation());
			if (Dist < NearestDist) {
				NearestDist = Dist;
				NearestActor = SplineActor;
				TargetSpline = Cast<AA_EnemySpline>(SplineActor);
			}
		}
		if (NearestActor) {
			TargetSpline = Cast<AA_EnemySpline>(NearestActor);
		}
	}

	if (TargetSpline) {
		SplineComponent = TargetSpline->FindComponentByClass<USplineComponent>();
		if (SplineComponent) {
			// FindDistanceAlongSplineAtLocation の代わりにこれを使う
			float ClosestKey = SplineComponent->FindInputKeyClosestToWorldLocation(GetActorLocation());
			CurrentSplineDistance = SplineComponent->GetDistanceAlongSplineAtSplineInputKey(ClosestKey);

			UE_LOG(LogTemp, Warning, TEXT("Spline Successfully Linked and Distance Initialized!"));
		}
	}

	if (TargetSpline) {
		SplineComponent = TargetSpline->FindComponentByClass<USplineComponent>();
		if (!SplineComponent) {
			UE_LOG(LogTemp, Warning, TEXT("SplineComponent Successfully Linked!"));
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("SplineActor is not found on TargetSpline"));
		}
	}

	DynMat = Mesh->CreateAndSetMaterialInstanceDynamic(0);

	// 最後に自分を呼び出す
	PlaySpawnTransition(true);
}

//攻撃していいかマネージャーに聞く
void AN_IkaEnemy1::TryFireProjectile() {
	if (!bIsMove) {
		ResetAttackCycle();
		return;
	}

	AN_AGameMode_Main* GM = Cast<AN_AGameMode_Main>(UGameplayStatics::GetGameMode(this));
	if (GM && GM->RequestAttackToken()) {
		StartAnticipation(); //許可が出たら予兆へ
	}
	else {
		//許可が出なければ0.5秒後に再試行
		GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AN_IkaEnemy1::TryFireProjectile, 0.5f, false);
	}
}

//レーザーサイトを表示して狙いを定める
void AN_IkaEnemy1::StartAnticipation() {
	if (LaserMesh) {
		LaserMesh->SetHiddenInGame(false);
		CurrentLaserLength = 0.0f;
	}

	//レーザー音の再生処理を追加
	if (LaserChargeSound) {
		//3D空間上の衝突位置から音を鳴らす
		UGameplayStatics::PlaySoundAtLocation(
			this,
			LaserChargeSound,
			FirePoint->GetComponentLocation(),
			0.2f,                 //音量
			2.f,                  //ピッチ
			0.0f                  //開始時間
		);
	}

	//ここでレーザーの向きをプレイヤーに向ける（Tickで更新してもOK）
	UpdateLaserTarget(0.0f);

	//予兆時間（溜め時間）をランダムにして作業感をなくす
	float ChargeTime = FMath::FRandRange(0.5f, 1.0f);
	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AN_IkaEnemy1::ExecuteFire, ChargeTime, false);
}

//発射
void AN_IkaEnemy1::ExecuteFire() {
	// 既存の弾丸生成ロジックを実行
	FireProjectile();

	//レーザー音を止める処理を追加
	if (LaserAudioComp && LaserAudioComp->IsPlaying()) {
		LaserAudioComp->Stop();
	}

	//レーザーを消す
	if (LaserMesh) {
		LaserMesh->SetHiddenInGame(true);
	}

	//トークンを返却
	if (AN_AGameMode_Main* GM = Cast<AN_AGameMode_Main>(UGameplayStatics::GetGameMode(this))) {
		if (GM) {
			GM->ReleaseAttackToken();
		}
	}

	ResetAttackCycle();
}

//レーザーの向きを更新する
void AN_IkaEnemy1::UpdateLaserTarget(float DeltaTime) {
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn || !LaserMesh || !FirePoint || !NewLaserPivot) return;

	//カメラの位置を取得
	UCameraComponent* Camera = PlayerPawn->FindComponentByClass<UCameraComponent>();
	FVector EyeLocation = Camera ? Camera->GetComponentLocation() : PlayerPawn->GetActorLocation();

	//目線から少し下（胸元あたり）にターゲットをずらす
	//Z軸方向にマイナス
	FVector TargetPos = EyeLocation - FVector(0.0f, 0.0f, 50.0f);

	FVector Start = FirePoint->GetComponentLocation();
	float TargetDistance = FVector::Dist(Start, TargetPos);

	//ビームを伸ばす
	CurrentLaserLength = FMath::FInterpConstantTo(CurrentLaserLength, TargetDistance, DeltaTime, LaserExtendSpeed);

	//向きを計算
	FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(Start, TargetPos);

	NewLaserPivot->SetWorldRotation(LookRot);
	NewLaserPivot->SetWorldLocation(Start);

	//スケール適用 (Y軸を伸ばす)
	float ScaleY = CurrentLaserLength / 100.0f;

	LaserMesh->SetRelativeScale3D(FVector(0.1f,ScaleY, 0.1f));

	LaserMesh->SetRelativeLocation(FVector(CurrentLaserLength * 0.5f, 0.0f, 0.0f));
}

//次の攻撃までのインターバル
void AN_IkaEnemy1::ResetAttackCycle() {
	// 次回までの間隔をランダムにしてバラけさせる
	float NextInterval = fFireInterval + FMath::FRandRange(-1.5f, 1.5f);
	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AN_IkaEnemy1::TryFireProjectile, NextInterval, false);
}


void AN_IkaEnemy1::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//レーザー表示中のみ、プレイヤーを追いかける
	if (LaserMesh && !LaserMesh->bHiddenInGame) {
		UpdateLaserTarget(DeltaTime);
	}

	//スプラインで移動する処理
	if (SplineComponent && ActorHasTag(FName("IsMover"))) {
		const float SplineLength = SplineComponent->GetSplineLength();

		if (bMovingForward) {
			CurrentSplineDistance += fMoveSpeed * DeltaTime;
			if (CurrentSplineDistance >= SplineLength) {
				CurrentSplineDistance = SplineLength;
				bMovingForward = false;
			}
		}
		else {
			CurrentSplineDistance -= fMoveSpeed * DeltaTime;
			if (CurrentSplineDistance <= 0.f) {
				CurrentSplineDistance = 0.f;
				bMovingForward = true;
			}
		}


		const FVector NewLocation = SplineComponent->GetLocationAtDistanceAlongSpline(CurrentSplineDistance, ESplineCoordinateSpace::World);
		
		//向きの計算
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
		if (PlayerPawn) {
			FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), PlayerPawn->GetActorLocation());
			SetActorRotation(LookAtRot);
		}

		SetActorLocation(NewLocation);
	}
}

//プレイヤーに向かって敵の弾発射
void AN_IkaEnemy1::FireProjectile()
{
	if (!EnemyProjectile) return;

	//ゲームが行われているか確認
	AN_AGameMode_Main* GM = Cast<AN_AGameMode_Main>(UGameplayStatics::GetGameMode(this));
	if (GM && !GM->IsGameActive())
	{
		//ゲーム終了してるのでタイマー止めて終わり
		GetWorldTimerManager().ClearTimer(FireTimerHandle);
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;

	UCameraComponent* Camera =
		PlayerPawn->FindComponentByClass<UCameraComponent>();
	if (!Camera) return;

	FVector Start = FirePoint->GetComponentLocation();
	FVector Target = Camera->GetComponentLocation();

	FRotator LookRot = (Target - Start).Rotation();

	FActorSpawnParameters Params;
	Params.Owner = this;


	AActor* SpawnedProjectile = GetWorld()->SpawnActor<AActor>(
		EnemyProjectile,
		Start,
		LookRot,
		Params
	);

	if (SpawnedProjectile)
	{
		//弾の中にある ProjectileMovementComponent を探す
		UProjectileMovementComponent* MoveComp = SpawnedProjectile->FindComponentByClass<UProjectileMovementComponent>();
		if (MoveComp)
		{
			//1200.0 ～ 1500.0 の間でランダムな速度を決定
			float RandomSpeed = FMath::FRandRange(1200.0f, 1500.0f);

			MoveComp->InitialSpeed = RandomSpeed;
			MoveComp->MaxSpeed = RandomSpeed;

			//既に動き出している場合があるので、速度ベクトルを直接更新する
			MoveComp->Velocity = LookRot.Vector() * RandomSpeed;
		}
	}
}

//送られてきた引数分ダメージを受ける
void AN_IkaEnemy1::TakeEnemyDamage(float Damage)
{
	fEnemyHP -= Damage;
	//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Damage"));
	FlashRed();

	//ヒットを鳴らす
	if (EnemyHitSound)
	{
		//少し低い音と高い間でランダムな数値を作る
		float RandomPitch = FMath::FRandRange(1.3f, 1.6f);

		//3D空間上の衝突位置から音を鳴らす
		UGameplayStatics::PlaySoundAtLocation(
			this,
			EnemyHitSound,
			GetActorLocation(),  //弾の位置
			1.f,                 //音量
			RandomPitch,         //ピッチ
			0.f                  //開始時間
		);
	}

	if (fEnemyHP <= 0)
	{
		//やられ音を鳴らす
		if (EnemyDeathSound)
		{
			//3D空間上の衝突位置から音を鳴らす
			UGameplayStatics::PlaySoundAtLocation(
				this,
				EnemyDeathSound,
				GetActorLocation(),  //弾の位置
				1.f,                 //音量
				1.f,                 //ピッチ
				0.5f                 //開始時間
			);
		}

		//爆発エフェクト
		if (DeathEffect)
		{
			// 指定した位置（敵の現在地）にエフェクトを発生させる
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				DeathEffect,
				GetActorLocation(),
				GetActorRotation()
			);
		}

		//GameModeを取得してカウントを増やす
		//攻撃トークンも返す
		AN_AGameMode_Main* GM = Cast<AN_AGameMode_Main>(UGameplayStatics::GetGameMode(this));
		if (GM)
		{
			GM->AddEnemyKillCount();
			GM->ReleaseAttackToken();
		}

		Destroy();
	}
}

//ダメージを受けた時に赤く光る
void AN_IkaEnemy1::FlashRed()
{
	if (!DynMat) return;

	DynMat->SetVectorParameterValue("EmissiveColor", FLinearColor::Red);
	DynMat->SetScalarParameterValue("EmissiveStrength", 10.f);

	FTimerHandle Timer;
	GetWorldTimerManager().SetTimer(Timer, [this]()
		{
			if (DynMat)
			{
				DynMat->SetScalarParameterValue("EmissiveStrength", 0.f);
			}
		}, 0.1f, false);
}

//剣が物理的に当たった時の処理（実際いらないけど近づけば切れる要素入れとく）
void AN_IkaEnemy1::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	//何かが重なった瞬間にログを出す
	if (OtherActor)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::White, FString::Printf(TEXT("Overlap: %s"), *OtherActor->GetName()));
	}

	//セイバー判定のデバッグ
	bool bSaberHit = false;
	TArray<UPrimitiveComponent*> OverlappingComps;

	//敵の全コンポーネントの重なりを取得
	GetOverlappingComponents(OverlappingComps);

	for (UPrimitiveComponent* Comp : OverlappingComps)
	{
		if (Comp && Comp->ComponentHasTag(TEXT("Saber")))
		{
			bSaberHit = true;
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("Saber Tag Found!"));
			break;
		}
	}

	if (bSaberHit)
	{
		//爆発エフェクト
		if (DeathEffect)
		{
			//敵の現在地にエフェクトを発生させる
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				DeathEffect,
				GetActorLocation(),
				GetActorRotation()
			);
		}

		//GameModeを取得してカウントを増やす、（テイクダメージのとこにも同じ処理あり）
		AN_AGameMode_Main* GM = Cast<AN_AGameMode_Main>(UGameplayStatics::GetGameMode(this));
		if (GM)
		{
			GM->AddEnemyKillCount();
		}

	}
}

void AN_IkaEnemy1::PlaySpawnTransition(bool bIsAppearing)
{
	//if (!GroundEffect || !PillarEffect) return;

	FVector Loc = GetActorLocation();
	FRotator Rot = GetActorRotation();

	if (bIsAppearing) {
		//出現パターン
		if (GroundEffect) {
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), GroundEffect, Loc, Rot);
		}

		//0.5秒待ってから敵本体を表示して動き出させる
		FTimerHandle ShowEnemyTimer;
		GetWorldTimerManager().SetTimer(ShowEnemyTimer, this, &AN_IkaEnemy1::FinishSpawnTransition, 0.8f, false);
	}
	else {
		//退場パターン
		//0.3秒後に「上に伸びる光」を出す
		FTimerHandle EffectTimer;
		GetWorldTimerManager().SetTimer(EffectTimer, [this, Loc, Rot]() {
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), PillarEffect, Loc, Rot);

			// 敵の姿を隠す
			if (Mesh) Mesh->SetHiddenInGame(true);
			if (LaserMesh) LaserMesh->SetHiddenInGame(true);

			//少し待ってから実際にDestroy
			FTimerHandle DestroyTimer;
			GetWorldTimerManager().SetTimer(DestroyTimer, this, &AN_IkaEnemy1::ExecuteDestroy, 0.5f, false);
			}, 0.3f, false);
	}
}

void AN_IkaEnemy1::ExecuteDestroy() {
	Destroy();
}

void AN_IkaEnemy1::FinishSpawnTransition()
{
	//メッシュを表示
	if (Mesh) Mesh->SetHiddenInGame(false);

	//移動許可
	bIsMove = true;

	//攻撃サイクルをここから開始する
	float InitialDelay = FMath::FRandRange(1.0f, fFireInterval);
	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AN_IkaEnemy1::TryFireProjectile, InitialDelay, false);

	UE_LOG(LogTemp, Warning, TEXT("Spawn Transition Finished: Enemy is now active!"));
}

void AN_IkaEnemy1::SetIsMove(bool IsMove) {
	bIsMove = IsMove;
}

void AN_IkaEnemy1::SetMoveSpeed(float NewSpeed) {
	fMoveSpeed = NewSpeed;
}

void AN_IkaEnemy1::SetFireInterval(float NewInterval) {
	fFireInterval = NewInterval;
}

