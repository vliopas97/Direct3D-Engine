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
        "%{prj.name}/vendor/stb_image",
        "%{prj.name}/vendor/ImGui",
        "%{prj.name}/vendor/assimp/include"
    }
    
    links
    {
        "d3d11.lib",
        "d3dcompiler.lib",
        "dxguid.lib",
        "ImGui",
        "assimp"
    }

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.hlsl",
        "%{prj.name}/vendor/DXErr/**.h",
        "%{prj.name}/vendor/DXErr/**.cpp",
        "%{prj.name}/vendor/stb_image/**.h",
        "%{prj.name}/vendor/stb_image/**.cpp"
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

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        symbols "on"
        optimize "Full"
        flags {"LinkTimeOptimization"}

        defines{
            "NDEBUG"
        }
