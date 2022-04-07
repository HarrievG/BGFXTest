@echo off

REM compile shaders

REM simple shader
REM bin\shadercRelease.exe ^
REM -f shaders\v_simple.sc -o shaders\v_simple.bin ^
REM --platform windows --type vertex --verbose -i ./ -p vs_5_0
REM 
REM bin\shadercRelease.exe ^
REM -f shaders\f_simple.sc -o shaders\f_simple.bin ^
REM --platform windows --type fragment --verbose -i ./ -p ps_5_0
REM 
REM bin\shadercRelease.exe ^
REM -f shaders\fs_bae_textured_physical.sc -o shaders\f_pbr.bin ^
REM --platform windows --type fragment --verbose -i ./ -p ps_5_0
REM 
REM bin\shadercRelease.exe ^
REM -f shaders\vs_bae_textured_physical.sc -o shaders\v_pbr.bin ^
REM --platform windows --type vertex --verbose -i ./ -p vs_5_0
REM 
REM bin\shadercRelease.exe ^
REM -f shaders\vs_bae_textured_physical.sc -o shaders\v_pbr.bin ^
REM --platform windows --type vertex --verbose -i ./ -p vs_5_0

bin\shadercRelease.exe ^
-f shaders\fs_tonemap.sc -o shaders\fs_tonemap.bin ^
--platform windows --type fragment --verbose -i ./ -p ps_5_0

bin\shadercRelease.exe ^
-f shaders\vs_tonemap.sc -o shaders\vs_tonemap.bin ^
--platform windows --type vertex --verbose -i ./ -p vs_5_0

bin\shadercRelease.exe ^
-f shaders\fs_forward.sc -o shaders\fs_forward.bin ^
--platform windows --type fragment --verbose -i ./ -p ps_5_0 --debug -O 0

bin\shadercRelease.exe ^
-f shaders\vs_forward.sc -o shaders\vs_forward.bin ^
--platform windows --type vertex --verbose -i ./ -p vs_5_0 --debug -O 0

bin\shadercRelease.exe ^
-f shaders\cs_multiple_scattering_lut.sc -o shaders\cs_multiple_scattering_lut.bin ^
--platform windows --type compute --verbose -i ./ -p cs_5_0 --debug -O 0

@echo SWF
bin\shadercRelease.exe ^
-f shaders\fs_swf.sc -o shaders\fs_swf.bin ^
--platform windows --type fragment --verbose -i ./ -p ps_5_0 --debug -O 0

bin\shadercRelease.exe ^
-f shaders\vs_swf.sc -o shaders\vs_swf.bin ^
--platform windows --type vertex --verbose -i ./ -p vs_5_0 --debug -O 0
@echo =======