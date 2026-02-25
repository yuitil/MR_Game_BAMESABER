//N_ResultGameMode.cpp

#include "N_ResultGameMode.h"
#include "Kismet/GameplayStatics.h"

void AN_ResultGameMode::TitleBack()
{
	UGameplayStatics::OpenLevel(this, FName("MRTitle"));
}
