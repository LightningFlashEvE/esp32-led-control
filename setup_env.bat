@echo off
echo Setting up ESP-IDF environment...

REM Set ESP-IDF path
set IDF_PATH=C:\Users\Light\esp\v5.1.6\esp-idf
set PATH=%IDF_PATH%\tools;%PATH%

REM Activate Python environment
call C:\Users\Light\.espressif\python_env\idf5.1_py3.11_env\Scripts\activate.bat

REM Export ESP-IDF tools
call %IDF_PATH%\export.bat

echo Environment ready. You can now use:
echo   idf.py build
echo   idf.py -p COM3 flash monitor
echo.

cmd /k
