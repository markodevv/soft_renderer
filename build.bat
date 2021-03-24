@echo off

set compiler_flags=-DDEBUG -Gm- -GR- -EHa- -O2 -Oi -W3 -wd4201 -wd4100 -wd4189 -D_CRT_SECURE_NO_WARNINGS -FC -Z7 -MTd -nologo
set linker_flags= /INCREMENTAL:NO /OPT:REF winmm.lib user32.lib gdi32.lib shell32.lib

IF NOT EXIST build mkdir build
pushd build

REM 64-bit build
cl  %compiler_flags% ../renderer.cpp /link %linker_flags%
popd
