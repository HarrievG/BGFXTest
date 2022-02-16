@echo off

REM compile shaders

REM simple shader
bin\shadercRelease.exe ^
-f shaders\v_simple.sc -o shaders\v_simple.bin ^
--platform windows --type vertex --verbose -i ./ -p vs_5_0

bin\shadercRelease.exe ^
-f shaders\f_simple.sc -o shaders\f_simple.bin ^
--platform windows --type fragment --verbose -i ./ -p ps_5_0

bin\shadercRelease.exe ^
-f shaders\fs_bae_textured_physical.sc -o shaders\f_pbr.bin ^
--platform windows --type fragment --verbose -i ./ -p ps_5_0

bin\shadercRelease.exe ^
-f shaders\vs_bae_textured_physical.sc -o shaders\v_pbr.bin ^
--platform windows --type vertex --verbose -i ./ -p vs_5_0

bin\shadercRelease.exe ^
-f shaders\vs_bae_textured_physical.sc -o shaders\v_pbr.bin ^
--platform windows --type vertex --verbose -i ./ -p vs_5_0

bin\shadercRelease.exe ^
-f shaders\fs_tonemap.sc -o shaders\fs_tonemap.bin ^
--platform windows --type fragment --verbose -i ./ -p ps_5_0

bin\shadercRelease.exe ^
-f shaders\vs_tonemap.sc -o shaders\vs_tonemap.bin ^
--platform windows --type vertex --verbose -i ./ -p vs_5_0

bin\shadercRelease.exe ^
-f shaders\fs_forward.sc -o shaders\fs_forward.bin ^
--platform windows --type fragment --verbose -i ./ -p ps_5_0

bin\shadercRelease.exe ^
-f shaders\vs_forward.sc -o shaders\vs_forward.bin ^
--platform windows --type vertex --verbose -i ./ -p vs_5_0

bin\shadercRelease.exe ^
-f shaders\cs_multiple_scattering_lut.sc -o shaders\cs_multiple_scattering_lut.bin ^
--platform windows --type compute --verbose -i ./ -p cs_5_0