powershell -NoProfile -ExecutionPolicy Bypass -File generate-icons.ps1
if errorlevel 1 exit /b 1
..\..\bin\make.exe DEFINES="-DTARGET_PRIZM=1 -DDEBUG=0" CONFIG=DeviceRelease
pause