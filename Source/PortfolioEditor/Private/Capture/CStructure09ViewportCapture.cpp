// Editor-only, authentic PIE viewport capture. Does not alter gameplay time or camera.
#include "CoreMinimal.h"
#include "Async/Async.h"
#include "AudioMixerBlueprintLibrary.h"
#include "AudioDevice.h"
#include "AudioDeviceManager.h"
#include "Containers/Ticker.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "FrameGrabber.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformProcess.h"
#include "HAL/ThreadSafeCounter.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/DateTime.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/Parse.h"
#include "Misc/ScopeLock.h"
#include "Modules/ModuleManager.h"
#include "RenderingThread.h"
#include "Rendering/SlateRenderer.h"
#include "Widgets/SViewport.h"
#include "Widgets/SWindow.h"
#include "Sound/SoundSubmix.h"
#include "CStructure09AsyncReadback.h"
#include "CRecordingLifecycle.h"
#include "Misc/Guid.h"

DEFINE_LOG_CATEGORY_STATIC(LogCapture09Viewport, Log, All);

namespace Capture09Viewport
{
struct FPayload final : IFramePayload
{
    int32 Index = 0;
    double RequestedWall = 0;
    double WorldSeconds = 0;
    mutable double ReadyWall = 0;
    mutable double CapturedWall = 0;
    virtual bool OnFrameReady_RenderThread(FColor*, FIntPoint, FIntPoint) const override
    {
        ReadyWall = FPlatformTime::Seconds();
        return true;
    }
};

struct FRecord
{
    int32 Index = 0;
    double Requested = 0;
    double Ready = 0;
    double World = 0;
    FString Status;
    double Captured = 0;
};

struct FSharedWriter
{
    FString Directory;
    double Epoch = 0;
    FThreadSafeCounter Pending;
    FCriticalSection Mutex;
    TArray<FRecord> Records;
    TArray<FRecord> Requests;
    IImageWrapperModule* Images = nullptr;
};

static TUniquePtr<FStructure09AsyncReadback> GPUGrabber;
static TSharedPtr<FSceneViewport> OffscreenViewport;
static TArray<FCapturedFrameData> DirectFrames;
static TSharedPtr<FSharedWriter, ESPMode::ThreadSafe> Writer;
static TWeakObjectPtr<UWorld> World;
static FTSTicker::FDelegateHandle TickHandle;
static bool Recording = false;
static int32 NextIndex = 0;
static int32 ReturnedFrames = 0;
static FIntPoint RecordingSize;
static FString StopReason;
static PortfolioRecording::FCVarOverride AppVolumeOverride;
static double LastRequest = 0;
static FDelegateHandle EndPIEHandle;
static FAudioDeviceHandle CaptureAudioDevice;
static TWeakObjectPtr<USoundSubmix> RecordingSubmix;
static bool OldSubmixAutoDisable = true;
static bool ChangedAudioRouting = false;
static bool OldDeviceMuted = false;
static TAutoConsoleVariable<int32> MaxFPS(TEXT("Portfolio.Capture09.RecordFPS"), 60, TEXT("Maximum real-time viewport capture requests/second; never changes simulation speed."));
static TAutoConsoleVariable<int32> RecorderBusy(TEXT("Portfolio.Capture09.RecorderBusy"), 0, TEXT("1 while recording or waiting for JPEG/WAV export; 0 after record_done.txt is written."));

static FString ValidateRecordedAudio(const FString& Directory, double ExpectedDuration)
{
    TArray<uint8> Bytes;
    if (!FFileHelper::LoadFileToArray(Bytes, *(Directory / TEXT("audio.wav"))) || Bytes.Num() < 44)
        return TEXT("invalid_wav");
    auto U16 = [&Bytes](int32 At) { uint16 V = 0; FMemory::Memcpy(&V, Bytes.GetData() + At, 2); return V; };
    auto U32 = [&Bytes](int32 At) { uint32 V = 0; FMemory::Memcpy(&V, Bytes.GetData() + At, 4); return V; };
    int32 Channels = 0, SampleRate = 0, Bits = 0, Format = 0, DataOffset = 0, DataBytes = 0;
    if (FMemory::Memcmp(Bytes.GetData(), "RIFF", 4) || FMemory::Memcmp(Bytes.GetData() + 8, "WAVE", 4)) return TEXT("invalid_wav");
    for (int32 At = 12; At + 8 <= Bytes.Num();)
    {
        const uint32 ChunkSize = U32(At + 4);
        if (int64(At) + 8 + ChunkSize > Bytes.Num()) break;
        if (!FMemory::Memcmp(Bytes.GetData() + At, "fmt ", 4) && ChunkSize >= 16)
        {
            Format = U16(At + 8); Channels = U16(At + 10); SampleRate = U32(At + 12); Bits = U16(At + 22);
        }
        if (!FMemory::Memcmp(Bytes.GetData() + At, "data", 4)) { DataOffset = At + 8; DataBytes = ChunkSize; }
        At += 8 + ChunkSize + (ChunkSize & 1);
    }
    if (Format != 1 || Bits != 16 || Channels <= 0 || SampleRate <= 0 || DataBytes <= 0) return TEXT("unsupported_or_empty_wav");
    const double Duration = double(DataBytes) / (2.0 * Channels * SampleRate);
    int32 Peak = 0, Nonzero = 0;
    for (int32 At = DataOffset; At + 1 < DataOffset + DataBytes; At += 2)
    {
        int16 Value = 0;
        FMemory::Memcpy(&Value, Bytes.GetData() + At, 2);
        Peak = FMath::Max(Peak, FMath::Abs(int32(Value)));
        Nonzero += Value != 0 ? 1 : 0;
    }
    FString Status = FMath::Abs(Duration - ExpectedDuration) > 0.25 ? TEXT("duration_mismatch") : (Nonzero == 0 ? TEXT("silent_pcm") : TEXT("valid_pcm_nonzero"));
    FString Report = FString::Printf(TEXT("status=%s\nexpected_seconds=%.9f\nrecorded_seconds=%.9f\nsample_rate=%d\nchannels=%d\npeak_pcm16=%d\nnonzero_samples=%d\nListening and event alignment review still required.\n"), *Status, ExpectedDuration, Duration, SampleRate, Channels, Peak, Nonzero);
    FFileHelper::SaveStringToFile(Report, *(Directory / TEXT("audio_qa.txt")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    return Status;
}

static void QueueFrames()
{
    if (!Writer) return;
    TArray<FCapturedFrameData> ReadyFrames = GPUGrabber ? GPUGrabber->GetCapturedFrames() : MoveTemp(DirectFrames);
    DirectFrames.Reset();
    for (FCapturedFrameData& Frame : ReadyFrames)
    {
        ++ReturnedFrames;
        const FPayload* P = Frame.GetPayload<FPayload>();
        if (!P) continue;
        FRecord Row{P->Index, P->RequestedWall - Writer->Epoch, P->ReadyWall - Writer->Epoch, P->WorldSeconds, TEXT("ok")};
        Row.Captured = P->CapturedWall > 0 ? P->CapturedWall - Writer->Epoch
            : (GPUGrabber ? -1 : P->RequestedWall - Writer->Epoch);
        // Pending pixels can straddle the layout change. Keep already written
        // frames, but never certify this uncertain tail as correctly cropped.
        if (StopReason == TEXT("viewport_geometry_changed"))
        {
            Row.Status = TEXT("dropped_geometry_change");
            FScopeLock Lock(&Writer->Mutex);
            Writer->Records.Add(MoveTemp(Row));
            continue;
        }
        if (Frame.ColorBuffer.IsEmpty())
        {
            Row.Captured = -1;
            Row.Status = TEXT("dropped_gpu_readback_or_unsubmitted_at_stop");
            FScopeLock Lock(&Writer->Mutex);
            Writer->Records.Add(MoveTemp(Row));
            continue;
        }
        if (Writer->Pending.GetValue() >= 12)
        {
            Row.Status = TEXT("dropped_writer_backpressure");
            FScopeLock Lock(&Writer->Mutex);
            Writer->Records.Add(MoveTemp(Row));
            continue;
        }
        Writer->Pending.Increment();
        auto Shared = Writer;
        const FIntPoint Size = Frame.BufferSize;
        TArray<FColor> Pixels = MoveTemp(Frame.ColorBuffer);
        Async(EAsyncExecution::ThreadPool, [Shared, Row = MoveTemp(Row), Size, Pixels = MoveTemp(Pixels)]() mutable
        {
            TSharedPtr<IImageWrapper> JPEG = Shared->Images->CreateImageWrapper(EImageFormat::JPEG);
            bool Saved = false;
            if (JPEG.IsValid() && JPEG->SetRaw(Pixels.GetData(), int64(Pixels.Num()) * sizeof(FColor), Size.X, Size.Y, ERGBFormat::BGRA, 8))
            {
                TArray64<uint8> Encoded = JPEG->GetCompressed(95);
                Saved = FFileHelper::SaveArrayToFile(Encoded, *(Shared->Directory / FString::Printf(TEXT("frame_%06d.jpg"), Row.Index)));
            }
            if (!Saved) Row.Status = TEXT("encode_or_write_failed");
            {
                FScopeLock Lock(&Shared->Mutex);
                Shared->Records.Add(MoveTemp(Row));
            }
            Shared->Pending.Decrement();
        });
    }
}

static void Stop()
{
    if (!Writer) return;
    if (StopReason == TEXT("requested_stop") && GPUGrabber && GPUGrabber->HasGeometryChanged())
        StopReason = TEXT("viewport_geometry_changed");
    Recording = false;
    if (TickHandle.IsValid()) FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
    TickHandle.Reset();
    if (EndPIEHandle.IsValid()) FEditorDelegates::EndPIE.Remove(EndPIEHandle);
    EndPIEHandle.Reset();
    const double AudioStopWall = FPlatformTime::Seconds();
    if (World.IsValid())
    {
        UAudioMixerBlueprintLibrary::StopRecordingOutput(World.Get(), EAudioRecordingExportType::WavFile, TEXT("audio"), Writer->Directory, RecordingSubmix.Get());
    }
    if (CaptureAudioDevice.IsValid() && RecordingSubmix.IsValid())
        CaptureAudioDevice->SetSubmixAutoDisable(RecordingSubmix.Get(), OldSubmixAutoDisable);
    if (ChangedAudioRouting && CaptureAudioDevice.IsValid())
        CaptureAudioDevice->SetDeviceMuted(OldDeviceMuted);
    AppVolumeOverride.Restore();
    CaptureAudioDevice.Reset();
    RecordingSubmix.Reset();
    ChangedAudioRouting = false;
    if (GPUGrabber)
    {
        GPUGrabber->StopCapturingFrames();
        GPUGrabber->Shutdown();
        QueueFrames();
        GPUGrabber.Reset();
    }
    QueueFrames();
    OffscreenViewport.Reset();
    const FString FinishedReason = StopReason;
    const auto FinishedWriter = Writer;
    const int32 RequestedCount = NextIndex;
    const double ExpectedAudioDuration = AudioStopWall - Writer->Epoch;
    PortfolioRecording::FExportProgress Progress;
    Progress.StartedAt = FPlatformTime::Seconds();
    // Workers own their writer until completion, including after a timeout.
    // A timeout reports an incomplete export; it never certifies late files.
    FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
        [FinishedWriter, FinishedReason, RequestedCount, ExpectedAudioDuration, Progress](float) mutable
        {
            const double Now = FPlatformTime::Seconds();
            const int32 Pending = FinishedWriter->Pending.GetValue();
            const FString& Directory = FinishedWriter->Directory;
            const int64 AudioSize = IFileManager::Get().FileSize(*(Directory / TEXT("audio.wav")));
            if (!Progress.Poll(Now, Pending, AudioSize)) return true;
            TArray<FRecord> Records;
            {
                FScopeLock Lock(&FinishedWriter->Mutex);
                Records = FinishedWriter->Records;
            }
            const int32 ReturnedCount = Records.Num();
            TSet<int32> ReturnedIndices;
            for (const FRecord& Row : Records) ReturnedIndices.Add(Row.Index);
            // EndPIE can discard a legacy request before Slate presents it.
            // Preserve its real request time and explicitly mark unavailable
            // capture/ready times, rather than silently omitting the CSV row.
            for (const FRecord& Request : FinishedWriter->Requests)
            {
                if (ReturnedIndices.Contains(Request.Index)) continue;
                FRecord Missing = Request;
                Missing.Status = Pending > 0 ? TEXT("incomplete_at_export_timeout") : TEXT("dropped_unreturned_at_stop");
                Records.Add(MoveTemp(Missing));
            }
            Records.Sort([](const FRecord& A, const FRecord& B){ return A.Index < B.Index; });
            FString CSV(TEXT("index,request_wall_seconds,readback_ready_wall_seconds,world_seconds,status,file,capture_wall_seconds\n"));
            int32 SavedCount = 0;
            for (const FRecord& Row : Records)
            {
                CSV += FString::Printf(TEXT("%d,%.9f,%.9f,%.9f,%s,frame_%06d.jpg,%.9f\n"), Row.Index, Row.Requested, Row.Ready, Row.World, *Row.Status, Row.Index, Row.Captured);
                SavedCount += Row.Status == TEXT("ok") ? 1 : 0;
            }
            const FString FrameStatus = Pending > 0 ? TEXT("incomplete_writer_timeout") :
                (SavedCount == RequestedCount ? TEXT("complete") : TEXT("complete_with_drops_or_errors"));
            const FString AudioStatus = Progress.HasStableAudio() ? ValidateRecordedAudio(Directory, ExpectedAudioDuration) : TEXT("unverified_export_timeout");
            const bool CSVWritten = FFileHelper::SaveStringToFile(CSV, *(Directory / TEXT("frames.csv")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
            const FString Ending = FString::Printf(TEXT("stop_reason=%s\nrequested=%d\nreturned=%d\nsaved=%d\npending_at_finalize=%d\naudio_stop_wall_seconds=%.9f\nframes=%s\naudio=%s\nUnavailable capture/ready timestamps are -1. Late worker files after a timeout are not verified by this manifest.\n"), *FinishedReason, RequestedCount, ReturnedCount, SavedCount, Pending, ExpectedAudioDuration, *FrameStatus, *AudioStatus);
            const bool SummaryWritten = FFileHelper::SaveStringToFile(Ending, *(Directory / TEXT("complete.txt")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
            const FString Status = FString::Printf(TEXT("stop_reason=%s\nframes=%s\naudio=%s\nmanifest=%s\n"), *FinishedReason, *FrameStatus, *AudioStatus, CSVWritten && SummaryWritten ? TEXT("written") : TEXT("write_failed"));
            const bool DoneWritten = FFileHelper::SaveStringToFile(Status, *(Directory / TEXT("record_done.txt")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
            RecorderBusy.AsVariable()->Set(0, ECVF_SetByCode);
            UE_LOG(LogCapture09Viewport, Display, TEXT("[Capture09Record] WRITE_COMPLETE %s frames=%s audio=%s marker=%d"), *Directory, *FrameStatus, *AudioStatus, DoneWritten ? 1 : 0);
            return false;
        }), 0.25f);
    UE_LOG(LogCapture09Viewport, Display, TEXT("[Capture09Record] STOP_QUEUED %s requested=%d pending=%d"), *Writer->Directory, NextIndex, Writer->Pending.GetValue());
    Writer.Reset();
    World.Reset();
}

static bool Tick(float)
{
    if (!Recording || !World.IsValid())
    {
        StopReason = TEXT("world_unavailable");
        Stop();
        return false;
    }
    if ((GPUGrabber && GPUGrabber->HasGeometryChanged()) ||
        (OffscreenViewport && OffscreenViewport->GetSizeXY() != RecordingSize))
    {
        StopReason = TEXT("viewport_geometry_changed");
        UE_LOG(LogCapture09Viewport, Warning, TEXT("[Capture09Record] Viewport geometry changed; ending this fixed-size take."));
        Stop();
        return false;
    }
    QueueFrames();
    const double Now = FPlatformTime::Seconds();
    const double Interval = 1.0 / FMath::Clamp(MaxFPS.GetValueOnGameThread(), 1, 120);
    if (Now + 0.0001 >= LastRequest)
    {
        // Avoid accumulation if Slate stops presenting while the editor is minimized.
        if (NextIndex - ReturnedFrames < 8)
        {
            auto Payload = MakeShared<FPayload, ESPMode::ThreadSafe>();
            Payload->Index = NextIndex++;
            Payload->RequestedWall = Now;
            Payload->WorldSeconds = World->GetTimeSeconds();
            Writer->Requests.Add(FRecord{Payload->Index, Now - Writer->Epoch, -1, Payload->WorldSeconds, TEXT("requested"), -1});
            if (OffscreenViewport)
            {
                // Render the real gameplay viewport including Canvas HUD. This
                // avoids Slate presentation, which is absent under RenderOffscreen.
                OffscreenViewport->Draw(false);
                FCapturedFrameData Frame(OffscreenViewport->GetSizeXY(), Payload);
                FReadSurfaceDataFlags Flags(RCM_UNorm);
                Flags.SetLinearToGamma(false);
                const bool ReadOK = OffscreenViewport->ReadPixels(Frame.ColorBuffer, Flags);
                Payload->ReadyWall = FPlatformTime::Seconds();
                if (ReadOK && Frame.ColorBuffer.Num() == Frame.BufferSize.X * Frame.BufferSize.Y)
                {
                    DirectFrames.Add(MoveTemp(Frame));
                    QueueFrames();
                }
                else
                {
                    ++ReturnedFrames;
                    FScopeLock Lock(&Writer->Mutex);
                    Writer->Records.Add(FRecord{Payload->Index, Now - Writer->Epoch, Payload->ReadyWall - Writer->Epoch, Payload->WorldSeconds, TEXT("offscreen_read_failed")});
                    UE_LOG(LogCapture09Viewport, Error, TEXT("[Capture09Record] OFFSCREEN_READ_FAILED index=%d size=%dx%d pixels=%d"), Payload->Index, Frame.BufferSize.X, Frame.BufferSize.Y, Frame.ColorBuffer.Num());
                }
            }
            else if (GPUGrabber) GPUGrabber->CaptureThisFrame(Payload);
        }
        LastRequest += Interval;
        if (LastRequest < Now) LastRequest = Now + Interval;
    }
    return true;
}

static void Start(const TArray<FString>& Args)
{
    if (Writer || RecorderBusy.GetValueOnGameThread() != 0)
    {
        UE_LOG(LogCapture09Viewport, Warning, TEXT("Capture already active. Stop it first."));
        return;
    }
    UWorld* PIEWorld = GEditor ? GEditor->PlayWorld : nullptr;
    if (!PIEWorld || !PIEWorld->GetGameViewport())
    {
        UE_LOG(LogCapture09Viewport, Error, TEXT("No PIE game viewport."));
        return;
    }
    UGameViewportClient* Client = PIEWorld->GetGameViewport();
    TSharedPtr<SViewport> Widget = Client->GetGameViewportWidget();
    TSharedPtr<ISlateViewport> Interface = Widget.IsValid() ? Widget->GetViewportInterface().Pin() : nullptr;
    if (!Interface.IsValid())
    {
        UE_LOG(LogCapture09Viewport, Error, TEXT("No Slate game viewport interface."));
        return;
    }
    TSharedPtr<FSceneViewport> SceneViewport = StaticCastSharedPtr<FSceneViewport>(Interface);
    FIntPoint Size = SceneViewport->GetSizeXY();
    const bool bOffscreen = FParse::Param(FCommandLine::Get(), TEXT("RenderOffscreen"));
    const bool bUseAsync = FParse::Param(FCommandLine::Get(), TEXT("Structure09LegacyFrameGrabber"))
        || FParse::Param(FCommandLine::Get(), TEXT("Structure09AsyncCapture"))
        || !FParse::Param(FCommandLine::Get(), TEXT("Structure09SyncCapture"));
    UE_LOG(LogCapture09Viewport, Display, TEXT("[Capture09Record] VIEWPORT before=%dx%d offscreen=%d"), Size.X, Size.Y, bOffscreen ? 1 : 0);
    if (bOffscreen)
    {
        int32 RequestedWidth = 1920, RequestedHeight = 1080;
        FParse::Value(FCommandLine::Get(), TEXT("Structure09Width="), RequestedWidth);
        FParse::Value(FCommandLine::Get(), TEXT("Structure09Height="), RequestedHeight);
        RequestedWidth = FMath::Clamp(RequestedWidth, 320, 7680);
        RequestedHeight = FMath::Clamp(RequestedHeight, 180, 4320);
        if (bUseAsync)
        {
            TSharedPtr<SWindow> CaptureWindow = FSlateApplication::Get().FindWidgetWindow(Widget.ToSharedRef());
            if (!CaptureWindow.IsValid())
            {
                UE_LOG(LogCapture09Viewport, Error, TEXT("[Capture09Record] ASYNC_NO_SLATE_WINDOW"));
                return;
            }
            // Keep Slate window/backbuffer and client viewport geometry coherent.
            // SetViewportSize alone overrides the entire window backbuffer size,
            // leaving its title/borders to steal pixels from the gameplay crop.
            CaptureWindow->Resize(FVector2D(RequestedWidth, RequestedHeight));
            // The native window can discover its non-client border during the
            // first resize. Correct the measured client-size deficit before
            // fixing the crop; otherwise the first offscreen take is all drops.
            const FVector2D ClientDeficit = FVector2D(RequestedWidth, RequestedHeight) - CaptureWindow->GetClientSizeInScreen();
            if (!ClientDeficit.IsNearlyZero())
                CaptureWindow->ReshapeWindow(CaptureWindow->GetPositionInScreen(), CaptureWindow->GetSizeInScreen() + ClientDeficit);
            CaptureWindow->SetViewportSizeDrivenByWindow(true);
            CaptureWindow->SlatePrepass(FSlateApplication::Get().GetApplicationScale() * CaptureWindow->GetDPIScaleFactor());
            // Native client size and Slate's arranged viewport can differ by
            // border pixels. Match the actual arranged viewport before fixing
            // the take's size, otherwise the next Slate tick resizes it again.
            FArrangedChildren Children(EVisibility::Visible);
            Children.AddWidget(FArrangedWidget(CaptureWindow.ToSharedRef(), CaptureWindow->GetWindowGeometryInWindow()));
            FWidgetPath Path(CaptureWindow.ToSharedRef(), Children);
            if (Path.ExtendPathTo(FWidgetMatcher(Widget.ToSharedRef()), EVisibility::Visible))
            {
                const auto Arranged = Path.FindArrangedWidget(Widget.ToSharedRef()).Get(FArrangedWidget::GetNullWidget());
                const FVector2D ViewportDeficit = FVector2D(RequestedWidth, RequestedHeight) - Arranged.Geometry.GetAbsoluteSize();
                if (!ViewportDeficit.IsNearlyZero())
                    CaptureWindow->ReshapeWindow(CaptureWindow->GetPositionInScreen(), CaptureWindow->GetSizeInScreen() + ViewportDeficit);
                CaptureWindow->SlatePrepass(FSlateApplication::Get().GetApplicationScale() * CaptureWindow->GetDPIScaleFactor());
            }
            const FVector2D WindowPixels = CaptureWindow->GetViewportSize();
            const FVector2D ClientPixels = CaptureWindow->GetClientSizeInScreen();
            SceneViewport->UpdateViewportRHI(false, RequestedWidth, RequestedHeight, EWindowMode::Windowed, PF_Unknown);
            FSlateApplication::Get().GetRenderer()->UpdateFullscreenState(CaptureWindow.ToSharedRef(), WindowPixels.X, WindowPixels.Y);
            UE_LOG(LogCapture09Viewport, Display, TEXT("[Capture09Record] ASYNC_WINDOW backbuffer=%.0fx%.0f client=%.0fx%.0f requested=%dx%d"), WindowPixels.X, WindowPixels.Y, ClientPixels.X, ClientPixels.Y, RequestedWidth, RequestedHeight);
        }
        else SceneViewport->SetViewportSize(RequestedWidth, RequestedHeight);
        Size = SceneViewport->GetSizeXY();
        if (Size.X != RequestedWidth || Size.Y != RequestedHeight)
        {
            // No Slate window geometry exists in some offscreen editor builds.
            SceneViewport->UpdateViewportRHI(false, RequestedWidth, RequestedHeight, EWindowMode::Windowed, PF_Unknown);
            Size = SceneViewport->GetSizeXY();
        }
        SceneViewport->Draw(false);
        FlushRenderingCommands();
        UE_LOG(LogCapture09Viewport, Display, TEXT("[Capture09Record] OFFSCREEN_INITIALIZED actual=%dx%d render_target_valid=%d"), Size.X, Size.Y, SceneViewport->GetRenderTargetTexture().IsValid() ? 1 : 0);
    }
    if (Size.X < 64 || Size.Y < 64)
    {
        UE_LOG(LogCapture09Viewport, Error, TEXT("[Capture09Record] INVALID_VIEWPORT_SIZE %dx%d; capture NOT started"), Size.X, Size.Y);
        return;
    }
    FString Directory = Args.Num() ? Args[0] : FPaths::ProjectSavedDir() / TEXT("PortfolioCaptures") / (FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")) + TEXT("_") + FGuid::NewGuid().ToString(EGuidFormats::Digits));
    Directory.TrimQuotesInline();
    Directory = FPaths::ConvertRelativePathToFull(Directory);
    if (IFileManager::Get().DirectoryExists(*Directory))
    {
        UE_LOG(LogCapture09Viewport, Error, TEXT("Refusing existing output directory: %s"), *Directory);
        return;
    }
    if (!IFileManager::Get().MakeDirectory(*Directory, true))
    {
        UE_LOG(LogCapture09Viewport, Error, TEXT("[Capture09Record] CREATE_DIRECTORY_FAILED %s"), *Directory);
        return;
    }
    Writer = MakeShared<FSharedWriter, ESPMode::ThreadSafe>();
    RecorderBusy.AsVariable()->Set(1, ECVF_SetByCode);
    Writer->Directory = Directory;
    Writer->Images = &FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
    Writer->Epoch = FPlatformTime::Seconds();
    World = PIEWorld;
    NextIndex = 0;
    ReturnedFrames = 0;
    LastRequest = 0;
    RecordingSize = Size;
    StopReason = TEXT("requested_stop");
    // UE 5.4 FFrameGrabber flushes from its render callback when the global
    // latency changes. Keep old invocation scripts usable through the safe GPU
    // backend instead of instantiating that optional, unsafe legacy backend.
    const bool LegacyRequested = FParse::Param(FCommandLine::Get(), TEXT("Structure09LegacyFrameGrabber"));
    if (LegacyRequested)
        UE_LOG(LogCapture09Viewport, Warning, TEXT("[Capture09Record] LEGACY_GPU_FALLBACK: using GPU readback; framegrabber.framelatency is unchanged."));
    if (bOffscreen && !bUseAsync && !LegacyRequested) OffscreenViewport = SceneViewport;
    else
    {
        GPUGrabber = MakeUnique<FStructure09AsyncReadback>(SceneViewport.ToSharedRef(), Size,
            [](FFramePayloadPtr P, double CapturedWall)
            {
                static_cast<FPayload*>(P.Get())->CapturedWall = CapturedWall;
            });
        GPUGrabber->StartCapturingFrames();
    }
    const double BeforeAudio = FPlatformTime::Seconds();
    CaptureAudioDevice = PIEWorld->GetAudioDevice();
    if (CaptureAudioDevice.IsValid())
    {
        USoundSubmix& MainSubmix = CaptureAudioDevice->GetMainSubmixObject();
        RecordingSubmix = &MainSubmix;
        OldSubmixAutoDisable = MainSubmix.bAutoDisable;
        CaptureAudioDevice->SetSubmixAutoDisable(&MainSubmix, false);
        if (bOffscreen)
        {
            AppVolumeOverride.Apply(TEXT("au.DisableAppVolume"), 1);
            // Do not modify the global solo/active routing of other PIE devices.
            OldDeviceMuted = CaptureAudioDevice->IsAudioDeviceMuted();
            CaptureAudioDevice->SetDeviceMuted(false);
            ChangedAudioRouting = true;
        }
        UE_LOG(LogCapture09Viewport, Display, TEXT("[Capture09Record] AUDIO_DEVICE id=%u main_submix=%s auto_disable=%d->0 app_volume_bypass=%d"), CaptureAudioDevice.GetDeviceID(), *MainSubmix.GetName(), OldSubmixAutoDisable ? 1 : 0, bOffscreen ? 1 : 0);
    }
    else UE_LOG(LogCapture09Viewport, Error, TEXT("[Capture09Record] AUDIO_DEVICE_UNAVAILABLE"));
    UAudioMixerBlueprintLibrary::StartRecordingOutput(PIEWorld, 60.0f, RecordingSubmix.Get());
    const double AfterAudio = FPlatformTime::Seconds();
    const FString Meta = FString::Printf(TEXT("width=%d\nheight=%d\nrequested_max_fps=%d\nwall_epoch=%.9f\nworld_start=%.9f\naudio_start_before_seconds=%.9f\naudio_start_after_seconds=%.9f\nPixels are the actual PIE viewport; desktop windows are excluded.\nWorld time is sampled at capture request; readback-ready time is not presentation time.\nNo fixed timestep, time dilation, resampling, or camera changes are applied.\n"), Size.X, Size.Y, MaxFPS.GetValueOnGameThread(), Writer->Epoch, PIEWorld->GetTimeSeconds(), BeforeAudio - Writer->Epoch, AfterAudio - Writer->Epoch);
    FFileHelper::SaveStringToFile(Meta, *(Directory / TEXT("capture.txt")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    FString Backend = GPUGrabber ? TEXT("Final Slate PIE viewport FRHIGPUTextureReadback 6-slot ring. IsReady polling; no per-frame GPU wait. Canvas/UMG included. Actual backbuffer submission timestamp recorded. Stop blocks once to drain submitted copies.\n") : TEXT("Actual PIE Draw(false)+ReadPixels; debug Canvas and Slate overlays may be excluded. Capture timestamp is request time.\n");
    if (LegacyRequested) Backend += TEXT("legacy_requested=1\nactual_backend=gpu_readback\nUE 5.4 legacy FrameGrabber is not instantiated; global framegrabber.framelatency is never modified.\n");
    Backend += TEXT("A viewport geometry change ends the fixed-size take with stop_reason=viewport_geometry_changed. Check stop_reason separately from per-file export success.\n");
    FFileHelper::SaveStringToFile(Backend, *(Directory / TEXT("backend.txt")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    Recording = true;
    TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateStatic(&Tick));
    EndPIEHandle = FEditorDelegates::EndPIE.AddLambda([](bool){ StopReason = TEXT("end_pie"); Stop(); });
    UE_LOG(LogCapture09Viewport, Display, TEXT("[Capture09Record] START %s %dx%d"), *Directory, Size.X, Size.Y);
}

static FAutoConsoleCommand StartCommand(TEXT("Portfolio.Capture09.RecordStart"), TEXT("Start authentic PIE viewport JPEG sequence + master submix WAV. Optional NEW output directory argument."), FConsoleCommandWithArgsDelegate::CreateStatic(&Start));
static FAutoConsoleCommand StopCommand(TEXT("Portfolio.Capture09.RecordStop"), TEXT("Stop capture and write timestamp CSV."), FConsoleCommandDelegate::CreateStatic(&Stop));
}
