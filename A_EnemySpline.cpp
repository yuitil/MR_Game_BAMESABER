//A_EnemySpline.cpp

#include "A_EnemySpline.h"

AA_EnemySpline::AA_EnemySpline()
{
	PrimaryActorTick.bCanEverTick = true;

	//スプラインコンポーネント作成
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	RootComponent = SplineComponent;
}

void AA_EnemySpline::BeginPlay()
{
	Super::BeginPlay();

	//StartLocation = GetActorLocation();
	//GetWorldTimerManager().SetTimer(MoveTimerHandle, this, &AA_EnemySpline::ToggleMoveDirection, MoveInterval, true);

	if (SplineComponent) {
		SplineLength = SplineComponent->GetSplineLength();
	}
}

//void AA_EnemySpline::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//	if (!SplineComponent) return;
//
//	//移動距離を増加
//	CurrentDistance += MoveSpeed * DeltaTime;
//
//	//スプライン長を超えたか判定
//	if (CurrentDistance > SplineLength) {
//		ToggleMoveDirection();
//		//必要に応じて距離リセット
//		//CurrentDistance = SplineLength;
//	}
//	else {
//		//現在距離に沿った位置を取得しActor移動
//		FVector NewLocation = SplineComponent->GetLocationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
//		SetActorLocation(NewLocation);
//
//	}
//
//}

void AA_EnemySpline::ToggleMoveDirection() {
	//Direction *= -1;
	//FVector NewLocation = StartLocation + FVector(MoveDistance * Direction, 0.f, 0.f);
	//SetActorLocation(NewLocation);
}