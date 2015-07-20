@echo off

echo Launch 1
start .\Release-1.0.2.18-6100\ta.exe
echo Wait for 30 sec
timeout /t 30 > nul

echo Launch 2
start .\Release-1.0.2.18-6102\ta.exe

echo Done