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

	friend class FHUDWidgetLifecycleTest;

public:
	// Construction
	UCCombatHUDWidget(const FObjectInitializer& ObjectInitializer);

protected:
	// Preview Config
	// Samples
	UPROPERTY(EditDefaultsOnly, Category="Combat HUD|Preview")
	bool bShowResourceSamples = true;

	// Font Config
	UPROPERTY(EditDefaultsOnly, Category="Combat HUD|Style")
	TObjectPtr<UFont> LabelFont;

	UPROPERTY(EditDefaultsOnly, Category="Combat HUD|Style")
	TObjectPtr<UFont> NameFont;

	// Item Icon
	UPROPERTY(EditDefaultsOnly, Category="Combat HUD|Style")
	TObjectPtr<UTexture2D> ItemIcon;

	// Action Icons
	UPROPERTY(EditDefaultsOnly, Category="Combat HUD|Style")
	TObjectPtr<UTexture2D> RushIcon;

	UPROPERTY(EditDefaultsOnly, Category="Combat HUD|Style")
	TObjectPtr<UTexture2D> GuardIcon;

	UPROPERTY(EditDefaultsOnly, Category="Combat HUD|Style")
	TObjectPtr<UTexture2D> DodgeIcon;

	UPROPERTY(EditDefaultsOnly, Category="Combat HUD|Style")
	TObjectPtr<UTexture2D> CounterIcon;

	UPROPERTY(EditDefaultsOnly, Category="Combat HUD|Style")
	TObjectPtr<UTexture2D> ExecutionIcon;

	// Action State Icons
	UPROPERTY(EditDefaultsOnly, Category="Combat HUD|Style")
	TObjectPtr<UTexture2D> GuardBreakIcon;

private:
	// Player Widget References
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> PlayerPanel;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> PlayerHealthGrid;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PlayerHealthValue;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> PlayerCells;

	// Target Widget References
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> TargetPanel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TargetName;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TargetHealthValue;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TargetBalanceValue;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> TargetCells;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> BalanceCells;

	// Action Widget References
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> SkillsPanel;

	UPROPERTY(Transient)
	TObjectPtr<UImage> GuardImage;

	UPROPERTY(Transient)
	TObjectPtr<UImage> DodgeImage;

	UPROPERTY(Transient)
	TObjectPtr<UImage> CounterImage;

	UPROPERTY(Transient)
	TObjectPtr<UImage> ExecutionImage;

private:
	// Runtime View Data
	FCombatHUDViewData ViewData;

	// Player Health Presentation Cache
	int32 PlayerHealthColumns = -1;
	float LastPlayerMaximum = -1.f;

protected:
	// Widget Lifecycle
	virtual TSharedRef<SWidget> RebuildWidget() override;

public:
	// Query
	UFUNCTION(BlueprintPure, Category="Combat HUD")
	FCombatHUDViewData GetViewData() const { return ViewData; }

public:
	// View Data Update
	void ApplyViewData(const FCombatHUDViewData& Data);
	void ApplyActionViewData(const FHUDActionViewData& Data);

private:
	// Widget Runtime
	void ResetWidgetRuntime();

private:
	// Widget Construction
	UCanvasPanel* BuildRootLayout();
	void BuildTargetPanel(UCanvasPanel* Root, bool bShowSamples);
	void BuildPlayerPanel(UCanvasPanel* Root, bool bShowSamples);
	void BuildItemSlots();
	void BuildActionPanel(UCanvasPanel* Root);

private:
	// Preview Presentation
	// Samples
	bool ShouldShowResourceSamples() const;
	void ApplyTargetShieldSample(const TArray<TObjectPtr<UImage>>& Cells, UTextBlock* Value);
	void ApplyPlayerResourceSample(int32 ResourceIndex, const TArray<TObjectPtr<UImage>>& Cells, UTextBlock* Value);

private:
	// Presentation Update
	void UpdatePresentation();
	void UpdatePanelVisibility();
	void UpdatePlayerPresentation();
	void UpdateTargetPresentation();
	void UpdateActionPresentation();

private:
	// Resource Presentation
	void RefreshPlayerHealthLayout();
	void UpdateTargetBalancePresentation();

private:
	// Widget Construction Helpers
	UImage* AddImage(UCanvasPanel* Parent, FVector2D Position, FVector2D Size, FLinearColor Color);
	UTextBlock* AddLabel(UCanvasPanel* Parent, const FString& Text, FVector2D Position, FVector2D Size, int32 FontSize);
	void AddSlotFrame(UCanvasPanel* Parent, FVector2D Position, float Size, float Radius);
	UImage* AddActionSlot(FVector2D Position, UTexture2D* Texture);
	void AddGauge(UCanvasPanel* Parent, FVector2D Position, float Width, int32 Rows, int32 UnitColumns,
		TArray<TObjectPtr<UImage>>& Cells, bool bUnimplemented = false, int32 ExplicitColumns = -1, bool bBoss = false);
};
