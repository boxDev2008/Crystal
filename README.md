<p align="center">
  <img width="400px" src="content/logo-large.png">
</p>

A minimal code editor written with C++ and ImGui.

## Project Generation
> [!NOTE]
> Crystal uses [premake](https://premake.github.io/) for project generation. Feel free to install it or use the one provided in `tools`.

Run `.\generate.bat` to generate a `Visual Studio 2022` project (On Windows)

## Compilation
> [!NOTE]
> You need to have `MSBuild` the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) installed.

Run `.\build.bat` (On Windows)

## Running
Run `.\run.bat` or just cd into the `bin/Release` folder (On Windows)

## Features
- [x] Typing
- [x] File Explorer
- [x] Syntax highlighting
- [x] Lightweight
- [x] Build error markers (compile-time, msvc only)
- [x] Basic terminal
- [x] Drag and drop files
- [x] Image viewer (png, jpg)
- [x] Customizable themes
- [x] File icons
- [ ] Lsp support
