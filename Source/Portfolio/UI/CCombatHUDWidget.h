#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Type/CCombatHUDTypes.h"
#include "CCombatHUDWidget.generated.h"

class UCanvasPanel;
class UImage;
class UTextBlock;
class UTexture2D;
class UFont;

UCLASS(Blueprintable)
class PORTFOLIO_API UCCombatHUDWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UCCombatHUDWidget(const FObjectInitializer& ObjectInitializer);
	void ApplyViewData(const FCombatHUDViewData& Data);
	UFUNCTION(BlueprintPure, Category="Combat HUD")
	FCombatHUDViewData GetViewData() const { return ViewData; }
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// Editor-only visual samples. Never used as gameplay resource state.
	UPROPERTY(EditDefaultsOnly, Category="Combat HUD|Preview")
	bool bShowResourceSamples = true;
	UPROPERTY(EditDefaultsOnly, Category="Combat HUD|Style")
	TObjectPtr<UFont> LabelFont;
	UPROPERTY(EditDefaultsOnly, Category="Combat HUD|Style")
	TObjectPtr<UFont> NameFont;
	UPROPERTY(EditDefaultsOnly, Category="Combat HUD|Style")
	TArray<TObjectPtr<UTexture2D>> SkillIcons;
	UPROPERTY(EditDefaultsOnly, Category="Combat HUD|Style")
	TObjectPtr<UTexture2D> ItemIcon;
	UPROPERTY(EditDefaultsOnly, Category="Combat HUD|Style")
	TObjectPtr<UTexture2D> RushIcon;
	// Reserved alternate image; not driven by gameplay until action state is wired.
	UPROPERTY(EditDefaultsOnly, Category="Combat HUD|Style")
	TObjectPtr<UTexture2D> GuardBreakIcon;

private:
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> TargetPanel;
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> PlayerPanel;
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> PlayerHealthGrid;
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> SkillsPanel;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TargetName;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PlayerHealthValue;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TargetHealthValue;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TargetBalanceValue;
	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> PlayerCells;
	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> TargetCells;
	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> BalanceCells;
	FCombatHUDViewData ViewData;
	int32 PlayerHealthColumns = -1;
	float LastPlayerMaximum = -1.f;

	UTextBlock* AddLabel(UCanvasPanel* Parent, const FString& Text, FVector2D Position, FVector2D Size, int32 FontSize);
	UImage* AddImage(UCanvasPanel* Parent, FVector2D Position, FVector2D Size, FLinearColor Color);
	void AddSlotFrame(UCanvasPanel* Parent, FVector2D Position, float Size, float Radius);
	void AddGauge(UCanvasPanel* Parent, FVector2D Position, float Width, int32 Rows, int32 UnitColumns,
		TArray<TObjectPtr<UImage>>& Cells, bool bUnimplemented = false, int32 ExplicitColumns = -1, bool bBoss = false);
	void UpdatePresentation();
};
