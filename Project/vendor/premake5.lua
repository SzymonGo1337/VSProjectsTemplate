project "glm"
    kind "StaticLib"
    language "C++"
    
    targetdir ("../../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../../bin-int/" .. outputdir .. "/%{prj.name}")

    includedirs
    {
        "glm"
    }

    files
    {
        "glm/glm/**.cpp",
        "glm/glm/**.hpp",
        "premake5.lua"
    }
    
    filter "system:windows"
        systemversion "latest"
        cppdialect "C++17"
        staticruntime "On"
        
    filter { "system:windows", "configurations:Release" }
        buildoptions "/MT"

project "GLFW"
	kind "StaticLib"
	language "C"

	targetdir ("../../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../../bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"GLFW/include/GLFW/glfw3.h",
		"GLFW/include/GLFW/glfw3native.h",
		"GLFW/src/glfw_config.h",
		"GLFW/src/context.c",
		"GLFW/src/init.c",
		"GLFW/src/input.c",
		"GLFW/src/monitor.c",
		"GLFW/src/vulkan.c",
		"GLFW/src/window.c",
		"premake5.lua"
	}
	filter "system:linux"
		pic "On"

		systemversion "latest"
		staticruntime "On"

		files
		{
			"GLFW/src/x11_init.c",
			"GLFW/src/x11_monitor.c",
			"GLFW/src/x11_window.c",
			"GLFW/src/xkb_unicode.c",
			"GLFW/src/posix_time.c",
			"GLFW/src/posix_thread.c",
			"GLFW/src/glx_context.c",
			"GLFW/src/egl_context.c",
			"GLFW/src/osmesa_context.c",
			"GLFW/src/linux_joystick.c"
		}

		defines
		{
			"_GLFW_X11"
		}

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

		files
		{
			"GLFW/src/win32_init.c",
			"GLFW/src/win32_joystick.c",
			"GLFW/src/win32_monitor.c",
			"GLFW/src/win32_time.c",
			"GLFW/src/win32_thread.c",
			"GLFW/src/win32_window.c",
			"GLFW/src/wgl_context.c",
			"GLFW/src/egl_context.c",
			"GLFW/src/osmesa_context.c"
		}

		defines 
		{ 
			"_GLFW_WIN32",
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

project "GLAD"
    kind "StaticLib"
    language "C"
    
    targetdir ("../../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../../bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "GLAD/include/glad/glad.h",
        "GLAD/include/KHR/khrplatform.h",
        "GLAD/src/glad.c",
        "premake5.lua"
    }

    includedirs
    {
        "GLAD/include"
    }
    
    filter "system:windows"
        systemversion "latest"
        staticruntime "On"
        
    filter { "system:windows", "configurations:Release" }
        buildoptions "/MT"

project "ImGui"
    kind "StaticLib"
    language "C++"
    
    targetdir ("../../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../../bin-int/" .. outputdir .. "/%{prj.name}")

    includedirs
    {
        "ImGui/ImGui",
        "GLFW/include",
        "GLAD/include"
    }

    defines {
        "IMGUI_IMPL_OPENGL_LOADER_GLAD"
    }

    files
    {
        "ImGui/ImGui/imconfig.h",
        "ImGui/ImGui/imgui.h",
        "ImGui/ImGui/imgui.cpp",
        "ImGui/ImGui/imgui_draw.cpp",
        "ImGui/ImGui/imgui_internal.h",
        "ImGui/ImGui/imgui_widgets.cpp",
        "ImGui/ImGui/imstb_rectpack.h",
        "ImGui/ImGui/imstb_textedit.h",
        "ImGui/ImGui/imstb_truetype.h",
        "ImGui/ImGui/imgui_demo.cpp",
        "ImGui/ImGui/examples/imgui_impl_glfw.h",
        "ImGui/ImGui/examples/imgui_impl_glfw.cpp",
        "ImGui/ImGui/examples/imgui_impl_opengl3.h",
        "ImGui/ImGui/examples/imgui_impl_opengl3.cpp",
        "premake5.lua"
    }
    
    filter "system:windows"
        systemversion "latest"
        cppdialect "C++17"
        staticruntime "On"
        
    filter { "system:windows", "configurations:Release" }
        buildoptions "/MT"

project "spdlog"
	kind "StaticLib"
	language "C++"

	targetdir ("../../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../../bin-int/" .. outputdir .. "/%{prj.name}")

    defines
    {
        "SPDLOG_COMPILED_LIB"
    }

	includedirs
	{
        "spdlog/include"
    }

	files
	{
        "spdlog/include/**.h",
        "spdlog/src/**.cpp",
        "premake5.lua"
	}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

project "stb"
    kind "StaticLib"
    language "C"
    
    targetdir ("../../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../../bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "stb/**.h",
        "stb/**.c",
        "premake5.lua"
    }

    includedirs
    {
        "stb/stb/"
    }
    
    filter "system:windows"
        systemversion "latest"
        staticruntime "On"
        
    filter { "system:windows", "configurations:Release" }
        buildoptions "/MT"

project "entt"
	kind "StaticLib"
	language "C++"

	targetdir ("../../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../../bin-int/" .. outputdir .. "/%{prj.name}")

	includedirs
	{
        "entt/single_include"
    }

    defines {
        "ENTT_STANDALONE"
    }

	files
	{
        "entt/single_include/entt/*.hpp"
	}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
