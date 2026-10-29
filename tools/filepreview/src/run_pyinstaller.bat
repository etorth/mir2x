@echo off
rem Build a single-file filepreview.exe into .\dist
rem Temporary build files and the generated .spec go into .\build
setlocal
cd /d "%~dp0"

python3 -m PyInstaller --noconfirm --clean --onefile --windowed ^
    --name filepreview ^
    --icon "%~dp0favicon.ico" ^
    --add-data "%~dp0favicon.ico;." ^
    --distpath "%~dp0dist" ^
    --workpath "%~dp0build" ^
    --specpath "%~dp0build" ^
    "%~dp0main.py"

if errorlevel 1 (
    echo.
    echo Build FAILED
    pause
    exit /b 1
)

echo.
echo Build OK: %~dp0dist\filepreview.exe
