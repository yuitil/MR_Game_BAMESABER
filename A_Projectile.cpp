//2025/12/19	銃弾　作成
//A_Projectile.cpp

#include "A_Projectile.h"

//コンストラクタ
AA_Projectile::AA_Projectile()
{
 	//更新するか否か
	PrimaryActorTick.bCanEverTick = true;

	//コリジョンの初期化
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	//コリジョンの半径
	CollisionComponent->InitSphereRadius(15.0f);
	//コリジョンの名前
	CollisionComponent->BodyInstance.SetCollisionProfileName("Projectile");
	//コリジョンの
	CollisionComponent->OnComponentHit.AddDynamic(this, &AA_Projectile::OnHit);
	//CollisionComponentをRootComponentに代入
	RootComponent = CollisionComponent;

	//弾の速度コンポーネント生成
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>
		(TEXT("ProjectileMovementComponent"));
	//弾の速度更新
	ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);

	//弾の初速
	ProjectileMovementComponent->InitialSpeed = 20.0f;
	//弾の最大速度
	ProjectileMovementComponent->MaxSpeed = 2000.0f;
	//移動方向に併せて弾の回転を更新するためのフラグ
	ProjectileMovementComponent->bRotationFollowsVelocity = true;

	//弾の重力
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;

	//寿命
	InitialLifeSpan = 3.0f;

	//メッシュコンポーネントの作成
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	//コリジョンの子にする
	ProjectileMesh->SetupAttachment(RootComponent);
	//メッシュ側の当たり判定は無効にする（CollisionComponentが担当するため）
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

//初期化
void AA_Projectile::BeginPlay()
{
	Super::BeginPlay();
	
	// 生まれた時の場所を記録
	StartLocation = GetActorLocation();
}

//更新
void AA_Projectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 発射地点からの現在距離を計算
	float DistanceTraveled = FVector::Dist(StartLocation, GetActorLocation());

	// 設定した最大距離を超えたら消滅
	if (DistanceTraveled > MaxDistance)
	{
		Destroy();
	}
}

//弾の当たり判定処理
void AA_Projectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
	// 自分自身や発射したPawnに当たって消えないよう注意
	if (OtherActor && (OtherActor != this) && (OtherActor != GetOwner())) {
		Destroy();
	}
}