# LastStand 클라이언트 실행 스크립트 (실행 즉시 서버 접속)
# 사용법: .\RunClient.ps1 [-ClientExe <경로>] [-ServerIP <IP>] [-Port <포트>] [-Windowed] [-ResX <w>] [-ResY <h>]
# 로그 콘솔은 항상 표시됨 (-log 기본 포함)

param(
    # 실행할 클라 exe 경로 (패키지 빌드 기본값, 필요시 수정)
    [string]$ClientExe = "C:\Workspace\LastStandTestBuild\Windows\LastStandClient.exe",
    [string]$ServerIP = "127.0.0.1",
    [int]   $Port     = 7777,
    [switch]$Windowed,
    [int]   $ResX = 1280,
    [int]   $ResY = 720
)

if (-not (Test-Path $ClientExe)) {
    Write-Error "Client exe not found: $ClientExe"
    exit 1
}

$clientArgs = @("$ServerIP`:$Port", "-log")
if ($Windowed)   { $clientArgs += @("-windowed", "-ResX=$ResX", "-ResY=$ResY") }

Write-Host "[RunClient] $ClientExe $($clientArgs -join ' ')" -ForegroundColor Cyan
& $ClientExe @clientArgs
