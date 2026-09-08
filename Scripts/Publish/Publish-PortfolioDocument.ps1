[CmdletBinding()]
param(
    [string]$Destination,
    [switch]$Clean
)

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$sourceDirectory = Join-Path $repoRoot 'Docs\07_Portfolio_Documents\Portfolio_Production'
$submittedSources = Get-ChildItem -LiteralPath $sourceDirectory -File -Filter '*_UE5_Portfolio_Project_Stellar.html'
$sourceHtml = if ($submittedSources.Count -eq 1) { $submittedSources[0].FullName } else { $null }
$outputRoot = [System.IO.Path]::GetFullPath((Join-Path $repoRoot 'output'))

if (-not $sourceHtml) {
    throw "Expected exactly one submitted portfolio source matching *_UE5_Portfolio_Project_Stellar.html under: $sourceDirectory"
}

if ([string]::IsNullOrWhiteSpace($Destination)) {
    $Destination = Join-Path $outputRoot 'portfolio-pages'
}

$destinationPath = [System.IO.Path]::GetFullPath($Destination)
if (-not $destinationPath.StartsWith($outputRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw 'Destination must stay under the repository output directory.'
}

if (Test-Path -LiteralPath $destinationPath) {
    if (-not $Clean) {
        throw "Destination already exists: $destinationPath. Re-run with -Clean to replace this generated staging directory."
    }
    Remove-Item -LiteralPath $destinationPath -Recurse -Force
}

New-Item -ItemType Directory -Path $destinationPath -Force | Out-Null
Copy-Item -LiteralPath $sourceHtml -Destination (Join-Path $destinationPath 'index.html')
Copy-Item -LiteralPath (Join-Path $sourceDirectory 'Assets') -Destination (Join-Path $destinationPath 'Assets') -Recurse

$publishedHtml = Get-Content -LiteralPath (Join-Path $destinationPath 'index.html') -Raw
$imageReferences = [regex]::Matches($publishedHtml, 'src="([^"]+)"') | ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique
$missing = $imageReferences | Where-Object { -not (Test-Path -LiteralPath (Join-Path $destinationPath ($_ -replace '/', '\'))) }
if ($missing) {
    throw "Publish validation failed. Missing asset references:`n$($missing -join "`n")"
}

Write-Host "Portfolio Pages staging package created: $destinationPath"
Write-Host "Validated image references: $($imageReferences.Count)"
