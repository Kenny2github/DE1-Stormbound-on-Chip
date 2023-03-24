@echo off
if exist combined.c del combined.c
for /R %%f in (src\*.c) do (
	echo Combining %%~nxf
	py combine.py "%%f"
)
