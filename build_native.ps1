param(
    [string]$Source = "server/game_logic.cpp",
    [string]$OutDir = "server"
)

Write-Host "Building native library from $Source" -ForegroundColor Cyan

if (-not (Test-Path $Source)) {
    Write-Error "Source file not found: $Source"
    exit 1
}

$tmp = Join-Path $OutDir "game_logic.build.cpp"

# Create temporary copy with __declspec(dllexport) removed (same approach as Dockerfile)
(Get-Content $Source -Raw) -replace '__declspec\(dllexport\)', '' | Set-Content -Path $tmp -Encoding UTF8

# Ensure g++ exists
$gpp = (Get-Command g++ -ErrorAction SilentlyContinue)
if (-not $gpp) {
    Write-Error "g++ not found in PATH. Please install MinGW, MSYS2, WSL, or have g++ available.";
    Remove-Item -Force $tmp -ErrorAction SilentlyContinue
    exit 2
}

if ($IsWindows) {
    $outDll = Join-Path $OutDir "game_logic.dll"
    $outSo = Join-Path $OutDir "game_logic.so"
    $args = @('-O2', '-shared', '-static-libgcc', '-static-libstdc++', '-o', $outDll, $tmp)
}
else {
    $outSo = Join-Path $OutDir "game_logic.so"
    $args = @('-O2', '-fPIC', '-shared', '-o', $outSo, $tmp)
}

Write-Host "Running: g++ $($args -join ' ')" -ForegroundColor Yellow

$proc = & g++ @args 2>&1
$code = $LASTEXITCODE
if ($code -ne 0) {
    Write-Host $proc
    Write-Error "g++ failed with exit code $code"
    Remove-Item -Force $tmp -ErrorAction SilentlyContinue
    exit $code
}

Write-Host "Build succeeded." -ForegroundColor Green

if ($IsWindows) {
    # Some setups expect a .so; copy .dll to .so so Python loader path (game_logic.so) may work
    try {
        Copy-Item -Force $outDll $outSo
        Write-Host "Copied $outDll -> $outSo" -ForegroundColor Green
    }
    catch {
        Write-Warning "Failed to copy DLL to SO: $_"
    }
}

# Cleanup
Remove-Item -Force $tmp -ErrorAction SilentlyContinue

Write-Host "Native build finished. Output: " -NoNewline
if (Test-Path $outSo) { Write-Host $outSo -ForegroundColor Cyan } elseif (Test-Path $outDll) { Write-Host $outDll -ForegroundColor Cyan } else { Write-Host "(no output found)" -ForegroundColor Red }

exit 0
