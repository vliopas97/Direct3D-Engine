workspace "DXRenderer"
    architecture "x64"
    startproject "DXRenderer"
    toolset "v143"

    configurations
    {
        "Debug",
        "Release"
    }

    OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    group "Dependencies"
        include "DXRenderer/vendor/ImGui"
        include "DXRenderer/vendor/assimp"
    group ""

project "DXRenderer"
    location "DXRenderer"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++latest"
    staticruntime "on"
    floatingpoint "fast"

    targetdir ("bin/" .. OutputDir .. "/%{prj.name}")
    objdir ("bin-int/" .. OutputDir .. "/%{prj.name}")

    linkoptions { "/SUBSYSTEM:WINDOWS"}

    includedirs
    {
        "%{prj.name}/src",
        "%{prj.name}/vendor/glm",
        "%{prj.name}/vendor/DXErr",
        "%{prj.name}/vendor/ImGui",
        "%{prj.name}/vendor/assimp/include",
        "%{prj.name}/vendor/DirectXTex/include"
    }
    
    links
    {
        "d3d11.lib",
        "d3dcompiler.lib",
        "dxguid.lib",
        "ImGui",
        "assimp",
        "DirectXTex.lib"
    }

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.hlsl",
        "%{prj.name}/vendor/DXErr/**.h",
        "%{prj.name}/vendor/DXErr/**.cpp"
    }

    removefiles
    {
        "%{prj.name}/vendor/DXErr/**.inl"
    }
    
    filter "files:**.hlsl"
        shadermodel "5.0"
        buildmessage 'Compiling HLSL shader %{file.relpath}'
       
    filter { "files:**/VertexShaders/*.hlsl" }
        removeflags "ExcludeFromBuild"
        shadertype "Vertex"
        shaderobjectfileoutput 'src/Rendering/Shaders/build/%%(Filename)VS.cso'

    filter { "files:**/PixelShaders/*.hlsl" }
        removeflags "ExcludeFromBuild"
        shadertype "Pixel"
        shaderobjectfileoutput 'src/Rendering/Shaders/build/%%(Filename)PS.cso'

    filter {"files:**.hlsli"}
        flags {"ExcludeFromBuild"}

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        libdirs {"%{prj.name}/vendor/DirectXTex/bin/debug" }

    filter "configurations:Release"
        runtime "Release"
        symbols "on"
        optimize "Full"
        flags {"LinkTimeOptimization"}
        libdirs {"%{prj.name}/vendor/DirectXTex/bin/release" }

        defines{
            "NDEBUG"
        }
