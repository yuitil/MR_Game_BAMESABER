//N_Saber.cpp

#include "N_Saber.h"

AN_Saber::AN_Saber()
{
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SaberMesh"));
	RootComponent = Mesh;
	CurrentState = ESwingState::Idle;
}

void AN_Saber::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DeltaTime > 0.f && Mesh)
	{
		FVector CurrentTipLoc = Mesh->GetSocketLocation(TEXT("Socket_Tip"));

		//‰‰ñŽÀsŽž‚ÌLastLocation‚Ì’µ‚Ëã‚ª‚è–hŽ~
		if (LastTipLocation.IsZero())
		{
			LastTipLocation = CurrentTipLoc;
			return;
		}

		//uŠÔˆÚ“®‹——£
		float FrameDist = FVector::Dist(CurrentTipLoc, LastTipLocation);
		float InstantSpeed = FrameDist / DeltaTime;

		//‰Á‘¬“x‚É‚æ‚é‹}Œƒ‚È•Ï‰»‚ð—}‚¦‚Â‚Â’Ç]
		CurrentSpeed = FMath::FInterpTo(CurrentSpeed, InstantSpeed, DeltaTime, 20.f);

		//ó‘Ô”»’è
		if (CurrentSpeed > HighSpeedThreshold) {
			CurrentState = ESwingState::HighSpeed;
		}
		else if (CurrentSpeed > LowSpeedThreshold) {
			CurrentState = ESwingState::LowSpeed;
		}
		else {
			CurrentState = ESwingState::Idle;
		}

		LastTipLocation = CurrentTipLoc;

		GEngine->AddOnScreenDebugMessage(0, 0.1f, FColor::Yellow, FString::Printf(TEXT("Speed: %f"), CurrentSpeed));
	}
}
