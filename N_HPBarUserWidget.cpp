//N_HPBarUserWidget.cpp

#include "N_HPBarUserWidget.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

void UN_HPBarUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (HP_Bar_Image)
	{
		DynamicMaterial = HP_Bar_Image->GetDynamicMaterial();
	}
}

void UN_HPBarUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (DynamicMaterial)
	{
		//滑らかに数値を近づける計算
		CurrentRatio = FMath::FInterpTo(CurrentRatio, TargetRatio, InDeltaTime, InterpSpeed);

		//マテリアルのパラメータを更新
		DynamicMaterial->SetScalarParameterValue(FName("HP_Ratio"), CurrentRatio);
	}
}

void UN_HPBarUserWidget::UpdateHP(float NewRatio)
{
	TargetRatio = FMath::Clamp(NewRatio, 0.0f, 1.0f);
}