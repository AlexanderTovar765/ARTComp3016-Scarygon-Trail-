# This part of the code downloads irrKlang (sound library) into vendored/irrKlang so the game builds WITH sound.
#
# Why I used a script and not committed to the repo: irrKlang's licence lets you redistribute
# its DLL with a non-commercial program, but not the whole SDK archive. So the SDK stays out of
# git (see .gitignore) and each machine fetches its own copy.
#
# Run from the repo root in PowerShell:   powershell -ExecutionPolicy Bypass -File tools\get-irrklang.ps1
# Without irrKlang the game still builds and runs - it is just silent.

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$dest = Join-Path $root "vendored\irrKlang"
$tmp  = Join-Path $env:TEMP ("irrklang_" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force -Path $tmp | Out-Null

# Official downloads: https://www.ambiera.com/irrklang/downloads.html (irrKlang 1.6.0, 64-bit).
# This script uses a public GitHub mirror of the same SDK so it can be fetched without a browser.
$zip = Join-Path $tmp "irrklang.zip"
Write-Host "Downloading irrKlang..."
Invoke-WebRequest -Uri "https://codeload.github.com/jonas2602/irrKlang/zip/refs/heads/master" -OutFile $zip
Expand-Archive -Path $zip -DestinationPath $tmp -Force
$sdk = Get-ChildItem -Path $tmp -Directory | Where-Object { $_.Name -like "irrKlang*" } | Select-Object -First 1
if (-not $sdk) { throw "Could not find the extracted irrKlang folder." }

New-Item -ItemType Directory -Force -Path "$dest\include", "$dest\lib", "$dest\bin" | Out-Null
Copy-Item "$($sdk.FullName)\include\*" "$dest\include" -Recurse -Force
Copy-Item "$($sdk.FullName)\lib\Winx64-visualStudio\irrKlang.lib" "$dest\lib" -Force
Copy-Item "$($sdk.FullName)\bin\winx64-visualStudio\*.dll" "$dest\bin" -Force
Copy-Item "$($sdk.FullName)\readme.txt" "$dest\irrKlang-readme.txt" -Force

Remove-Item -Recurse -Force $tmp
Write-Host "Done. irrKlang is in vendored\irrKlang. Rebuild the project to get sound."
