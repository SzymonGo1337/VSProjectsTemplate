workspace "VSProjectsTemplate"
    architecture "x86_64"
    startproject "Project"


    configurations{
        "Debug",
        "Release"
    }

    flags {
        "MultiProcessorCompile"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "Project/vendor/GLFW/include"
IncludeDir["GLAD"] = "Project/vendor/GLAD/include"
IncludeDir["ImGui"] = "Project/vendor/ImGui"
IncludeDir["spdlog"] = "Project/vendor/spdlog/include"
IncludeDir["glm"] = "Project/vendor/glm"
IncludeDir["stb"] = "Project/vendor/stb"
IncludeDir["entt"] = "Project/vendor/entt/single_include"

group "Dependencies"
    include "Project/vendor"

group ""

project "Project"
    location "Project"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    
    pchheader "dfpch.h"
	pchsource "Project/src/dfpch.cpp"
    
    
    files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.tpp",
        "%{prj.name}/out/**",
        "Premake5.lua"
    }
    
    defines
	{
		"_CRT_SECURE_NO_WARNINGS",
        "DF_NO_LOG_FILE",
        "DF_EDITOR"
	}

    includedirs
	{
        "Project/src",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.GLAD}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb}",
        "%{IncludeDir.entt}"
    }

    links 
	{ 
        "GLFW",
        "GLAD",
        "ImGui",
        "glm",
        "spdlog",
        "opengl32.lib"
    }
    
    debugdir "Project/out"
    
    filter "system:windows"
		systemversion "latest"
		
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
