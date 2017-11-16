@echo off
set workdir=%~dp0
set workdir=%workdir:~0,-1%
echo 当前工作目录：%workdir%

copy /Y "..\Debug\PluginServer.dll" "..\..\Bin\Debug\plugin\PluginServer.dll"
copy /Y "config.ini"                "..\..\Bin\Debug\plugin\config.ini"

copy /Y "..\Debug\PluginServer.dll" "C:\Program Files (x86)\FTNN\plugin\PluginServer.dll"
copy /Y "config.ini"                "C:\Program Files (x86)\FTNN\plugin\config.ini"
pause

