#include "Application.h"
#include "Utils.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <vector>

#include <tree_sitter/api.h>
#include <tree_sitter/tree_sitter_cpp.h>

#if 0
#include <windows.h>

int WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    int         nCmdShow
    )
{
	Crystal::Application app = Crystal::Application();
	return 0;
}

#else

int main(int argc, char **argv)
{
	Crystal::Application app = Crystal::Application();
	return 0;
}

#endif