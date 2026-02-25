//N_EnemyProjectile.cpp

#include "N_EnemyProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "Math/UnrealMathUtility.h"

#include "N_IkaEnemy1.h"
#include "N_VRPawn.h"
#include "N_Saber.h"

#include "DrawDebugHelpers.h"//画面デバッグ用

AN_EnemyProjectile::AN_EnemyProjectile() :
	fAssistAngleDeg(30.f),
	fMaxAssistDist(4000.f)
{
	PrimaryActorTick.bCanEverTick = false;

	// Collision
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(8.f);
	//Collision->SetCollisionProfileName(TEXT("Projectile"));

	Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//オブジェクトタイプにEnemyProjectileをセット
	Collision->SetCollisionObjectType(ECC_GameTraceChannel1);

	//全無視、必要なものをBlockに変える
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	//壁
	Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	//プレイヤー
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	//セイバー
	Collision->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Block);
	//反射弾は無視する
	Collision->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore);

	SetRootComponent(Collision);

	//Mesh
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//ProjectileMovement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 1500.f;
	ProjectileMovement->MaxSpeed = 1500.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;

	//Hit Event
	Collision->OnComponentHit.AddDynamic(this, &AN_EnemyProjectile::OnHit);
}

void AN_EnemyProjectile::BeginPlay()
{
	Super::BeginPlay();

    //５秒後に自動消滅処理
    SetLifeSpan(10.0f);

	DynMat = Mesh->CreateAndSetMaterialInstanceDynamic(0);
}


//void AN_EnemyProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
//	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
//{
//	if (!OtherActor || OtherActor == this) return;
//
//	// 敵に当たったら敵にダメージ
//	if (OtherActor && OtherActor->ActorHasTag(TEXT("Enemy"))) {
//		if (AN_IkaEnemy1* Enemy = Cast<AN_IkaEnemy1>(OtherActor)) {
//			Enemy->TakeEnemyDamage(CurrentReflectDamage);
//		}
//		Destroy();
//		return;
//	}
//
//	//// --- OnHitの中のSaber取得部分を以下に差し替え ---
//
//	//AN_Saber* Saber = nullptr;
//
//	//// 1. 直接キャストを試みる
//	//Saber = Cast<AN_Saber>(OtherActor);
//
//	//// 2. 直接ダメなら、当たったコンポーネントの「持ち主」を確認する
//	//if (!Saber && OtherComp)
//	//{
//	//	Saber = Cast<AN_Saber>(OtherComp->GetOwner());
//	//}
//
//	//// 3. それでもダメなら「チャイルドあクタ」の関係性を辿る
//	//if (!Saber && OtherComp)
//	//{
//	//	// コンポーネントがアタッチされている親（ChildActorComponent）を探す
//	//	UObject* LoopTarget = OtherComp;
//	//	while (LoopTarget)
//	//	{
//	//		// チャイルドあクタコンポーネントを見つけたら、その中身（Actor）を取得
//	//		if (UChildActorComponent* CAC = Cast<UChildActorComponent>(LoopTarget))
//	//		{
//	//			Saber = Cast<AN_Saber>(CAC->GetChildActor());
//	//			if (Saber) break;
//	//		}
//	//		// 親へと辿っていく
//	//		LoopTarget = LoopTarget->GetOuter();
//	//	}
//	//}
//
//	//// 4. 最終手段：アクターが持っている「全コンポーネント」からSaberクラスを探す
//	//if (!Saber && OtherActor)
//	//{
//	//	TArray<AActor*> AttachedActors;
//	//	OtherActor->GetAttachedActors(AttachedActors);
//	//	for (AActor* Attached : AttachedActors)
//	//	{
//	//		if (AN_Saber* FoundSaber = Cast<AN_Saber>(Attached))
//	//		{
//	//			Saber = FoundSaber;
//	//			break;
//	//		}
//	//	}
//	//}
//
//	//剣に当たった時の処理
//	AN_Saber* Saber = Cast<AN_Saber>(OtherActor);
//
//	// OtherActorがSaberでない場合（VRPawnなど）、コンポーネントから逆引きする
//	if (!Saber && OtherComp) {
//		// ChildActorComponent経由で付いている場合、その中身（Actor）を取得
//		if (UChildActorComponent* CAC = Cast<UChildActorComponent>(OtherComp->GetAttachParent())) {
//			Saber = Cast<AN_Saber>(CAC->GetChildActor());
//		}
//		// それでもダメなら、剣のタグを持っているかチェック
//		else {
//			// 直接の親がSaberクラスか確認
//			Saber = Cast<AN_Saber>(OtherComp->GetOwner());
//		}
//	}
//
//	if (Saber)
//	{
//		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Hansya!"));
//
//		//基本パラメータ取得
//		float Speed = ProjectileMovement->Velocity.Size();
//		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
//		if (!PlayerPawn) return;
//		UCameraComponent* Camera = PlayerPawn->FindComponentByClass<UCameraComponent>();
//		if (!Camera) return;
//
//
//		//セーバーから状態を取得
//		ESwingState SwingState = Saber->GetCurrentSwingState();
//		float CurrentSwingSpeed = Saber->GetSaberSpeed();
//		FVector TipLocation = OtherComp->GetComponentLocation();
//
//		float FinalDamage = 1.0f;
//		float VisualIntensity = 50.0f;
//		FLinearColor ReflectColor = FLinearColor::Blue;
//		float TimeStopDuration = 0.0f;
//		bIsStrongHit = false;
//
//		switch (SwingState)
//		{
//		case ESwingState::HighSpeed:
//			FinalDamage = 5.0f;
//			ReflectColor = FLinearColor::Red;
//			VisualIntensity = 100.f;
//			bIsStrongHit = true;
//			// 高速時はタイムディレーションをかけるなど
//			break;
//
//		case ESwingState::LowSpeed:
//			FinalDamage = 3.0f;
//			ReflectColor = FLinearColor::Green;
//			VisualIntensity = 70.f;
//			bIsStrongHit = false;
//			break;
//
//		case ESwingState::Idle:
//		default:
//			// 止まっている剣に当たった場合は跳ね返さない、あるいは弱く落とす
//			FinalDamage = 0.5f;
//			VisualIntensity = 20.f;
//			bIsStrongHit = false;
//			break;
//		}
//
//
//		////テクニック判定
//		//float CurrentSwingSpeed = Saber->GetSaberSpeed();
//		//FVector TipLocation = OtherComp->GetComponentLocation();
//		//float ClampedSpeed = FMath::Clamp(CurrentSwingSpeed, 0.f, 5000.f);
//		//bIsStrongHit = (ClampedSpeed > MinSwingSpeed);
//
//		float DisToTip = FVector::Dist(Hit.ImpactPoint, TipLocation);
//		bIsSweetSpot = (DisToTip < SweetSpotRang);
//		bIsPerfectTiming = (FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation()) < 100.f);
//
//		//デバッグ
//		//DrawDebugSphere(GetWorld(), TipLocation, SweetSpotRang, 12, FColor::Yellow, false, 1.f);
//
//		//if (GEngine) {
//		//	FColor LogColor = bIsStrongHit ? FColor::Red : FColor::Cyan;
//		//	FString DebugMsg = FString::Printf(TEXT("SwingSpeed: %.1f / Min: %.1f"), ClampedSpeed, MinSwingSpeed);
//		//	GEngine->AddOnScreenDebugMessage(2, 2.f, LogColor, DebugMsg);
//		//}
//
//		//威力と演出の決定
//		float FinalDamage = 1.0f;
//		float VisualIntensity = 50.0f;
//		FLinearColor ReflectColor = FLinearColor(0.f, 0.f, 1.0f); // 基本は青
//		float TimeStopDuration = 0.0f;
//
//		//パターンA：スピード重視
//		if (bIsStrongHit) {
//			FinalDamage = 3.0f;
//			ReflectColor = FLinearColor(0.0f, 1.f, 0.f); // 緑っぽく
//			VisualIntensity = 100.f;
//			//TimeStopDuration = 0.05f;
//		}
//
//		//パターンB：スポットに当たったら
//		if (bIsSweetSpot)
//		{
//			FinalDamage = 5.0f;
//			ReflectColor = FLinearColor(1.0f, 1.0f, 0.0f); // 金色
//			VisualIntensity = 200.f; // 眩しくする
//			TimeStopDuration = 0.05f;
//			// スイートスポット時のみ派手な音を鳴らす
//			if (SaberHitSound) UGameplayStatics::PlaySoundAtLocation(this, SaberHitSound, GetActorLocation(), 1.2f, 1.5f);
//		}
//
//		//テクニック判定の直後に追加
//		if (GEngine) {
//			FColor LogColor = bIsStrongHit ? FColor::Red : FColor::Cyan;
//			FString DebugMsg = FString::Printf(TEXT("SwingSpeed: %1.f / Target: %1.f"), ClampedSpeed, MinSwingSpeed);
//			GEngine->AddOnScreenDebugMessage(2, 2.f, LogColor, DebugMsg);
//		}
//
//		//反射威力決定
//		CurrentReflectDamage = FinalDamage;
//
//		// --- 演出への反映 ---
//		if (DynMat) {
//			DynMat->SetVectorParameterValue("EmissiveColor", ReflectColor);
//			DynMat->SetScalarParameterValue("EmissiveStrngth", VisualIntensity);
//		}
//
//		//ヒットストップの実行
//		if (TimeStopDuration > 0.0f) {
//			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.1f);
//			FTimerHandle StopTimerHandle;
//			//通常速度に戻す
//			FTimerDelegate TimerDel;
//			TimerDel.BindLambda([this]() { if (GetWorld()) UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);});
//			GetWorldTimerManager().SetTimer(StopTimerHandle, TimerDel, TimeStopDuration, false, -1.0f);
//		}
//
//		//振動処理
//		if (APlayerController* PC = Cast<APlayerController>(PlayerPawn->GetController())) {
//			EControllerHand Hand = OtherComp->ComponentHasTag(TEXT("Right")) ? EControllerHand::Right : EControllerHand::Left;
//			if (SaberHitHaptic) {
//				float HapticScale = FMath::GetMappedRangeValueClamped(FVector2D(0.f, 2000.f), FVector2D(0.3f, 1.f), ClampedSpeed);
//				// 強反射(bIsStrongHit)ならさらに少し盛る、という調整もアリ
//				if (bIsStrongHit) HapticScale = FMath::Clamp(HapticScale * 1.5f, 0.f, 1.f);
//				PC->PlayHapticEffect(SaberHitHaptic, Hand, HapticScale);
//			}
//		}
//
//		//反射方向の決定
//		AActor* BestTarget = nullptr;
//		float BestDot = 0.0f;
//		TArray<AActor*> Enemies;
//		UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("Enemy"), Enemies);
//
//		for (AActor* Enemy : Enemies) {
//			FVector ToEnemy = Enemy->GetActorLocation() - Camera->GetComponentLocation();
//			if (ToEnemy.Size() > fMaxAssistDist) continue;
//			float Dot = FVector::DotProduct(Camera->GetForwardVector(), ToEnemy.GetSafeNormal());
//			if (FMath::RadiansToDegrees(FMath::Acos(Dot)) < fAssistAngleDeg && (Dot > BestDot)) {
//				BestDot = Dot;
//				BestTarget = Enemy;
//			}
//		}
//
//		FVector TargetDir;
//		if (BestTarget) {
//			TargetDir = (BestTarget->GetActorLocation() + FVector(0, 0, 30.f) - GetActorLocation()).GetSafeNormal();
//		}
//		else {
//			TargetDir = UKismetMathLibrary::GetReflectionVector(ProjectileMovement->Velocity.GetSafeNormal(), Hit.ImpactNormal);
//		}
//
//		//反射弾への切り替え設定
//		bReflected = true;
//		Collision->SetCollisionObjectType(ECC_GameTraceChannel2); // ReflectedProjectile
//		Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
//		Collision->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Block); // 敵
//		Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);       // 壁
//		Collision->IgnoreComponentWhenMoving(OtherComp, true);
//
//		ProjectileMovement->Velocity = TargetDir * Speed;
//
//		//反射音を鳴らす
//		if (SaberHitSound)
//		{
//			//少し低い音と高い間でランダムな数値を作る
//			float RandomPitch = FMath::FRandRange(0.9f, 1.2f);
//
//			//3D空間上の衝突位置から音を鳴らす
//			UGameplayStatics::PlaySoundAtLocation(
//				this,
//				SaberHitSound,
//				GetActorLocation(),  //弾の位置
//				1.f,                 //音量
//				RandomPitch,         //ピッチ
//				0.f                  //開始時間
//			);
//		}
//		//火花エフェクトを出す
//		if (SparkEffect)
//		{
//			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
//				GetWorld(),
//				SparkEffect,
//				Hit.ImpactPoint, // 実際に剣と弾がぶつかった座標
//				Hit.ImpactNormal.Rotation() // ぶつかった面の向きに合わせる
//			);
//		}
//
//		return;	 //Destroyしない
//	}
//
//	//プレイヤーに当たったらノーダメージ
//	if (AN_VRPawn* Player = Cast<AN_VRPawn>(OtherActor)){
//		//反射済みの弾なら、プレイヤーにはダメージを与えない
//		if (bReflected) return;
//		Player->TakePlayerDamage(10.f);
//		Destroy();
//	}
//	Destroy();
//}

void AN_EnemyProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!OtherActor || OtherActor == this) return;

    //敵に当たった時の処理
    if (OtherActor->ActorHasTag(TEXT("Enemy"))) {
        if (AN_IkaEnemy1* Enemy = Cast<AN_IkaEnemy1>(OtherActor)) {
            Enemy->TakeEnemyDamage(CurrentReflectDamage);
        }
        Destroy();
        return;
    }

    //セーバーの取得
    AN_Saber* Saber = Cast<AN_Saber>(OtherActor);
    if (!Saber && OtherComp) {
        // ChildActorComponentやOwnerからセーバーを探す
        if (UChildActorComponent* CAC = Cast<UChildActorComponent>(OtherComp->GetAttachParent())) {
            Saber = Cast<AN_Saber>(CAC->GetChildActor());
        }
        else {
            Saber = Cast<AN_Saber>(OtherComp->GetOwner());
        }
    }

    //反射判定
    if (Saber)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
        if (!PlayerPawn) return;
        UCameraComponent* Camera = PlayerPawn->FindComponentByClass<UCameraComponent>();

        ESwingState SwingState = Saber->GetCurrentSwingState();
        FVector TipLocation = OtherComp->GetComponentLocation();
        float DistToTip = FVector::Dist(Hit.ImpactPoint, TipLocation);

        //各種判定フラグのセット
        bIsSweetSpot = (DistToTip < SweetSpotRang);
        bIsStrongHit = (SwingState == ESwingState::HighSpeed);

        //反射の種類を出す
        float FinalDamage = 0.0f;
        FLinearColor ReflectColor = FLinearColor::White;
        float VisualIntensity = 50.0f;
        float TimeStopDuration = 0.0f;

        if (SwingState == ESwingState::Idle) {
            if (bIsSweetSpot) {
                // 【停 + スイート】ダメージ1で反射
                FinalDamage = 1.0f;
                ReflectColor = FLinearColor(0.7f, 1.0f, 0.0f); //黄色
                VisualIntensity = 50.0f;
                GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("1"));
            }
            else {
                // 【停 + 普通】反射なし（弾を消して終了）
                GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::White, TEXT("0"));

                // 消滅音を鳴らす
                if (ProjectileDestroySound) {
                    UGameplayStatics::PlaySoundAtLocation(
                        this,
                        ProjectileDestroySound,
                        GetActorLocation(),
                        2.0f, // 少し音量を抑えめにしてもいいかもしれません
                        1.2f  // 弾けた感じを出すためにピッチを少し高めに
                    );
                }

                Destroy();
                return;
            }
        }
        else if (SwingState == ESwingState::LowSpeed) {
            if (bIsSweetSpot) {
                // 【弱 + スイート】ダメージ3
                FinalDamage = 3.0f;
                ReflectColor = FLinearColor(1.0f, 0.1f, 0.0f); // オレンジ
                VisualIntensity = 100.f;
                GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("3"));
            }
            else {
                // 【弱 + 普通】ダメージ1
                FinalDamage = 1.0f;
                ReflectColor = FLinearColor(0.7f, 1.0f, 0.0f); // 黄色
                VisualIntensity = 50.0f;
                GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::White, TEXT("2"));
            }
        }
        else if (SwingState == ESwingState::HighSpeed) {
            if (bIsSweetSpot) {
                // 【強 + スイート】ダメージ5
                FinalDamage = 5.0f;
                ReflectColor = FLinearColor(1.0f, 0.0f, 0.6f); // 紫
                VisualIntensity = 250.f;
                TimeStopDuration = 0.05f; // 強ヒットの演出
                GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("5"));
            }
            else {
                // 【強 + 普通】ダメージ3
                FinalDamage = 3.0f;
                ReflectColor = FLinearColor(1.0f, 0.1f, 0.0f); // オレンジ
                VisualIntensity = 100.f;
                GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::White, TEXT("4"));
            }
        }

        //演出とパラメータの適用
        CurrentReflectDamage = FinalDamage;

        if (DynMat) {
            DynMat->SetVectorParameterValue("EmissiveColor", ReflectColor);
            DynMat->SetScalarParameterValue("EmissiveStrngth", VisualIntensity);
        }

        //ヒットストップ
        if (TimeStopDuration > 0.0f) {
            UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.1f);
            FTimerHandle StopTimerHandle;
            FTimerDelegate TimerDel;
            TimerDel.BindLambda([this]() { if (GetWorld()) UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f); });
            GetWorldTimerManager().SetTimer(StopTimerHandle, TimerDel, TimeStopDuration, false, -1.0f);
        }

        //反射ベクトルの計算
        AActor* BestTarget = nullptr;
        float BestDot = 0.0f;
        TArray<AActor*> Enemies;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("Enemy"), Enemies);

        for (AActor* Enemy : Enemies) {
            FVector ToEnemy = Enemy->GetActorLocation() - Camera->GetComponentLocation();
            if (ToEnemy.Size() > fMaxAssistDist) continue;
            float Dot = FVector::DotProduct(Camera->GetForwardVector(), ToEnemy.GetSafeNormal());
            if (FMath::RadiansToDegrees(FMath::Acos(Dot)) < fAssistAngleDeg && (Dot > BestDot)) {
                BestDot = Dot;
                BestTarget = Enemy;
            }
        }

        FVector TargetDir;
        if (BestTarget) {
            TargetDir = (BestTarget->GetActorLocation() + FVector(0, 0, 30.f) - GetActorLocation()).GetSafeNormal();
            
            //強振り 兼 スイートスポットの場合のみホーミングさせる
            if (bIsStrongHit && bIsSweetSpot) {
                ProjectileMovement->bIsHomingProjectile = true;
                //敵のRootComponentをターゲットに設定
                ProjectileMovement->HomingTargetComponent = BestTarget->GetRootComponent();
                ProjectileMovement->HomingAccelerationMagnitude = HomingStrength;

                GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("HOMING ACTIVATED!"));
            }
        }
        else {
            ProjectileMovement->bIsHomingProjectile = false;
            TargetDir = UKismetMathLibrary::GetReflectionVector(ProjectileMovement->Velocity.GetSafeNormal(), Hit.ImpactNormal);
        }

        //弾の性質を「味方の弾」に変更
        bReflected = true;
        SetLifeSpan(6.0f);
        Collision->SetCollisionObjectType(ECC_GameTraceChannel2);
        Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
        Collision->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Block); // 敵
        Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);       // 壁
        Collision->IgnoreComponentWhenMoving(OtherComp, true);

        float Speed = ProjectileMovement->Velocity.Size();
        ProjectileMovement->Velocity = TargetDir * Speed;

        //効果音・エフェクト
        if (SaberHitSound) {
            UGameplayStatics::PlaySoundAtLocation(this, SaberHitSound, GetActorLocation(), 1.f, FMath::FRandRange(0.9f, 1.2f));
        }
        if (SparkEffect) {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SparkEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
        }

        //振動処理の復活
        if (APlayerController* PC = Cast<APlayerController>(PlayerPawn->GetController())) {
            // コンポーネントのタグ、またはその親アクターのタグに "Right" が含まれているかチェック
            bool bIsRight = OtherComp->ComponentHasTag(TEXT("Right")) || (OtherActor && OtherActor->ActorHasTag(TEXT("Right")));

            EControllerHand Hand = bIsRight ? EControllerHand::Right : EControllerHand::Left;

            if (SaberHitHaptic) {
                GEngine->AddOnScreenDebugMessage(-1, 1.f, bIsRight ? FColor::Red : FColor::Blue, bIsRight ? TEXT("Haptic: Right") : TEXT("Haptic: Left"));

                //基本の強さを設定
                float HapticScale = 0.5f;

                //強振り(bIsStrongHit)やスイートスポット(bIsSweetSpot)なら振動を強くする
                if (bIsStrongHit) HapticScale += 0.3f;
                if (bIsSweetSpot) HapticScale += 0.2f;

                // 振動実行
                PC->PlayHapticEffect(SaberHitHaptic, Hand, FMath::Clamp(HapticScale, 0.f, 1.f));
            }
        }

        return; //反射成功時はDestroyせずに戻る
    }

    //セーバー以外（プレイヤー自身など）に当たった場合
    if (OtherActor->IsA(AN_VRPawn::StaticClass())) {
        if (!bReflected) {
            AN_VRPawn* Player = Cast<AN_VRPawn>(OtherActor);
            Player->TakePlayerDamage(10.f);
        }
        else {
            return; //自分の弾なら無視
        }
    }
    Destroy();
}
