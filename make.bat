@echo off

if "%1"=="assets" goto :assets
if "%1"=="" goto :combined.c
if "%1"=="combined.c" goto :combined.c

goto :error

:combined.c

if exist combined.c del combined.c
for /R "src" %%f in (*.c) do (
	echo Combining %%~nxf
	py scripts\combine.py "%%f"
)

goto :eof

:assets

for %%f in (assets\*.png assets\*.jpg assets\*.bmp) do (
	echo Converting "assets\%%~nxf" to "src\assets\%%~nxf.c"
	py scripts\img_convert.py "assets\%%~nxf" "src\assets\%%~nxf.c"
)

echo Combining into include\assets.h
py scripts\img_combine.py

goto :eof

:error

echo error: unknown target "%1"

:eof
