# DirectX Tool Kit for DirectX 11
#
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
#
# http://go.microsoft.com/fwlink/?LinkId=248929

cmake_minimum_required (VERSION 3.8)
project (DirectXTK_CMake LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(WarningsLib "-Wall" "-Wpedantic" "-Wextra")
set(WarningsEXE ${WarningsLib} "-Wno-c++98-compat" "-Wno-c++98-compat-pedantic"
    "-Wno-double-promotion" "-Wno-exit-time-destructors" "-Wno-gnu-anonymous-struct"
    "-Wno-missing-prototypes" "-Wno-nested-anon-types" "-Wno-unused-const-variable")

add_library (directxtk STATIC
    Inc/DirectXTK/Audio.h
    Inc/DirectXTK/CommonStates.h
    Inc/DirectXTK/DDSTextureLoader.h
    Inc/DirectXTK/DirectXHelpers.h
    Inc/DirectXTK/Effects.h
    Inc/DirectXTK/GamePad.h
    Inc/DirectXTK/GeometricPrimitive.h
    Inc/DirectXTK/GraphicsMemory.h
    Inc/DirectXTK/Keyboard.h
    Inc/DirectXTK/Model.h
    Inc/DirectXTK/Mouse.h
    Inc/DirectXTK/PostProcess.h
    Inc/DirectXTK/PrimitiveBatch.h
    Inc/DirectXTK/ScreenGrab.h
    Inc/DirectXTK/SimpleMath.h
    Inc/DirectXTK/SimpleMath.inl
    Inc/DirectXTK/SpriteBatch.h
    Inc/DirectXTK/SpriteFont.h
    Inc/DirectXTK/VertexTypes.h
    Inc/DirectXTK/WICTextureLoader.h
    Audio/AudioEngine.cpp
    Audio/DynamicSoundEffectInstance.cpp
    Audio/SoundCommon.cpp
    Audio/SoundCommon.h
    Audio/SoundEffect.cpp
    Audio/SoundEffectInstance.cpp
    Audio/WaveBank.cpp
    Audio/WaveBankReader.cpp
    Audio/WaveBankReader.h
    Audio/WAVFileReader.cpp
    Audio/WAVFileReader.h
    Src/AlignedNew.h
    Src/AlphaTestEffect.cpp
    Src/BasicEffect.cpp
    Src/BasicPostProcess.cpp
    Src/Bezier.h
    Src/BinaryReader.cpp
    Src/BinaryReader.h
    Src/CommonStates.cpp
    Src/ConstantBuffer.h
    Src/dds.h
    Src/DDSTextureLoader.cpp
    Src/DebugEffect.cpp
    Src/DemandCreate.h
    Src/DGSLEffect.cpp
    Src/DGSLEffectFactory.cpp
    Src/DualPostProcess.cpp
    Src/DualTextureEffect.cpp
    Src/EffectCommon.cpp
    Src/EffectCommon.h
    Src/EffectFactory.cpp
    Src/EnvironmentMapEffect.cpp
    Src/GamePad.cpp
    Src/GeometricPrimitive.cpp
    Src/Geometry.h
    Src/Geometry.cpp
    Src/GraphicsMemory.cpp
    Src/Keyboard.cpp
    Src/LoaderHelpers.h
    Src/Model.cpp
    Src/ModelLoadCMO.cpp
    Src/ModelLoadSDKMESH.cpp
    Src/ModelLoadVBO.cpp
    Src/Mouse.cpp
    Src/NormalMapEffect.cpp
    Src/PBREffect.cpp
    Src/PBREffectFactory.cpp
    Src/pch.h
    Src/PlatformHelpers.h
    Src/PrimitiveBatch.cpp
    Src/ScreenGrab.cpp
    Src/SDKMesh.h
    Src/Shaders
    Src/SharedResourcePool.h
    Src/SimpleMath.cpp
    Src/SkinnedEffect.cpp
    Src/SpriteBatch.cpp
    Src/SpriteFont.cpp
    Src/TeapotData.inc
    Src/ToneMapPostProcess.cpp
    Src/vbo.h
    Src/VertexTypes.cpp
    Src/WICTextureLoader.cpp
)

source_group(Audio REGULAR_EXPRESSION Audio/*.*)
source_group(Inc REGULAR_EXPRESSION Inc/DirectXTK/*.*)
source_group(Src REGULAR_EXPRESSION Src/*.*)

target_include_directories( directxtk PUBLIC Inc/DirectXTK )
target_include_directories( directxtk PRIVATE Audio Src )

target_compile_options( directxtk PRIVATE /EHsc /GR /fp:fast )

if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    target_compile_options( directxtk PRIVATE ${WarningsLib} )
    target_compile_options( xwbtool PRIVATE ${WarningsEXE} )
    if (${CMAKE_SIZEOF_VOID_P} EQUAL "4")
        target_compile_options( directxtk PRIVATE /arch:SSE2 )
    endif()
endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    target_compile_options( directxtk PRIVATE /Wall /permissive- /Zc:__cplusplus )
endif()

# Windows 8 is used here to use XAudio 2.8 rather than require the legacy DirectX SDK
add_compile_definitions(_UNICODE UNICODE _WIN32_WINNT=0x0602)
