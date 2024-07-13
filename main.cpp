#include "Application.h"

int main(void)
{
	std::shared_ptr<Crystal::Application> app = Crystal::CreateApplication();
	app->Run();
	return 0;
}
