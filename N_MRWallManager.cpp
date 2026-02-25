//N_MRWallManager.cpp

#include "N_MRWallManager.h"
#include "Kismet/GameplayStatics.h"

AN_MRWallManager::AN_MRWallManager()
{
	PrimaryActorTick.bCanEverTick = false;

}

void AN_MRWallManager::BeginPlay()
{
	Super::BeginPlay();
	
    // シーンキャプチャを起動（OS標準機能をコマンドで呼び出す）
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController()) {
        PC->ConsoleCommand(TEXT("ovr_SceneCapture"), true);
    }

    // スキャン後、アクターが生成されるのを待つ
    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, this, &AN_MRWallManager::FindWallAndSpawn, 2.0f, false);

}

void AN_MRWallManager::FindWallAndSpawn()
{
    TArray<AActor*> SceneAnchors;
    // 1. まずMeta XRが生成した全アンカーを取得
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("OculusXRSceneAnchor"), SceneAnchors);

    for (AActor* Anchor : SceneAnchors)
    {
        // 2. 関数を使わず、アクターに付いている「タグ」を直接チェック！
        // Meta XR Pluginは自動的に "LABEL_WALL_FACE" というタグを付けてくれます
        if (Anchor->ActorHasTag(FName("LABEL_WALL_FACE")))
        {
            FVector WallLoc = Anchor->GetActorLocation();
            FVector WallForward = Anchor->GetActorForwardVector();
            FRotator WallRot = Anchor->GetActorRotation();

            // 壁の正面方向に10cmオフセット
            FVector SpawnPos = WallLoc + (WallForward * 10.0f);

            if (EnemyClass)
            {
                GetWorld()->SpawnActor<AActor>(EnemyClass, SpawnPos, WallRot);
                UE_LOG(LogTemp, Log, TEXT("Success: Spawned on wall using Tag!"));
                break; // 最初の1枚目の壁で見つかったら終了
            }
        }
    }
}

//void AN_MRWallManager::FindWallAndSpawn()
//{
//    TArray<AActor*> SceneAnchors;
//    // Meta XRが生成した「現実の物体」のアクターをすべて取得
//    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("OculusXRSceneAnchor"), SceneAnchors);
//
//    for (AActor* Anchor : SceneAnchors)
//    {
//        // そのアクターが持っているSceneAnchorコンポーネントを取得
//        UActorComponent* SceneCompRaw = Anchor->GetComponentByClass(UOculusXRSceneAnchorComponent::StaticClass());
//        UOculusXRSceneAnchorComponent* SceneComp = Cast<UOculusXRSceneAnchorComponent>(SceneCompRaw);
//
//        if (SceneComp)
//        {
//            TArray<FString> Labels = SceneComp->GetSemanticClassifications();
//            if (Labels.Contains(TEXT("WALL_FACE"))) {
//                FVector WallLoc = Anchor->GetActorLocation();
//                FVector WallForward = Anchor->GetActorForwardVector();
//                FRotator WallRot = Anchor->GetActorRotation();
//
//                // 10cm 手前にスポーン
//                FVector SpawnPos = WallLoc + (WallForward * 10.0f);
//
//                if (EnemyClass) {
//                    GetWorld()->SpawnActor<AActor>(EnemyClass, SpawnPos, WallRot);
//                    break;
//                }
//            }
//        }
//    }
//}

//void AN_MRWallManager::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//}

