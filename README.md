# RollingSphere
An animation system with various graphics effects.

I use Xcode as my develope environment

My source code: rolling-sphere-final.cpp
My vertex shader file: vshader53.glsl, vFirework.glsl
My fragment shader file: fshader53.glsl, fFirework.glsl

The folloing files (without modification) are needed for compiling:
	Angel-yjc.h
	CheckError.h
	CMakeLists.txt
	InitShader.cpp
	mat-yjc-new.h
	vec.h

Put all these files above into the same directory, run cmake:
	$mkdir build
	$cd build
	$cmake .. -G Xcode

Open the .xcodeproj file in Xcode, click "ALL_BUILD" 

Copy the following file into build/Debug:
	vshader53.glsl
	fshader53.glsl
	vFirework.glsl
	fFirework.glsl
	sphere.1024.txt
