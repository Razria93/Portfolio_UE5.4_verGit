// Opt-in editor-only export of the real Slate settings widget for video evidence.
// No replacement UI, mock values or gameplay event injection.
#include "CoreMinimal.h"
#include "SPortfolioDebugOverlayEditorWidget.h"
#include "Slate/WidgetRenderer.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ImageUtils.h"
#include "HAL/IConsoleManager.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Framework/Application/SlateApplication.h"

static void ExportPanel(const TArray<FString>& Args)
{
    UE_LOG(LogTemp,Display,TEXT("[PanelCapture] Args=%d Gate=%d"),Args.Num(),FParse::Param(FCommandLine::Get(),TEXT("Structure10ExportPanel")));
    if(!FParse::Param(FCommandLine::Get(),TEXT("Structure10ExportPanel")) || Args.Num()!=1) return;
    TSharedRef<SPortfolioDebugOverlayEditorWidget> Widget=SNew(SPortfolioDebugOverlayEditorWidget);
    int32 CaptureWidth=1080;
    FParse::Value(FCommandLine::Get(),TEXT("Structure10PanelWidth="),CaptureWidth);
    CaptureWidth=FMath::Clamp(CaptureWidth,560,1600);
    FWidgetRenderer Renderer(true);
    UTextureRenderTarget2D* Target=Renderer.DrawWidget(Widget,FVector2D(CaptureWidth,1700));
    if(!Target) {UE_LOG(LogTemp,Error,TEXT("[PanelCapture] Missing target"));return;}
    TArray<FColor> Pixels;
    if (!Target->GameThread_GetRenderTargetResource()->ReadPixels(Pixels) || Pixels.Num() != CaptureWidth * 1700)
    {
        UE_LOG(LogTemp, Error, TEXT("[PanelCapture] Pixel readback failed"));
        return;
    }
    for(FColor& P:Pixels) P.A=255;
    TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(CaptureWidth,1700,Pixels,PNG);
    FString Path=Args[0];Path.TrimQuotesInline();
    const bool Saved=FFileHelper::SaveArrayToFile(PNG,*Path);
    UE_LOG(LogTemp,Display,TEXT("[PanelCapture] %s Saved=%d Pixels=%d PNG=%lld"),*Path,Saved,Pixels.Num(),PNG.Num());
}
static FAutoConsoleCommand ExportCommand(TEXT("Portfolio.Capture10.ExportPanel"),TEXT("Opt-in real Slate panel capture."),FConsoleCommandWithArgsDelegate::CreateStatic(&ExportPanel));


#include "Widgets/Input/SCheckBox.h"
#include "InputCoreTypes.h"
static TSharedPtr<SWidget> FindTaggedWidget(TSharedRef<SWidget> Root,FName Tag)
{
    if(Root->GetTag()==Tag)return Root;
    FChildren* Children=Root->GetChildren();
    for(int32 I=0;I<Children->Num();++I){auto Found=FindTaggedWidget(Children->GetChildAt(I),Tag);if(Found.IsValid())return Found;}
    return nullptr;
}
static void TogglePanelEventLog()
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("Structure10ExportPanel")))return;
    TSharedRef<SPortfolioDebugOverlayEditorWidget> Widget=SNew(SPortfolioDebugOverlayEditorWidget);
    auto Found=FindTaggedWidget(Widget,FName(TEXT("Portfolio.DebugOverlay.EventLog.Visible")));
    if(!Found.IsValid()){UE_LOG(LogTemp,Error,TEXT("[PanelCapture] Checkbox missing"));return;}
    auto Check=StaticCastSharedPtr<SCheckBox>(Found);
    const bool Before=Check->IsChecked();

    Check->ToggleCheckedState();
    UE_LOG(LogTemp,Display,TEXT("[PanelCapture] REAL CHECKBOX CHANGE HANDLER EventLog.Visible %d -> %d"),Before,Check->IsChecked());
}
static FAutoConsoleCommand TogglePanelCommand(TEXT("Portfolio.Capture10.ToggleEventLog"),TEXT("Opt-in actual Slate checkbox keyboard interaction."),FConsoleCommandDelegate::CreateStatic(&TogglePanelEventLog));
