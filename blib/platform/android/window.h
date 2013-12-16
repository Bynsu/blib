#pragma once

#include <blib/IWindow.h>

#include <EGL/egl.h>


namespace blib
{
	namespace platform
	{
		namespace android
		{
			class Window : public blib::IWindow
			{
			private:
			    EGLDisplay display;
			    EGLSurface surface;
			    EGLContext context;
			protected:
			public:
				Window();
				virtual ~Window();

				virtual void swapBuffers();
				virtual void create(int icon, std::string title);
				virtual void makeCurrent();
				virtual void unmakeCurrent();
			};
		}
	}
}