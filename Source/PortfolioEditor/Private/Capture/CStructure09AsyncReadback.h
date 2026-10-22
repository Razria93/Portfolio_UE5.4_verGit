#pragma once

#include "CoreMinimal.h"
#include "CRecordingLifecycle.h"
#include "FrameGrabber.h"
#include "Framework/Application/SlateApplication.h"
#include "Layout/ArrangedChildren.h"
#include "Layout/WidgetPath.h"
#include "Misc/ScopeLock.h"
#include "Rendering/SlateRenderer.h"
#include "RenderingThread.h"
#include "RHICommandList.h"
#include "RHIGPUReadback.h"
#include "Widgets/SViewport.h"
#include "Widgets/SWindow.h"

// A payload stays on the same GPU readback slot as its pixels. Lock is called
// only after IsReady(), never to wait on a current-frame GPU fence.
class FStructure09AsyncReadback
{
    struct FSlot
    {
        TUniquePtr<FRHIGPUTextureReadback> Readback;
        FFramePayloadPtr Payload;
        EPixelFormat Format = PF_Unknown;
        bool Pending = false;
    };
    TArray<FSlot> Slots;
    FIntPoint TargetSize;
    FIntRect Crop;
    SWindow* TargetWindow = nullptr;
    FDelegateHandle Delegate;
    TWeakPtr<FSceneViewport> SourceViewport;
    PortfolioRecording::FRecordingGeometry InitialGeometry;
    TAtomic<bool> GeometryChanged{false};

    static bool ReadGeometry(TSharedRef<FSceneViewport> Viewport, PortfolioRecording::FRecordingGeometry& Out)
    {
        TSharedPtr<SViewport> Widget = Viewport->GetViewportWidget().Pin();
        if (!Widget || !FSlateApplication::IsInitialized()) return false;
        TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(Widget.ToSharedRef());
        if (!Window) return false;
        FArrangedChildren Children(EVisibility::Visible);
        Children.AddWidget(FArrangedWidget(Window.ToSharedRef(), Window->GetWindowGeometryInWindow()));
        FWidgetPath Path(Window.ToSharedRef(), Children);
        if (!Path.ExtendPathTo(FWidgetMatcher(Widget.ToSharedRef()), EVisibility::Visible)) return false;
        const auto Arranged = Path.FindArrangedWidget(Widget.ToSharedRef()).Get(FArrangedWidget::GetNullWidget());
        const FVector2D Position = Arranged.Geometry.GetAbsolutePosition();
        const FVector2D WindowSize = Window->GetViewportSize();
        Out.ViewportSize = Viewport->GetSizeXY();
        Out.CropOrigin = FIntPoint(FMath::RoundToInt(Position.X), FMath::RoundToInt(Position.Y));
        Out.BackBufferSize = FIntPoint(FMath::RoundToInt(WindowSize.X), FMath::RoundToInt(WindowSize.Y));
        Out.WindowId = reinterpret_cast<UPTRINT>(Window.Get());
        return true;
    }
    FCriticalSection RequestMutex, FrameMutex;
    TArray<FFramePayloadPtr> Requests;
    TArray<FCapturedFrameData> Frames;
    TFunction<void(FFramePayloadPtr, double)> Submitted;
    bool Active = false;
    bool LoggedBackBuffer = false;

    void CompleteFailed(FFramePayloadPtr Payload)
    {
        if (Payload.IsValid()) Payload->OnFrameReady_RenderThread(nullptr, FIntPoint::ZeroValue, TargetSize);
        FScopeLock Lock(&FrameMutex);
        Frames.Emplace(FIntPoint::ZeroValue, Payload);
    }

    void PumpReady()
    {
        for (FSlot& Slot : Slots)
        {
            if (!Slot.Pending || !Slot.Readback->IsReady()) continue;
            int32 Pitch = 0, Height = 0;
            const void* Data = Slot.Readback->Lock(Pitch, &Height);
            if (!Data || Pitch < TargetSize.X || Height < TargetSize.Y)
            {
                if (Data) Slot.Readback->Unlock();
                CompleteFailed(Slot.Payload);
                Slot.Payload.Reset();
                Slot.Pending = false;
                continue;
            }
            FCapturedFrameData Frame(TargetSize, Slot.Payload);
            Frame.ColorBuffer.SetNumUninitialized(TargetSize.X * TargetSize.Y);
            for (int32 Y = 0; Y < TargetSize.Y; ++Y)
            {
                const uint32* Source = static_cast<const uint32*>(Data) + int64(Y) * Pitch;
                FColor* Dest = Frame.ColorBuffer.GetData() + int64(Y) * TargetSize.X;
                if (Slot.Format == PF_B8G8R8A8)
                    FMemory::Memcpy(Dest, Source, int64(TargetSize.X) * sizeof(FColor));
                else if (Slot.Format == PF_R8G8B8A8)
                {
                    for (int32 X = 0; X < TargetSize.X; ++X)
                    {
                        const uint32 V = Source[X];
                        Dest[X] = FColor(V & 255, (V >> 8) & 255, (V >> 16) & 255, 255);
                    }
                }
                else // SDR 10-bit swapchain, channels packed R10 G10 B10 A2.
                {
                    for (int32 X = 0; X < TargetSize.X; ++X)
                    {
                        const uint32 V = Source[X];
                        Dest[X] = FColor(((V & 1023) * 255 + 511) / 1023, (((V >> 10) & 1023) * 255 + 511) / 1023, (((V >> 20) & 1023) * 255 + 511) / 1023, 255);
                    }
                }
            }
            Slot.Readback->Unlock();
            if (Slot.Payload.IsValid()) Slot.Payload->OnFrameReady_RenderThread(Frame.ColorBuffer.GetData(), TargetSize, TargetSize);
            {
                FScopeLock Lock(&FrameMutex);
                Frames.Add(MoveTemp(Frame));
            }
            Slot.Payload.Reset();
            Slot.Pending = false;
        }
    }

    void OnBackBuffer(SWindow& Window, const FTexture2DRHIRef& Texture)
    {
        if (&Window != TargetWindow) return;
        PumpReady();
        FFramePayloadPtr Payload;
        {
            FScopeLock Lock(&RequestMutex);
            if (Requests.IsEmpty()) return;
            Payload = Requests[0];
            Requests.RemoveAt(0, 1, EAllowShrinking::No);
        }
        // The render-thread check also closes the resize-to-next-game-tick gap.
        // Slate geometry is read only on the game thread; this compares RHI size.
        if (GeometryChanged.Load() || FIntPoint(Texture->GetSizeX(), Texture->GetSizeY()) != InitialGeometry.BackBufferSize)
        {
            GeometryChanged.Store(true);
            CompleteFailed(Payload);
            return;
        }
        FSlot* FreeSlot = nullptr;
        for (FSlot& Slot : Slots) if (!Slot.Pending) { FreeSlot = &Slot; break; }
        const EPixelFormat Format = Texture->GetFormat();
        if (!LoggedBackBuffer)
        {
            UE_LOG(LogTemp, Display, TEXT("[Capture09GPUReadback] source=%ux%u format=%d crop=(%d,%d)-(%d,%d)"), Texture->GetSizeX(), Texture->GetSizeY(), int32(Format), Crop.Min.X, Crop.Min.Y, Crop.Max.X, Crop.Max.Y);
            LoggedBackBuffer = true;
        }
        if (!FreeSlot || (Format != PF_B8G8R8A8 && Format != PF_R8G8B8A8 && Format != PF_A2B10G10R10)
            || Crop.Min.X < 0 || Crop.Min.Y < 0 || Crop.Max.X > int32(Texture->GetSizeX()) || Crop.Max.Y > int32(Texture->GetSizeY()))
        {
            CompleteFailed(Payload);
            return;
        }
        Submitted(Payload, FPlatformTime::Seconds());
        FreeSlot->Payload = Payload;
        FreeSlot->Format = Format;
        FreeSlot->Pending = true;
        FRHICommandListImmediate& Cmd = GetImmediateCommandList_ForRenderCommand();
        Cmd.Transition(FRHITransitionInfo(Texture, ERHIAccess::SRVGraphics, ERHIAccess::CopySrc));
        FreeSlot->Readback->EnqueueCopy(Cmd, Texture, FIntVector(Crop.Min.X, Crop.Min.Y, 0), 0, FIntVector(TargetSize.X, TargetSize.Y, 1));
        Cmd.Transition(FRHITransitionInfo(Texture, ERHIAccess::CopySrc, ERHIAccess::SRVGraphics));
    }

public:
    FStructure09AsyncReadback(TSharedRef<FSceneViewport> Viewport, FIntPoint Size, TFunction<void(FFramePayloadPtr, double)> OnSubmitted)
        : TargetSize(Size), Crop(FIntPoint::ZeroValue, Size), Submitted(MoveTemp(OnSubmitted))
    {
        SourceViewport = Viewport;
        if (!ReadGeometry(Viewport, InitialGeometry)) GeometryChanged.Store(true);
        TargetWindow = reinterpret_cast<SWindow*>(InitialGeometry.WindowId);
        Crop = FIntRect(InitialGeometry.CropOrigin, InitialGeometry.CropOrigin + TargetSize);
        for (int32 Index = 0; Index < 6; ++Index)
        {
            FSlot& Slot = Slots.AddDefaulted_GetRef();
            Slot.Readback = MakeUnique<FRHIGPUTextureReadback>(FName(*FString::Printf(TEXT("Structure09Readback%d"), Index)));
        }
        UE_LOG(LogTemp, Display, TEXT("[Capture09GPUReadback] crop=(%d,%d)-(%d,%d) slots=6"), Crop.Min.X, Crop.Min.Y, Crop.Max.X, Crop.Max.Y);
    }
    ~FStructure09AsyncReadback() { Shutdown(); }
    bool HasGeometryChanged()
    {
        check(IsInGameThread());
        PortfolioRecording::FRecordingGeometry Current;
        TSharedPtr<FSceneViewport> Viewport = SourceViewport.Pin();
        if (!Viewport || !ReadGeometry(Viewport.ToSharedRef(), Current) || !InitialGeometry.Matches(Current))
            GeometryChanged.Store(true);
        return GeometryChanged.Load();
    }
    void StartCapturingFrames()
    {
        Active = true;
        Delegate = FSlateApplication::Get().GetRenderer()->OnBackBufferReadyToPresent().AddRaw(this, &FStructure09AsyncReadback::OnBackBuffer);
    }
    void CaptureThisFrame(FFramePayloadPtr Payload)
    {
        if (!Active) return;
        FScopeLock Lock(&RequestMutex);
        Requests.Add(Payload);
    }
    void StopCapturingFrames() { Active = false; }
    TArray<FCapturedFrameData> GetCapturedFrames()
    {
        FScopeLock Lock(&FrameMutex);
        TArray<FCapturedFrameData> Result;
        Swap(Result, Frames);
        return Result;
    }
    void Shutdown()
    {
        if (!Delegate.IsValid()) return;
        Active = false;
        FSlateApplication::Get().GetRenderer()->OnBackBufferReadyToPresent().Remove(Delegate);
        Delegate.Reset();
        // One deliberate GPU wait at STOP only drains already-submitted copies.
        // No GPU synchronization wait occurs during normal frame capture.
        ENQUEUE_RENDER_COMMAND(Structure09DrainReadbacks)([this](FRHICommandListImmediate& Cmd)
        {
            Cmd.BlockUntilGPUIdle();
            PumpReady();
        });
        FlushRenderingCommands();
        FScopeLock Lock(&RequestMutex);
        for (FFramePayloadPtr Payload : Requests) CompleteFailed(Payload);
        Requests.Reset();
    }
};
