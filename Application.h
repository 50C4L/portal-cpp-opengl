#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <memory>
#include "Renderer.h"

namespace portal
{
	class LevelController;

	class Application
	{
	public:
		using Ptr = std::shared_ptr<Application>;

		///
		/// Pack args to pass to gluInit()
		/// Not actually used
		/// 
		struct Params
		{
			int argc;
			char** argv;
		};

		///
		/// Create and return a std::shared_ptr<Application>
		/// 
		/// @param parmas
		///		Command-line args (unused)
		/// 
		/// @return
		///		std::shared_ptr<Application>
		/// 
		static Ptr CreateApp( Params params );

		///
		/// GLUT callbacks, see Application.cpp for the details
		/// 
		static void GLUTRenderCallback();
		static void GLUTResizeCallback( int width, int height );
		static void GLUTUpdateCallback( int value );
		static void GLUTMouseMoveCallback( int x, int y );
		static void GLUTKeyboardDownCallback( unsigned char key, int x, int y );
		static void GLUTKeyboardUpCallback( unsigned char key, int x, int y );
		static void GLUTMousePressedCallback( int button, int state, int x, int y );

	public:
		///
		/// Use CreateApp(), don't construct this directly
		///
		Application( Params params );

		/// 
		/// Initialize everything
		/// 
		bool Initialize();

		///
		/// Enter the main loop
		///
		void Run();

		///
		/// Update game logic
		/// 
		void Update();

		///
		/// Render one frame
		/// 
		void Render();

	private:
		///
		/// Change the viewport size
		/// 
		/// @param width
		///		Width
		/// 
		/// @param height
		///		Height
		/// 
		void ResizeViewport( int width, int height );

		///
		/// Mouse position changed (moved)
		/// 
		/// @param x, y
		///		Mouse coordinates in the window
		/// 
		void MouseMoved( int x, int y );

		///
		/// Keyboard key update
		/// 
		/// @param key
		///		ASCII of the key
		/// 
		/// @param is_down
		///		Whether it's pressed
		/// 
		void KeyChanged( unsigned char key, bool is_down );

		void MousePresed( int button, bool is_pressed );

	private:
		static Ptr sInstance;

		Params mParams;
		int mWindowWidth;
		int mWindowHeight;
		std::unique_ptr<Renderer> mRenderer;
		std::unique_ptr<LevelController> mLevelController;
		std::unordered_map<unsigned int, bool> mKeyStatus;
		std::unordered_map<int, bool> mMouseButtonState;
	};
}

#endif
