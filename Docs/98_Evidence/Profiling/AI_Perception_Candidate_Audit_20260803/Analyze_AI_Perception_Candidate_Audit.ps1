$ErrorActionPreference = 'Stop'
$Root = $PSScriptRoot

function Get-P95([double[]]$Values) {
    $Sorted = @($Values | Sort-Object)
    if ($Sorted.Count -eq 0) { return $null }
    return $Sorted[[Math]::Ceiling($Sorted.Count * 0.95) - 1]
}

function Read-CsvP95([string]$Path) {
    $Lines = Get-Content -LiteralPath $Path
    $Header = $Lines[0].Split(',')
    $Names = @('FrameTime', 'GameThreadTime', 'GPUTime', 'Exclusive/GameThread/AIPerception')
    $Index = @{}
    foreach ($Name in $Names) { $Index[$Name] = [Array]::IndexOf($Header, $Name) }

    $ElapsedSeconds = 0.0
    $Rows = @()
    for ($RowIndex = 1; $RowIndex -lt $Lines.Count; $RowIndex++) {
        $Fields = $Lines[$RowIndex].Split(',')
        $FrameTime = 0.0
        [void][double]::TryParse($Fields[$Index['FrameTime']], [Globalization.NumberStyles]::Float, [Globalization.CultureInfo]::InvariantCulture, [ref]$FrameTime)
        $ElapsedSeconds += $FrameTime / 1000.0
        $Row = [ordered]@{ Time = $ElapsedSeconds }
        foreach ($Name in $Names) {
            $Value = 0.0
            [void][double]::TryParse($Fields[$Index[$Name]], [Globalization.NumberStyles]::Float, [Globalization.CultureInfo]::InvariantCulture, [ref]$Value)
            $Row[$Name] = $Value
        }
        $Rows += [pscustomobject]$Row
    }

    $Kept = @($Rows | Where-Object { $_.Time -ge 3 -and $_.Time -le ($ElapsedSeconds - 3) })
    return [pscustomobject]@{
        DurationSeconds = [Math]::Round($ElapsedSeconds, 3)
        KeptFrames = $Kept.Count
        FrameTimeP95 = [Math]::Round((Get-P95 ([double[]]@($Kept | ForEach-Object { $_.FrameTime }))), 4)
        GameThreadP95 = [Math]::Round((Get-P95 ([double[]]@($Kept | ForEach-Object { $_.GameThreadTime }))), 4)
        GPUP95 = [Math]::Round((Get-P95 ([double[]]@($Kept | ForEach-Object { $_.GPUTime }))), 4)
        AIPerceptionP95 = [Math]::Round((Get-P95 ([double[]]@($Kept | ForEach-Object { $_.'Exclusive/GameThread/AIPerception' }))), 4)
    }
}

function Read-CandidateAuditP95([string]$Path) {
    $Rows = @(Select-String -LiteralPath $Path -Pattern '\[PerceptionCandidateAudit\]' | ForEach-Object {
        if ($_.Line -match 'RawActors=(?<Raw>\d+).*?InvalidProviders=(?<Invalid>\d+).*?MaxTargetDataMap=(?<Map>\d+).*?FirstValidLatency=(?<FirstValid>[\d.]+)') {
            [pscustomobject]@{ Raw = [double]$Matches.Raw; Invalid = [double]$Matches.Invalid; Map = [double]$Matches.Map; FirstValid = [double]$Matches.FirstValid }
        }
    })
    return [pscustomobject]@{
        Owners = $Rows.Count
        RawActorsP95 = Get-P95 ([double[]]$Rows.Raw)
        InvalidProvidersP95 = Get-P95 ([double[]]$Rows.Invalid)
        MaxTargetDataMapP95 = Get-P95 ([double[]]$Rows.Map)
        FirstValidLatencyP95 = [Math]::Round((Get-P95 ([double[]]$Rows.FirstValid)), 3)
    }
}

$Cases = @(
    @{ Name = 'Before'; Csv = 'Profile(20260803_143437).csv'; Log = 'Log(20260803_143437).txt' },
    @{ Name = 'After'; Csv = 'Profile(20260803_151028).csv'; Log = 'Log(20260803_151028).txt' }
)

$Results = foreach ($Case in $Cases) {
    $Csv = Read-CsvP95 (Join-Path $Root $Case.Csv)
    $Audit = Read-CandidateAuditP95 (Join-Path $Root $Case.Log)
    [pscustomobject]@{
        Case = $Case.Name
        Owners = $Audit.Owners
        RawActorsP95 = $Audit.RawActorsP95
        InvalidProvidersP95 = $Audit.InvalidProvidersP95
        MaxTargetDataMapP95 = $Audit.MaxTargetDataMapP95
        FirstValidLatencyP95Seconds = $Audit.FirstValidLatencyP95
        FrameTimeP95Milliseconds = $Csv.FrameTimeP95
        AIPerceptionP95Milliseconds = $Csv.AIPerceptionP95
        CaptureDurationSeconds = $Csv.DurationSeconds
        KeptFrames = $Csv.KeptFrames
    }
}

$Results | Format-Table -AutoSize
