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
		/// 打包参数传给gluInit()
		/// 并没有用上
		/// 
		struct Params
		{
			int argc;
			char** argv;
		};

		///
		/// 创建并返回一个std::shared_ptr<Application>
		/// 
		/// @param parmas
		///		命令行参数 （没有用）
		/// 
		/// @return
		///		std::shared_ptr<Application>
		/// 
		static Ptr CreateApp( Params params );

		///
		/// GLUT回调函数，具体看Application.cpp
		/// 
		static void GLUTRenderCallback();
		static void GLUTResizeCallback( int width, int height );
		static void GLUTUpdateCallback( int value );
		static void GLUTMouseMoveCallback( int x, int y );
		static void GLUTKeyboardDownCallback( unsigned char key, int x, int y );
		static void GLUTKeyboardUpCallback( unsigned char key, int x, int y );

	public:
		///
		/// 用CreateApp()，不要直接构造
		///
		Application( Params params );

		/// 
		/// 初始化所有东西
		/// 
		bool Initialize();

		///
		/// 进入主循环
		///
		void Run();

		///
		/// 更新游戏逻辑
		/// 
		void Update();

		///
		/// 渲染一帧
		/// 
		void Render();

	private:
		///
		/// 改变视口大小
		/// 
		/// @param width
		///		宽度
		/// 
		/// @param height
		///		高度
		/// 
		void ResizeViewport( int width, int height );

		///
		/// 鼠标位置改变（移动）
		/// 
		/// @param x, y
		///		鼠标窗口坐标
		/// 
		void MouseMoved( int x, int y );

		///
		/// 键盘按键更新
		/// 
		/// @param key
		///		键位的ASCII
		/// 
		/// @param is_down
		///		是否被按下
		/// 
		void KeyChanged( unsigned char key, bool is_down );

	private:
		static Ptr sInstance;

		Params mParams;
		int mWindowWidth;
		int mWindowHeight;
		std::unique_ptr<Renderer> mRenderer;
		std::unique_ptr<LevelController> mLevelController;
		std::unordered_map<unsigned int, bool> mKeyStatus;
	};
}

#endif
