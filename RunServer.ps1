# LastStand 데디케이트 서버 실행 스크립트
# 사용법: .\RunServer.ps1 [-ServerExe <경로>] [-Map <맵>] [-Port <포트>]
# 로그 콘솔은 항상 표시됨 (-log 기본 포함)

param(
    # 실행할 서버 exe 경로 (패키지 빌드 기본값, 필요시 수정)
    [string]$ServerExe = "C:\Workspace\LastStandTestBuild\WindowsServer\LastStandServer.exe",
    [string]$Map  = "/Game/Level/NewMap",
    [int]   $Port = 7777
)

if (-not (Test-Path $ServerExe)) {
    Write-Error "Server exe not found: $ServerExe"
    exit 1
}

$serverArgs = @($Map, "-Port=$Port", "-log")

Write-Host "[RunServer] $ServerExe $($serverArgs -join ' ')" -ForegroundColor Cyan
& $ServerExe @serverArgs
