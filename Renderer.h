#ifndef _Renderer_H
#define _Renderer_H

#include <unordered_map>
#include <vector>
#include <memory>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace portal
{
	class Camera;

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec4 color;
		glm::vec2 uv;
		glm::vec3 normal;
	};

	///
	/// 简易（简陋）渲染器
	/// 只能在获取OpenGL Context后使用
	/// 
	class Renderer
	{
	public:
		static const std::string DEFAULT_SHADER;
		static const std::string DEBUG_PHYSICS_SHADER;
		static const std::string PORTAL_HOLE_SHADER;
		static const std::string PORTAL_FRAME_SHADER;

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
			~Shader();

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
			/// 设置模型矩阵
			/// 
			/// @param matrix
			///		Const reference to matrix
			/// 
			void SetModelMatrix( const glm::mat4& matrix );

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
			int mModelMatUniformLocation;
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
			///
			/// 顶点列表绘制类型
			/// 
			enum class DrawType
			{
				LINES,
				TRIANGLES,
				TRIANGLE_FANS
			};

			///
			/// 构造函数
			/// 
			/// @param vertices
			///		顶点
			/// 
			/// @param shader_name
			///		本次渲染用到的shader
			/// 
			/// @param texture_id
			///		本次渲染用到的贴图
			/// 
			/// @param draw_type
			///		绘制类型，默认三角形
			/// 
			Renderable( 
				std::vector<Vertex>&& vertices, 
				std::string shader_name, 
				unsigned int texture_id, 
				DrawType draw_type = DrawType::TRIANGLES );
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
			/// 平移
			/// 
			/// @param offset
			///		平移量
			/// 
			void Translate( glm::vec3 offset );

			///
			/// 选择
			/// 
			/// @param angle
			///		旋转角度
			/// 
			/// @param axis
			///		转轴
			/// 
			void Rotate( float angle, glm::vec3 axis );

			std::string GetShaderName() const;
			unsigned int GetTexture() const;
			DrawType GetDrawType() const;
			glm::mat4 GetTransform();

		private:
			unsigned int mVBO;
			unsigned int mVAO;
			int mNumberOfVertices;
			std::string mShader;
			unsigned int mTexture;
			DrawType mDrawType;
			glm::vec3 mTranslation;
			glm::vec3 mRotation;
			glm::mat4 mTransform;
			bool mIsDirty;
		};

		///
		/// 简陋渲染资源管理器
		/// 负责加载贴图，shader
		/// 
		class Resources
		{
		public:
			Resources();
			~Resources() = default;

			///
			/// 从文件加载贴图
			/// 
			/// @param path
			///		贴图文件相对路径
			/// 
			/// @return bool
			///		True表示成功
			/// 
			bool LoadTexture( const std::string& path );

			///
			/// 获取已加载的贴图Id
			/// 
			/// @param path
			///		贴图文件的路径
			/// 
			/// @param unsigned int
			///		Texture id
			/// 
			unsigned int GetTextureId( const std::string& path );

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
			/// 根据名字查找并返回已编译的shader
			/// 
			/// @param name
			///		THE NAME
			/// 
			/// @return unsigned int
			///		Reference to Shader
			/// 
			Shader& GetShader( const std::string& name );

		private:
			std::unordered_map<std::string, unsigned int> mLoadedTextures;
			std::unordered_map<std::string, Shader> mCompiledShaders;
		};

public:
		/// Renderer class
		Renderer();
		~Renderer();

		void ResizeViewport( glm::ivec2 size );
		glm::ivec2 GetViewportSize();

		///
		/// 渲染!!!
		/// 
		void RenderOneoff( Renderable* renderable_obj );

		///
		/// 将提供的摄像机作为之后渲染的摄像机
		/// 
		/// @param camera
		///		Raw pointer to Camera
		/// 
		void UseCameraMatrix( Camera* camera );

		void SetViewMatrix( glm::mat4 view );

		void SetProjectionMatrix( glm::mat4 projection );

		Resources& GetResources();

	private:
		glm::mat4 mProjectionMatrix;
		glm::mat4 mViewMatrix;

		std::unique_ptr<Resources> mResources;

		glm::ivec2 mViewportSize;
	};
}

#endif
