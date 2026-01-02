#include "WxApp.hpp"

#include <iostream>
#include <stdexcept>

wxIMPLEMENT_APP_NO_MAIN(wxApp);

int APIENTRY wWinMain(HINSTANCE hinst, HINSTANCE, LPWSTR, int ncmdshow)
{
	try
	{
		wxApp::SetInstance(new eg::bc::WxApp);
		if (not wxEntryStart(hinst))
		{
			return EXIT_FAILURE;
		}

		if (not wxTheApp->CallOnInit())
		{
			wxEntryCleanup();
			return EXIT_FAILURE;
		}

		wxTheApp->OnRun();
		wxTheApp->OnExit();

		wxEntryCleanup();

		return EXIT_SUCCESS;
	}

	catch (const std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
		return EXIT_FAILURE;
	}

	catch (...)
	{
		MessageBoxA(nullptr, "Unknown error encountered.", "Error", MB_OK | MB_ICONERROR);
		return EXIT_FAILURE;
	}
}