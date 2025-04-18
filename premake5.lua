workspace "Crystal"
    configurations { "Release" }
    startproject "Crystal"
    architecture "x64"

project "Crystal"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetdir "bin/%{cfg.buildcfg}"

    files {
        "src/**.h", "src/**.c", "src/**.cpp",
        "vendor/glfw/**.h", "vendor/glfw/**.c",
        "vendor/imgui/*.h", "vendor/imgui/*.cpp",
        "vendor/ImGuiFileDialog/*.cpp", "vendor/ImGuiFileDialog/*.h",
        "vendor/tiny-process/*.cpp", "vendor/tiny-process/*.h",
        "vendor/tree-sitter/**.c", "vendor/tree-sitter/**.h",
        "vendor/TextEditor/*.h", "vendor/TextEditor/*.cpp"
    }

    includedirs {
        ".",
        "src",
        "vendor",
        "vendor/imgui",
        "vendor/ImGuiFileDialog",
        "vendor/glfw/include",
        "vendor/tree-sitter/include"
    }

    libdirs { "bin/%{cfg.buildcfg}" }

    optimize "On"

    filter { "system:windows" }
        local VULKAN_SDK = os.getenv("VULKAN_SDK")
        if VULKAN_SDK then
            includedirs { VULKAN_SDK .. "\\Include" }
            libdirs { VULKAN_SDK .. "\\Lib" }
        end
        links { "user32", "gdi32", "vulkan-1" }
        defines { "NDEBUG", "VK_USE_PLATFORM_WIN32_KHR", "_GLFW_WIN32" }

    filter { "system:linux" }
        links { "vulkan" }
        defines { "NDEBUG", "VK_USE_PLATFORM_XCB_KHR", "_GLFW_X11" }
        buildoptions { "-fpermissive" }