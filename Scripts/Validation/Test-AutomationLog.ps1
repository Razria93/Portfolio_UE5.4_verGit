param([Parameter(Mandatory = $true)][string]$LogPath)
$ErrorActionPreference = 'Stop'
$lines = Get-Content -LiteralPath $LogPath
$success = @($lines | Select-String 'Test Completed\. Result=\{Success\}')
$failure = @($lines | Select-String 'Test Completed\. Result=\{Fail\}')
$completed = $lines | Select-String 'Automation Test Queue Empty (\d+) tests performed\.' | Select-Object -Last 1
$expected = if ($completed) { [int]$completed.Matches[0].Groups[1].Value } else { 0 }
Write-Output "Success=$($success.Count) Fail=$($failure.Count) Executed=$expected"
$failure | ForEach-Object { Write-Output $_.Line }
if ($expected -eq 0 -or $failure.Count -gt 0 -or $success.Count -ne $expected) {
    Write-Error 'Automation did not complete with every executed test successful.'
    exit 1
}
exit 0
