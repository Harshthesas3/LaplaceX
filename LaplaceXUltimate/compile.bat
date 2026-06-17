@echo off
:: Re-initialize environment for x64
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

:: Compile with Unicode support. 
:: (Libraries are now linked via #pragma comment in main.cpp)
cl.exe /EHsc /W4 /DUNICODE /D_UNICODE main.cpp /Fe:LaplaceX.exe

if %ERRORLEVEL% equ 0 (
    echo.
    echo ========================================
    echo Compilation Successful!
    echo ========================================
    echo.
) else (
    echo.
    echo ========================================
    echo Compilation Failed!
    echo ========================================
    echo.
)
pause
