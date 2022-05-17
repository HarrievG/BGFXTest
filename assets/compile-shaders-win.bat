@echo off

REM compile shaders

set shaderDefines=USE_SKINNING

@echo PBR/Tonemapper with ( %shaderDefines% )
bin\shadercRelease.exe ^
-f shaders\fs_tonemap.sc -o shaders\fs_tonemap.bin ^
--platform windows --type fragment --verbose -i ./ -p ps_5_0 --define %shaderDefines%

bin\shadercRelease.exe ^
-f shaders\vs_tonemap.sc -o shaders\vs_tonemap.bin ^
--platform windows --type vertex --verbose -i ./ -p vs_5_0 --define %shaderDefines%

bin\shadercRelease.exe ^
-f shaders\fs_forward.sc -o shaders\fs_forward.bin ^
--platform windows --type fragment --verbose -i ./ -p ps_5_0 --debug -O 0 --define %shaderDefines%

bin\shadercRelease.exe ^
-f shaders\vs_forward.sc -o shaders\vs_forward.bin ^
--platform windows --type vertex --verbose -i ./ -p vs_5_0 --debug -O 0 --define %shaderDefines%

bin\shadercRelease.exe ^
-f shaders\cs_multiple_scattering_lut.sc -o shaders\cs_multiple_scattering_lut.bin ^
--platform windows --type compute --verbose -i ./ -p cs_5_0 --debug -O 0 --define %shaderDefines%
@echo =======

@echo SWF
bin\shadercRelease.exe ^
-f shaders\fs_swf.sc -o shaders\fs_swf.bin ^
--platform windows --type fragment --verbose -i ./ -p ps_5_0 --debug -O 0

bin\shadercRelease.exe ^
-f shaders\vs_swf.sc -o shaders\vs_swf.bin ^
--platform windows --type vertex --verbose -i ./ -p vs_5_0 --debug -O 0
@echo =======

@echo degbugRender
bin\shadercRelease.exe ^
-f shaders\fs_debugRender.sc -o shaders\fs_debugRender.bin ^
--platform windows --type fragment --verbose -i ./ -p ps_5_0 --debug -O 0

bin\shadercRelease.exe ^
-f shaders\vs_debugRender.sc -o shaders\vs_debugRender.bin ^
--platform windows --type vertex --verbose -i ./ -p vs_5_0 --debug -O 0
@echo =======