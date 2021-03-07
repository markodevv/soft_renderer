@echo off

set compiler_flags=-DEBUG -Gm- -GR- -EHa- -Od -Oi -W3 -wd4201 -wd4100 -wd4189 -DGAME_DEBUG=1 -FC -Z7 -MTd


IF NOT EXIST build mkdir build
pushd build

REM 64-bit build
cl  %compiler_flags% ../renderer.cpp
popd
