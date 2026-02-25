//N_AGameMode_Main.cpp

#include "N_AGameMode_Main.h"
#include "N_EnemyProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"
#include "A_EnemySpline.h"
#include "N_VRPawn.h"
#include "N_MyGameInstance.h"
#include "Kismet/KismetMathLibrary.h"

#include "DrawDebugHelpers.h"//画面デバッグ用

void AN_AGameMode_Main::StartInGame()
{
	UGameplayStatics::OpenLevel(this, FName("MRInGame"));

	//AudioComponentを生成してアタッチ
	BGMAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("BGMAudioComp"));
	BGMAudioComp->SetupAttachment(RootComponent);

	//ゲーム開始時に自動再生するかどうか
	BGMAudioComp->bAutoActivate = true;
}

void AN_AGameMode_Main::BeginPlay()
{
	Super::BeginPlay();

	// BGMがセットされていれば再生
	if (BackgroundMusic)
	{
		BGMAudioComp->SetSound(BackgroundMusic);
		BGMAudioComp->SetVolumeMultiplier(0.5f); // 音量調整
		BGMAudioComp->Play();
	}

	// 正しいレベル（インゲーム画面）にいる時だけタイマーを回す
	FString CurrentLevelName = GetWorld()->GetMapName();
	if (CurrentLevelName.Contains(TEXT("MRInGame")))
	{
		CurrentPhase = EGamePhase::Front;
		bIsGameActive = true;

		// フェーズチェンジ準備を開始する
		GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &AN_AGameMode_Main::PreparePhaseChange, 25.0f, true);

		// 敵スポーンタイマー
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AN_AGameMode_Main::SpawnEnemyRandomly, 4.0f, true);
	}
}

void AN_AGameMode_Main::PreparePhaseChange()
{
	//全敵の攻撃を止める
	TArray<AActor*> OutEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), EnemyClass, OutEnemies);

	for (AActor* Enemy : OutEnemies) {
		AN_IkaEnemy1* Ika = Cast<AN_IkaEnemy1>(Enemy);
		if (Ika) {
			Ika->SetIsMove(false);
			if (Ika->LaserMesh) Ika->LaserMesh->SetHiddenInGame(true); // レーザー消す
		}
	}

	//5秒後に本番のAdvancePhaseを呼ぶ
	FTimerHandle TransitionTimer;
	GetWorldTimerManager().SetTimer(TransitionTimer, this, &AN_AGameMode_Main::AdvancePhase, 5.0f, false);
}

void AN_AGameMode_Main::AdvancePhase()
{
	//次のフェーズに行く前に今いる敵全員を逃走させる
	TArray<AActor*> OutEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), EnemyClass, OutEnemies);

	// 攻撃中だった場合に備えてトークンだけ返しておく
	ReleaseAttackToken();

	for (AActor* Enemy : OutEnemies) {
		AN_IkaEnemy1* Ika = Cast<AN_IkaEnemy1>(Enemy);
		if (Ika) {
			
			// 退場演出を開始（関数内で最後にDestroyされる）
			Ika->PlaySpawnTransition(false);
		}
	}

	//フェーズ更新
	int32 PhaseIndex = (int32)CurrentPhase;
	PhaseIndex++;

	//クリア判定 次のフェーズがFinishedに達したら
	if (PhaseIndex >= (int32)EGamePhase::Finished) {
		CurrentPhase = EGamePhase::Finished;
		OnGameFinish(); // クリア専用の関数を呼ぶ
		return;
	}
	CurrentPhase = (EGamePhase)PhaseIndex;

	// --- ここでスポーンタイマーを一旦止める ---
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);

	//フェーズに応じた誘導演出 ---
	float NextPhaseDuration = 30.0f;
	if (CurrentPhase == EGamePhase::Left || CurrentPhase == EGamePhase::FinalFront) {
		NextPhaseDuration = 45.0f;
	}

	//誘導UIを出す
	ShowPhaseDirectionArrow(CurrentPhase);

	//3秒後にスポーンを開始する
	float SpawnInterval = (CurrentPhase == EGamePhase::FinalFront) ? 2.0f : 3.0f;

	FTimerHandle DelaySpawnTimer;
	GetWorldTimerManager().SetTimer(DelaySpawnTimer, [this, SpawnInterval]() {
		//2秒経ってからスポーンループを開始
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AN_AGameMode_Main::SpawnEnemyRandomly, SpawnInterval, true);
		}, 2.0f, false);

	// 次のフェーズチェンジタイマーも、誘導時間の3秒を足して再設定
	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	// 準備（攻撃停止）を呼ぶタイミングを調整
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &AN_AGameMode_Main::PreparePhaseChange, NextPhaseDuration + 3.0f - 5.0f, false);

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Follow the Arrow!"));
}

void AN_AGameMode_Main::SpawnEnemyRandomly() {

	if (!bIsGameActive) return;

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Spown"));

	ActiveEnemies.RemoveAll([](AActor* A) { return !IsValid(A); });

	// --- 1. フェーズごとのルール設定 ---
	int32 MaxStop = 0;
	int32 MaxMove = 0;

	switch (CurrentPhase)
	{
	case EGamePhase::Front:
		MaxStop = 3;
		MaxMove = 0;
		break;
	case EGamePhase::Right:
		MaxStop = 2;
		MaxMove = 2;
		break;
	case EGamePhase::Back:
		MaxStop = 3;
		MaxMove = 3;
		break;
	case EGamePhase::Left:
		MaxStop = 0;
		MaxMove = 7;
		break;
	case EGamePhase::FinalFront:
		MaxStop = 8;
		MaxMove = 0;
		break;
	default:
		break;
	}


	// --- 2. 現在のタイプ別生存数をカウント ---
	int32 CurrentStopCount = 0;
	int32 CurrentMoveCount = 0;
	for (AActor* Enemy : ActiveEnemies) {
		if (Enemy->ActorHasTag("IsMover")) CurrentMoveCount++;
		else CurrentStopCount++;
	}

	// --- 3. どちらのタイプを出すべきか決定（足りない方を優先） ---
	bool bWantMover = false;
	if (CurrentMoveCount < MaxMove) {
		bWantMover = true;
	}
	else if (CurrentStopCount < MaxStop) {
		bWantMover = false;
	}
	else {
		return; // 全ての枠が埋まっていれば終了
	}

	// --- 4. 検索用タグの確定 ---
	FName PhaseTag;
	switch (CurrentPhase) {
	case EGamePhase::Front:      PhaseTag = "Front"; break;
	case EGamePhase::Right:      PhaseTag = "Right"; break;
	case EGamePhase::Back:       PhaseTag = "Back"; break;
	case EGamePhase::Left:       PhaseTag = "Left"; break;
	case EGamePhase::FinalFront: PhaseTag = "Final"; break;
	}
	FName TypeTag = bWantMover ? "Move" : "Stop";

	// --- 5. 条件に合うスプラインを検索 ---
	TArray<AActor*> FoundSplines;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), PhaseTag, FoundSplines);
	// 型タグで絞り込み
	FoundSplines.RemoveAll([TypeTag](AActor* A) { return !A->ActorHasTag(TypeTag); });

	//今回のゲーム全体で一度でも使ったスプラインを除外する
	FoundSplines.RemoveAll([this](AActor* S) {
		return UsedSplinesTotal.Contains(S);
		});

	if (FoundSplines.Num() == 0) return;

	//既に使用されているスプラインを除外する
	TArray<AActor*> UsedSplines;
	for (AActor* Enemy : ActiveEnemies) {
		//敵が持っているスプラインを取得
		AN_IkaEnemy1* Ika = Cast<AN_IkaEnemy1>(Enemy);
		if (Ika && Ika->TargetSpline) {
			UsedSplines.Add(Ika->TargetSpline);
		}
	}

	//候補の中から、既に使用中のものを削除
	FoundSplines.RemoveAll([UsedSplines](AActor* s) {
		return UsedSplines.Contains(s);
		});

	//もし空いてるスプラインがなければ終了
	if (FoundSplines.Num() == 0) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("All Splines are Busy!"));
		return;
	}

	// --- 6. スポーン実行 ---
	AActor* TargetSplineActor = FoundSplines[FMath::RandRange(0, FoundSplines.Num() - 1)];
	USplineComponent* SplineComp = TargetSplineActor->FindComponentByClass<USplineComponent>();

	if (SplineComp) {
		float RandomDist = FMath::FRandRange(0.f, SplineComp->GetSplineLength());
		FVector SpawnLocation = SplineComp->GetLocationAtDistanceAlongSpline(RandomDist, ESplineCoordinateSpace::World);

		//プレイヤーの位置取得
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
		FRotator SpawnRotation;

		if (PlayerPawn) {
			//敵がプレイヤーの方向を向く回転を計算
			SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, PlayerPawn->GetActorLocation());
		}
		else {
			SpawnRotation = (FVector::ZeroVector - SpawnLocation).Rotation();
		}

		// 敵をスポーン
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* NewEnemy = GetWorld()->SpawnActor<AActor>(EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);

		if (NewEnemy) {
			UsedSplinesTotal.Add(TargetSplineActor); // 二度と使われないようにリストへ
			ActiveEnemies.Add(NewEnemy);
			AN_IkaEnemy1* Ika = Cast<AN_IkaEnemy1>(NewEnemy);

			//Ika自体が正しく生成されたか確認
			if (Ika) {
				Ika->Tags.Add(bWantMover ? FName("IsMover") : FName("Stop"));
				//スポーンした敵にどのスプラインを使っているか覚えさせる
				Ika->TargetSpline = Cast<AA_EnemySpline>(TargetSplineActor);
				Ika->SetIsMove(true);

				// --- フェーズごとに敵のスピードを直接設定 ---
				switch (CurrentPhase)
				{
				case EGamePhase::Front:
					Ika->SetMoveSpeed(50.f);  // Frontはゆっくり
					break;
				case EGamePhase::Right:
					Ika->SetMoveSpeed(70.f);  // 指定の 70
					break;
				case EGamePhase::Back:
					Ika->SetMoveSpeed(100.f); // 指定の 100
					//MaxConcurrentAttacks = 3; //同時に攻撃してくる敵の数を3
					break;
				case EGamePhase::Left:
					Ika->SetMoveSpeed(150.f); // 指定の 150
					MaxConcurrentAttacks = 3;
					break;
				case EGamePhase::FinalFront:
					Ika->SetMoveSpeed(200.f); // Finalはさらに速く
					Ika->SetFireInterval(2.0f); //最後だけ敵の攻撃間隔を半分にする
					//MaxConcurrentAttacks = 3;
					break;
				default:
					Ika->SetMoveSpeed(70.f);
					Ika->SetFireInterval(3.0f);
					MaxConcurrentAttacks = 2;
					break;
				}

				if (Ika->TargetSpline) {
					Ika->SplineComponent = Ika->TargetSpline->FindComponentByClass<USplineComponent>();

					//SplineComponent がちゃんと取得できたか確認
					if (Ika->SplineComponent) {
						float ClosestKey = Ika->SplineComponent->FindInputKeyClosestToWorldLocation(SpawnLocation);
						Ika->CurrentSplineDistance = Ika->SplineComponent->GetDistanceAlongSplineAtSplineInputKey(ClosestKey);
					}
				}

				Ika->PlaySpawnTransition(true);
			}
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("EnemySpown"));
		}
	}
}

void AN_AGameMode_Main::ShowPhaseDirectionArrow(EGamePhase NextPhase)
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;

	// 全ての矢印を一旦隠す
	TArray<UWidgetComponent*> Widgets;
	PlayerPawn->GetComponents<UWidgetComponent>(Widgets);
	for (UWidgetComponent* W : Widgets) {
		if (W->GetName().StartsWith(TEXT("Arrow_"))) {
			W->SetVisibility(false);
		}
	}

	// 次のフェーズに対応する矢印を表示
	FName TargetArrowName = TEXT("Arrow_F"); // デフォルト
	switch (NextPhase) {
	case EGamePhase::Right:      TargetArrowName = TEXT("Arrow_F"); break;
	case EGamePhase::Back:       TargetArrowName = TEXT("Arrow_R"); break;
	case EGamePhase::Left:       TargetArrowName = TEXT("Arrow_B"); break;
	case EGamePhase::FinalFront: TargetArrowName = TEXT("Arrow_L"); break;
	default: return;
	}

	// 名前で探して表示
	for (UWidgetComponent* W : Widgets) {
		if (W->GetName() == TargetArrowName) {
			W->SetVisibility(true);

			// 3秒後に隠すタイマー
			FTimerHandle HideTimer;
			GetWorldTimerManager().SetTimer(HideTimer, [W]() {
				if (W) W->SetVisibility(false);
				}, 3.0f, false);
			break;
		}
	}
}

void AN_AGameMode_Main::AddEnemyKillCount()
{
	iEnemyKillCount++;

	//生存している敵をリストから整理
	ActiveEnemies.RemoveAll([](AActor* A) { return !IsValid(A); });

	//即座に次フェーズへ行く判定

	//現在のフェーズタグで「まだ使っていないスプライン」があるか確認
	FName PhaseTag;
	switch (CurrentPhase) {
	case EGamePhase::Front:      PhaseTag = "Front"; break;
	case EGamePhase::Right:      PhaseTag = "Right"; break;
	case EGamePhase::Back:       PhaseTag = "Back"; break;
	case EGamePhase::Left:       PhaseTag = "Left"; break;
	case EGamePhase::FinalFront: PhaseTag = "Final"; break;
	}

	TArray<AActor*> AvailableSplines;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), PhaseTag, AvailableSplines);
	AvailableSplines.RemoveAll([this](AActor* S) { return UsedSplinesTotal.Contains(S); });

	//「もうスポーン待ちのスプラインがない」かつ「画面に敵がいない」なら次へ
	if (AvailableSplines.Num() == 0 && ActiveEnemies.Num() == 0) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Phase Clear! Skipping Timer..."));

		// タイマーをキャンセルして即座にフェーズ移行準備へ
		GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
		PreparePhaseChange();
	}

	//if (iEnemyKillCount >= iBossSpawnNumber)
	//{
	//	//ゲーム終了
	//	bIsGameActive = false;
	//
	//	TArray<AActor*> FoundProjectiles;
	//
	//	//画面内のすべての弾を消滅させる
	//	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AN_EnemyProjectile::StaticClass(), FoundProjectiles);
	//	for (AActor* ProjectileActor : FoundProjectiles)
	//	{
	//		if (ProjectileActor)
	//		{
	//			ProjectileActor->Destroy();
	//		}
	//	}
	//
	//	// プレイヤー(Pawn)を取得
	//	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	//	if (PlayerPawn)
	//	{
	//		// Pawnにくっついている全Widgetコンポーネントを取得
	//		TArray<UWidgetComponent*> WidgetComps;
	//		PlayerPawn->GetComponents<UWidgetComponent>(WidgetComps);
	//
	//		bool bFound = false;
	//		for (UWidgetComponent* Comp : WidgetComps)
	//		{
	//			// あなたがPawnで付けた名前「ClaerWidget」と一致するか確認
	//			// ※「Clear」ではなく「Claer」になっているなら、そのまま「ClaerWidget」と書きます
	//			if (Comp && Comp->GetName() == TEXT("ClaerWidget"))
	//			{
	//				Comp->SetVisibility(true);
	//				Comp->SetHiddenInGame(false);
	//				bFound = true;
	//				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("ClaerWidget Found and Set Visible!"));
	//				break;
	//			}
	//		}
	//
	//		if (!bFound)
	//		{
	//			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ClaerWidget NOT FOUND by Name!"));
	//		}
	//	}
	//
	//	//ここでタイトルに戻る、かボス戦に行くか
	//	ReturnToTitleWithDelay();
	//}
}


bool AN_AGameMode_Main::RequestAttackToken()
{
	// ゲームがアクティブでないなら許可しない
	if (!bIsGameActive) return false;

	// 現在の攻撃数が最大数未満なら許可
	if (CurrentAttackCount < MaxConcurrentAttacks)
	{
		CurrentAttackCount++;
		return true;
	}

	return false;
}

void AN_AGameMode_Main::ReleaseAttackToken()
{
	// 数を減らす（0未満にならないようにガード）
	CurrentAttackCount = FMath::Max(0, CurrentAttackCount - 1);
}

void AN_AGameMode_Main::OnGameFinish()
{
	if (!bIsGameActive) return; 
	bIsGameActive = false;

	// タイマー全停止
	GetWorldTimerManager().ClearAllTimersForObject(this);

	// --- 1. 撃破数をGameInstanceに保存 ---
	// UMyGameInstance はご自身のクラス名に合わせてください。
	// もしBPで作っている場合は、Cast<UGameInstance> して BP側でセットしてもOKです。
	UN_MyGameInstance* GI = Cast<UN_MyGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GI) {
		GI->FinalKillCount = iEnemyKillCount;
	}

	// --- 2. プレイヤーの FinishWidget を表示 ---
	AN_VRPawn* Player = Cast<AN_VRPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (Player) {
		TArray<UWidgetComponent*> Widgets;
		Player->GetComponents<UWidgetComponent>(Widgets);

		for (UWidgetComponent* W : Widgets) {
			// "FinishWidget" という名前が含まれるコンポーネントを表示
			if (W && W->GetName().Contains(TEXT("FinishWidget"))) {
				W->SetVisibility(true);
			}
		}
	}

	// --- 3. 3秒後にリザルトレベルへ移動 ---
	FTimerHandle ResultTimer;
	GetWorldTimerManager().SetTimer(ResultTimer, [this]() {
		UGameplayStatics::OpenLevel(GetWorld(), FName("ResultLevel"));
		}, 3.0f, false);
}
