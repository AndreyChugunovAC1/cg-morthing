#pragma once

#include "Window.h"
#include <QOpenGLFunctions>

#include <tinygltf/tiny_gltf.h>

class Duck
{
private:
	GLint mvpUniform_ = -1;
	GLint timeUniform_ = -1;
	GLint userPosUniform_ = -1;
	GLint dotLightAngleUniform_ = -1;
	GLint dotLightHeightUniform_ = -1;
	GLint enableDotLightUniform_ = -1;
	GLint spotLightLatitudeUniform_ = -1;
	GLint spotLightLongitudeUniform_ = -1;
	GLint enableSpotLightUniform_ = -1;

	QOpenGLBuffer vbo_{QOpenGLBuffer::Type::VertexBuffer};
	QOpenGLBuffer ibo_{QOpenGLBuffer::Type::IndexBuffer};
	QOpenGLVertexArrayObject vao_;

	size_t indexCount_ = 0;
	size_t vertexCount_ = 0;

	std::unique_ptr<QOpenGLTexture> texture_;
	std::unique_ptr<QOpenGLShaderProgram> program_;

	void loadInterleavedData(const tinygltf::Model & model,
							 const tinygltf::Primitive & primitive,
							 GLfloat * vertices, int cnt, bool hasNormals);
	void loadIndices(const tinygltf::Model & model,
					 const tinygltf::Primitive & primitive,
					 std::vector<GLuint> & indices);

public:
	void init(Window * const wnd);
	void render(Window * const wnd, const QMatrix4x4 & viewProjection);
	void release();
};