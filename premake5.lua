local VULKAN_LIB_PATH = os.getenv("VULKAN_SDK").."\\Lib";

workspace "Crystal"
	configurations { "Release" }
	startproject "Crystal"

project "Crystal"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	targetdir "bin/%{cfg.buildcfg}"

	architecture "x64"

	files {
		"src/**.h", "src/**.c", "src/**.cpp",
		"vendor/glfw/**.h", "vendor/glfw/**.c",
		"vendor/imgui/*.h", "vendor/imgui/*.cpp",
		"vendor/ImGuiFileDialog/*.cpp", "vendor/ImGuiFileDialog/*.h",
		"vendor/tiny-process/*.cpp", "vendor/tiny-process/*.h"
	}
	includedirs {
		".",
		"src",
		"vendor",
		"vendor/imgui",
		"vendor/ImGuiFileDialog",
		"vendor/glfw/include"
	}

	libdirs { "bin/%{cfg.buildcfg}" }

	optimize "On"

   	filter { "system:windows" }
		libdirs { VULKAN_LIB_PATH }
		links { "user32", "gdi32", "vulkan-1" }
        defines { "NDEBUG", "VK_USE_PLATFORM_WIN32_KHR", "_GLFW_WIN32" }