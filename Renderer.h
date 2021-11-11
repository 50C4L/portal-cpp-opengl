#ifndef _Renderer_H
#define _Renderer_H

#include <unordered_map>
#include <vector>

namespace portal
{
	class Renderer
	{
	public:
		const std::string DEFAULT_SHADER = "DEFLAULT_SHADER";

		struct Vertex
		{
			float x;
			float y;
			float z;
		};

		class Renderable
		{
		public:
			Renderable( std::vector<Vertex>&& vertices );
			~Renderable();

			unsigned int GetVAO();

		private:
			unsigned int mVBO;
			unsigned int mVAO;
		};

		Renderer();
		~Renderer();

		///
		/// 初始化
		/// 编译默认shader
		/// 
		bool Initialize();

		///
		/// 编译Shader，成功编译后Shader的program id会存放在mCompiledShaders里。
		/// Key是提供的名字
		/// 
		/// @param name
		///		存放在map中对应的名字
		/// 
		/// @param vertex_shader
		///		顶点shader
		/// 
		/// @param fragment_shader
		///		片源shader
		/// 
		/// @return
		///		true成功编译，false失败
		/// 
		bool CompileShader( const std::string& name, std::string vertex_shader, std::string fragment_shader );

		///
		/// 渲染提供的Renderable obj
		/// 
		/// @param renderable_obj
		///		Renderable引用
		/// 
		void Render( Renderable& renderable_obj );

	private:
		std::unordered_map<std::string, unsigned int> mCompiledShaders;
	};
}

#endif
