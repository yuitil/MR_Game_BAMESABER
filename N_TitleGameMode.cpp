//N_TitleGameMode.cpp

#include "N_TitleGameMode.h"
#include "Kismet/GameplayStatics.h"

void AN_TitleGameMode::StartInGame2()
{
	UGameplayStatics::OpenLevel(this, FName("MRInGame"));
}

