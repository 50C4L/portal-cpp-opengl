#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <memory>

namespace portal
{
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

		static void GLUTRenderCallback();
		static void GLUTResizeCallback( int width, int height );
		static void GLUTUpdateCallback(int value);

	public:
		///
		/// 用CreateApp()，不要直接构造
		///
		Application( Params params );

		/// 
		/// 初始化所有东西
		/// 
		void Initialize();

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

	private:
		static Ptr sInstance;

		Params mParams;
		int mWindowWidth;
		int mWindowHeight;
	};
}

#endif
