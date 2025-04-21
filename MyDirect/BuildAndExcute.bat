@echo off
setlocal

REM 솔루션 경로 설정
set SLN_PATH=%~dp0MyDirect.sln

REM MSBuild 경로 (Visual Studio 2022 Community 기준)
set MSBUILD_PATH="C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"

REM 빌드 실행
echo [빌드 시작...]
%MSBUILD_PATH% %SLN_PATH% /p:Configuration=Debug /p:Platform=x64

REM 빌드 결과 확인
if %errorlevel% neq 0 (
    echo [빌드 실패! 종료합니다.]
    exit /b %errorlevel%
)

REM 실행파일 경로 설정 (MyDirect 프로젝트가 .exe 생성한다고 가정)
set EXE_PATH=%~dp0x64\Debug\MyDirect.exe

REM 실행파일 존재 확인 후 실행
if exist %EXE_PATH% (
    echo [실행 시작...]
    start "" "%EXE_PATH%"
) else (
    echo [실행 파일이 존재하지 않습니다: %EXE_PATH%]
)

endlocal
pause
