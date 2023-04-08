@echo off

if "%1"=="assets" goto :assets
if "%1"=="" goto :combined.c
if "%1"=="combined.c" goto :combined.c

goto :error

:combined.c

echo #define CPULATOR>combined.c
for /R "src" %%f in (*.c) do (
	echo Combining %%~nxf
	py scripts\combine.py "%%f"
)

goto :eof

:assets

setlocal ENABLEDELAYEDEXPANSION

for /R "assets" %%f in (*.png *.jpg *.bmp *.txt) do (
	set "reldir=%%~dpf"
	call set "reldir=%%reldir:%CD%\=%%"
	if not exist "src\!reldir!" mkdir src\!reldir!
	echo Converting "!reldir!\%%~nxf" to "src\!reldir!\%%~nxf.c"
	if "%%~xf"==".txt" py scripts\txt_convert.py "!reldir!\%%~nxf" "src\!reldir!\%%~nxf.c"
	if "%%~xf"==".png" py scripts\img_convert.py "!reldir!\%%~nxf" "src\!reldir!\%%~nxf.c"
	if "%%~xf"==".jpg" py scripts\img_convert.py "!reldir!\%%~nxf" "src\!reldir!\%%~nxf.c"
	if "%%~xf"==".bmp" py scripts\img_convert.py "!reldir!\%%~nxf" "src\!reldir!\%%~nxf.c"
)

echo Combining into include\assets.h
py scripts\assets_combine.py

goto :eof

:error

echo error: unknown target "%1"

:eof
