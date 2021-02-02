#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "CMainApplication.h"

int main(int argc, char **argv)
{
	CMainApplication *pMainApplication = new CMainApplication();

	if (!pMainApplication->BInit())
	{
		pMainApplication->Shutdown();
		return 1;
	}

	pMainApplication->RunMainLoop();

	pMainApplication->Shutdown();

	return 0;
}
