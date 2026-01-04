#pragma once

#include "NoCopyMove.h"
#include "WxBoxCountFrame.h"
#include <wx/app.h>
#include <wx/image.h>

namespace eg::bc
{
	class WxApp :
		public wxApp,
		private eg::sys::NoCopyMove
	{
	public:
		bool OnInit() override
		{
			wxInitAllImageHandlers();

			WxBoxCountFrame* frame = new WxBoxCountFrame(nullptr);
			frame->Show(true);

			return true;
		}

		int OnExit() override
		{
			return wxApp::OnExit();
		}
	};
}