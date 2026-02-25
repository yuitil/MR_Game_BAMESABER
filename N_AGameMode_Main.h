//N_AGameMode_Main.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Components/AudioComponent.h"

#include "N_AGameMode_Main.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8 {
    Front,
    Right,
    Back,
    Left,
    FinalFront,
    Finished
};

UCLASS()
class MR_1_API AN_AGameMode_Main : public AGameModeBase
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay()override;

    //BGMアセット
    UPROPERTY(EditAnywhere, Category = "Audio")
    class USoundBase* BackgroundMusic;

    //BGM再生用コンポーネント
    UPROPERTY()
    UAudioComponent* BGMAudioComp;

public:
    //ゲーム開始関数
    UFUNCTION(BlueprintCallable, Category = "GameFlow")
    void StartInGame();

    //ゲーム終了
    UFUNCTION(BlueprintCallable, Category = "GameFlow")
    void OnGameFinish();


    //敵が死んだ時に呼び出す関数
    UFUNCTION(BlueprintCallable, Category = "GameLogic")
    void AddEnemyKillCount();

    //ゲームが行われているか確認するための関数
    UFUNCTION(BlueprintCallable, Category = "GameLogic")
    bool IsGameActive() const { return bIsGameActive; }

protected:
    //敵を倒したカウントする変数
    UPROPERTY(BlueprintReadOnly, Category = "GameLogic")
    int32 iEnemyKillCount = 0;

    //ボスが出てくる、敵を倒した数を設定（まだゲームクリア）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameLogic")
    int32 iBossSpawnNumber = 3;

    //ゲームが行われている状況なのか判断
    bool bIsGameActive = true;

    //フェーズチェンジ前の準備
    void PreparePhaseChange();

    //クリアUIをセットするよう
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UUserWidget> ClearWidgetClass;

public:
    //同時に攻撃できる敵の数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameLogic|Combat")
    int32 MaxConcurrentAttacks = 2;

    //現在攻撃中の敵の数
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameLogic|Combat")
    int32 CurrentAttackCount = 0;

    //攻撃許可をリクエストする
    bool RequestAttackToken();

    //攻撃が終わったことを報告する
    void ReleaseAttackToken();

protected:
	//現在のゲームフェーズ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameLogic")
    EGamePhase CurrentPhase = EGamePhase::Front;

	//フェーズチェンジのタイマーとスポーンのタイマー
    FTimerHandle PhaseTimerHandle;
    FTimerHandle SpawnTimerHandle;

	//フェーズチェンジの前に呼び出される関数（UI表示など）
    void AdvancePhase();

    //敵をスポーンさせる関数
    void SpawnEnemyRandomly();

    //現在画面にいる敵のリスト
    UPROPERTY()
    TArray<AActor*> ActiveEnemies;

    //スポーンさせる敵のクラス（エディタで設定）
    UPROPERTY(EditAnywhere, Category = "GameLogic")
    TSubclassOf<AActor> EnemyClass;

    UFUNCTION()
    void ShowPhaseDirectionArrow(EGamePhase NextPhase);

    // ゲームを通して使用済みのスプラインをすべて保持
    TArray<AActor*> UsedSplinesTotal; 
    // 現在のフェーズで「あと何体スポーンさせるか」
    int32 EnemiesRemainingInPhase = 0; 
};
