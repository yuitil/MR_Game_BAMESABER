//N_HPBarUserWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "N_HPBarUserWidget.generated.h"

UCLASS()
class MR_1_API UN_HPBarUserWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	//初期化
	virtual void NativeConstruct() override;

	//更新処理
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	//BPのイメージと紐づけ
	UPROPERTY(meta = (BindWidget))
	class UImage* HP_Bar_Image;

	//マテリアル操作用
	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicMaterial;

	//比率計算用の変数
	float CurrentRatio = 1.0f;
	float TargetRatio = 1.0f;

	UPROPERTY(EditAnywhere, Category = "HP Bar")
	float InterpSpeed = 5.f;

public:
	UFUNCTION(BlueprintCallable, Category = "HP Bar")
	void UpdateHP(float NewRatio);
};
