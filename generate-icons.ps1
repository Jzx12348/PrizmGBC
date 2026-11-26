$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($name in @("selected", "unselected")) {
    $source = Join-Path $root ($name + ".bmp.base64")
    $target = Join-Path $root ($name + ".bmp")
    $encoded = (Get-Content -LiteralPath $source -Raw).Trim()
    [System.IO.File]::WriteAllBytes($target, [System.Convert]::FromBase64String($encoded))
}

Write-Host "Generated PrizmGBC add-in icons."
