local VULKAN_LIB_PATH = os.getenv("VULKAN_SDK").."\\Lib";

workspace "Crystal"
	configurations { "Release" }
	startproject "Crystal"

project "GLFW"
	kind "StaticLib"
	language "C"
	targetdir "bin/%{cfg.buildcfg}"

	architecture "x64"

	files { "glfw/*.h", "glfw/*.c" }
	includedirs { "include" }

	libdirs { "bin/%{cfg.buildcfg}" }

	filter { "system:windows" }
		libdirs { VULKAN_LIB_PATH }
		links { "user32", "gdi32", "vulkan-1" }
		filter "configurations:Release"
			defines { "NDEBUG", "VK_USE_PLATFORM_WIN32_KHR", "_GLFW_WIN32" }

project "Crystal"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	targetdir "bin/%{cfg.buildcfg}"

	architecture "x64"

	files { "src/**.h", "src/**.cpp", "imgui/*.h", "imgui/*.cpp" }
	includedirs { ".", "src", "imgui", "include" }

	links { "GLFW" }
	libdirs { "bin/%{cfg.buildcfg}" }

   	filter { "system:windows" }
		libdirs { VULKAN_LIB_PATH }
		links { "user32", "gdi32", "vulkan-1" }
		filter "configurations:Release"
        defines { "NDEBUG", "VK_USE_PLATFORM_WIN32_KHR" }