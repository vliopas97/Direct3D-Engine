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
        "%{prj.name}/vendor/DXErr"
    }
    
    links
    {
        "d3d11.lib",
        "d3dcompiler.lib",
        "dxguid.lib"
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
        shaderobjectfileoutput 'src/Rendering/Shaders/%%(Filename).cso'
    filter { "files:**/V**.hlsl" }
        removeflags "ExcludeFromBuild"
        shadertype "Vertex"

    filter { "files:**/P**.hlsl" }
        removeflags "ExcludeFromBuild"
        shadertype "Pixel"
    
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
