#ifndef _Renderer_H
#define _Renderer_H

#include <unordered_map>
#include <vector>
#include <memory>
#include <chrono>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace portal
{
	class Camera;

	///
	/// 简易（简陋）渲染器
	/// 
	class Renderer
	{
	public:
		static const std::string DEFAULT_SHADER;

		///
		/// Shader类
		/// 
		class Shader
		{
		public:
			///
			/// 默认构造函数，将会编译内置shader
			/// 
			Shader();

			///
			/// 参数构造
			/// 
			/// @param vertex_shader
			///		顶点shader文本
			/// 
			/// @param fragment_shader
			///		片源shader文本
			/// 
			Shader( const std::string& vertex_shader, const std::string& fragment_shader );
			~Shader() = default;

			///
			/// 检查Shader是否编译成功
			/// 
			/// @return bool
			///		True表示编译成功
			/// 
			bool IsValid() const;

			///
			/// 获取Shader id
			/// 
			/// @return unsigned int
			///		OpenGL Shader Program id
			/// 
			unsigned int GetId() const;

			///
			/// 设置视图矩阵
			/// 
			/// @param matrix
			///		Const reference to view matrix
			/// 
			void SetViewMatrix( const glm::mat4& matrix );

			///
			/// 设置投影矩阵
			/// 
			/// @param matrix
			///		Const reference to projection matrix
			/// 
			void SetProjectionMatrix( const glm::mat4& matrix );

			///
			///	设置矩阵
			/// 
			/// @param location
			///		shader中通过glGetUniformLocation获取的变量位置
			/// 
			/// @param matrix
			///		Const reference to the matrix you want
			/// 
			void SetMat4( int location, const glm::mat4& matrix );

		private:
			bool mIsValid;
			unsigned int mId;
			int mViewMatUniformLocation;
			int mProjectionMatUniformLocation;
		};

		///
		/// 可渲染对象
		/// 包括了它用到的Shader id, texture id, 缓存id
		/// 
		class Renderable
		{
		public:
			Renderable( std::vector<glm::vec3>&& vertices, std::string shader_name );
			~Renderable();

			///
			/// 获取VAO Id
			/// 
			/// @return unsigned int
			///		OpenGL VAO id
			/// 
			unsigned int GetVAO() const;

			///
			/// 获取顶点数量
			/// 
			/// @return int
			///		Number of vertices
			/// 
			int GetNumberOfVertices() const;

			///
			/// 获取需要的Shader名字
			/// 
			/// @return std::string
			///		Shader名字
			/// 
			std::string GetShaderName() const;

		private:
			unsigned int mVBO;
			unsigned int mVAO;
			int mNumberOfVertices;
			std::string mShader;
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

		///
		/// 将提供的摄像机作为之后渲染的摄像机
		/// 
		/// @param camera
		///		Shared pointer to Camera
		/// 
		void SetCameraAsActive( std::shared_ptr<Camera> camera );

	private:
		std::unordered_map<std::string, Shader> mCompiledShaders;
		std::shared_ptr<Camera> mActiveCamera;
		glm::mat4 mProjectionMatrix;

		std::chrono::steady_clock::time_point mLastRenderTimepoint;
	};
}

#endif
