:BUILD_OPIUM
echo [*] Compiling OPIUM...
echo.

g++ -o build\OPIUM.exe ^
    src\main.cpp ^
    src\window.cpp ^
    src\database.cpp ^
    src\auth.cpp ^
    src\crypto.cpp ^
    -Icryptopp ^
    -Isrc ^
    -Lbuild ^
    -lcryptopp ^
    -lgdi32 ^
    -lcomctl32 ^
    -lcomdlg32 ^
    -luser32 ^
    -std=c++17 ^
    -O2 ^
    -static-libgcc ^
    -static-libstdc++ ^
    -w

if %errorlevel% neq 0 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

echo [OK] OPIUM.exe ready!
echo ================================
build\OPIUM.exe
pause