#pragma once

#include "Window.h"

class Morth
{
private:
	GLint mvpUniform_ = -1;
	GLint timeUniform_ = -1;
  GLint modeUniform_ = -1;
  GLint lerpUniform_ = -1;
  GLint enableManualUniform_ = -1;

	QOpenGLBuffer vbo_{QOpenGLBuffer::Type::VertexBuffer};
	QOpenGLBuffer ibo_{QOpenGLBuffer::Type::IndexBuffer};
	QOpenGLVertexArrayObject vao_;

	size_t indexCount_ = 0;
	size_t vertexCount_ = 0;

	std::unique_ptr<QOpenGLShaderProgram> program_;

public:
	void init(Window * const wnd);
	void render(Window * const wnd, const QMatrix4x4 & viewProjection);
	void release();
};
