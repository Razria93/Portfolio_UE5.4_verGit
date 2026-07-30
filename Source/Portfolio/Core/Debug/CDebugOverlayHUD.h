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
	TWeakObjectPtr<ACEnemy> CachedEnemy;
	float LastEnemyScanTimeSeconds = -1.f;
	int32 LastEnemyScanCount = 0;

private:
	ACEnemy* ResolveDisplayEnemy(const APawn* InViewerPawn, TArray<FString>& OutSourceLines);
	ACEnemy* ResolveTargetComponentEnemy(TArray<FString>& OutSourceLines) const;
	ACEnemy* ResolveRecentCombatEnemy(const APawn* InViewerPawn, TArray<FString>& OutSourceLines) const;
	ACEnemy* ResolveWorldScanFallbackEnemy(TArray<FString>& OutSourceLines);
	void RefreshCachedEnemyIfNeeded();
#endif

public:
	virtual void DrawHUD() override;
};
