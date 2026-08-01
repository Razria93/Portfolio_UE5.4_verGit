#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CDebugOverlayHUD.generated.h"

class ACEnemy;

UCLASS()
class PORTFOLIO_API ACDebugOverlayHUD : public AHUD
{
	GENERATED_BODY()

#if !UE_BUILD_SHIPPING
private:
	// Runtime State
	TWeakObjectPtr<ACEnemy> CachedEnemy;
	float LastEnemyScanTimeSeconds = -1.f;
	int32 LastEnemyScanCount = 0;

	// Enemy Resolution
	ACEnemy* ResolveDisplayEnemy(TArray<FString>& OutSourceLines);
	ACEnemy* ResolveTargetComponentEnemy(TArray<FString>& OutSourceLines) const;
	ACEnemy* ResolveRecentCombatEnemy(const APawn* InViewerPawn, TArray<FString>& OutSourceLines) const;
	ACEnemy* ResolveWorldScanFallbackEnemy(TArray<FString>& OutSourceLines);

	// Enemy Cache
	void RefreshCachedEnemyIfNeeded();
#endif

public:
	// Rendering
	virtual void DrawHUD() override;
};
