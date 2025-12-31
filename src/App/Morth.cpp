#include "Morth.h"

#include <algorithm>
#include <cmath>
#include <vector>

void Morth::init(Window * const wnd)
{
	static constexpr size_t N = 60;// 2N - side of box
	float PI = acos(-1);

	// x1 y1 z1 nx1 ny1 nz1 x2 y2 z2 nx2 xy2 nz2
	std::vector<GLfloat> vertices;
	std::vector<GLfloat> sphereVertices;
	std::vector<GLfloat> cubeVertices;
	std::vector<size_t> indices;

	size_t numOfCircles = 4 * N - 2;
	float deltaPhi = PI / numOfCircles;
	float phi = deltaPhi / 2.0f;

	// top
	for (size_t i = 0; i < N; i++)
	{
		size_t numOfDots = 8 * i + 4;

		for (size_t j = 0; j < numOfDots; j++)
		{
			float theta = 2.0 * PI * ((float)j + 0.5) / numOfDots;
			float s = sin(phi);
			float x = s * (float)cos(theta);
			float y = (float)cos(phi);
			float z = s * (float)sin(theta);

			float x1 = -x / y;
			float z1 = -z / y;

			float k = sqrt(x1 * x1 + z1 * z1) / (std::max(abs(x1), abs(z1)));
			sphereVertices.insert(sphereVertices.end(), {x, y, z, x, y, z});
			cubeVertices.insert(cubeVertices.end(), {k * x / y, 1, k * z / y, 0, 1, 0});
		}
		phi += deltaPhi;
	}
	// body
	for (size_t i = 0; i < 2 * N - 2; i++)
	{
		size_t numOfDots = 8 * N - 4;

		for (size_t j = 0; j < numOfDots; j++)
		{
			float theta = 2.0 * PI * ((float)j + 0.5) / numOfDots;
			float s = (float)sin(phi);
			float x = s * (float)cos(theta);
			float y = (float)cos(phi);
			float z = s * (float)sin(theta);

			sphereVertices.insert(sphereVertices.end(), {x, y, z, x, y, z});
		}
		phi += deltaPhi;

		float dd = 2.0f / (2 * N - 1);
		float y = 1.0f - dd * (1 + i);
    float x = 1, z = dd / 2.0f;
    for (size_t j = 0; j < N - 1; j++)
    {
      cubeVertices.insert(cubeVertices.end(), {x, y, z, 1, 0, 0});
      z += dd;
    }
    cubeVertices.insert(cubeVertices.end(), {x = 1, y, z = 1, 1, 0, 1});
    for (size_t j = 0; j < 2 * N - 2; j++)
    {
      x -= dd;
      cubeVertices.insert(cubeVertices.end(), {x, y, z, 0, 0, 1});
    }
    cubeVertices.insert(cubeVertices.end(), {x = -1, y, z = 1, -1, 0, 1});
    for (size_t j = 0; j < 2 * N - 2; j++)
    {
      z -= dd;
      cubeVertices.insert(cubeVertices.end(), {x, y, z, -1, 0, 0});
    }
    cubeVertices.insert(cubeVertices.end(), {x = -1, y, z = -1, -1, 0, -1});
    for (size_t j = 0; j < 2 * N - 2; j++)
    {
      x += dd;
      cubeVertices.insert(cubeVertices.end(), {x, y, z, 0, 0, -1});
    }
    cubeVertices.insert(cubeVertices.end(), {x = 1, y, z = -1, 1, 0, -1});
    for (size_t j = 0; j < N - 1; j++)
    {
      z += dd;
      cubeVertices.insert(cubeVertices.end(), {x, y, z, 1, 0, 0});
    }
	}

	// bottom
	phi = PI - deltaPhi / 2.0f;
	for (size_t i = 0; i < N; i++)
	{
		size_t numOfDots = 8 * i + 4;

		for (size_t j = 0; j < numOfDots; j++)
		{
			float theta = 2.0 * PI * ((float)j + 0.5) / numOfDots;
			float s = sin(phi);
			float x = s * (float)cos(theta);
			float y = (float)cos(phi);
			float z = s * (float)sin(theta);
			float x1 = -x / y;
			float z1 = -z / y;
			float k = -sqrt(x1 * x1 + z1 * z1) / (std::max(abs(x1), abs(z1)));

			sphereVertices.insert(sphereVertices.end(), {x, y, z, x, y, z});
			cubeVertices.insert(cubeVertices.end(), {k * x / y, -1, k * z / y, 0, -1, 0});
		}

		phi -= deltaPhi;
	}

  float s3 = sqrt(3);
  for (size_t i = 0; i < sphereVertices.size(); i += 6)
  {
    vertices.insert(vertices.end(), {
      sphereVertices[i] * s3, sphereVertices[i + 1] * s3, sphereVertices[i + 2] * s3,
      sphereVertices[i + 3] * s3, sphereVertices[i + 4] * s3, sphereVertices[i + 5] * s3,
      cubeVertices[i], cubeVertices[i + 1], cubeVertices[i + 2],
      cubeVertices[i + 3], cubeVertices[i + 4], cubeVertices[i + 5] 
    });
  }


	program_ = std::make_unique<QOpenGLShaderProgram>(wnd);
	program_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/morth.vs");
	program_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/morth.fs");
	program_->link();

	vao_.create();
	vao_.bind();

	vbo_.create();
	vbo_.bind();
	vbo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
	vbo_.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(GLfloat)));

	ibo_.create();
	ibo_.bind();
	ibo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
	ibo_.allocate(indices.data(), static_cast<int>(indices.size() * sizeof(GLuint)));

	program_->bind();

	size_t offset = 0;
	size_t fullSize = sizeof(GLfloat) * 12;

	// pos1 (location=0)
	program_->enableAttributeArray(0);
	program_->setAttributeBuffer(0, GL_FLOAT, offset, 3, fullSize);
	offset += 3 * sizeof(GLfloat);

	// norm1 (location=1)
	program_->enableAttributeArray(1);
	program_->setAttributeBuffer(1, GL_FLOAT, offset, 3, fullSize);
	offset += 3 * sizeof(GLfloat);

	// pos2 (location=2)
	program_->enableAttributeArray(2);
	program_->setAttributeBuffer(2, GL_FLOAT, offset, 3, fullSize);
	offset += 3 * sizeof(GLfloat);

	// norm2 (location=3)
	program_->enableAttributeArray(3);
	program_->setAttributeBuffer(3, GL_FLOAT, offset, 3, fullSize);

	vertexCount_ = vertices.size();
	indexCount_ = indices.size();

	mvpUniform_ = program_->uniformLocation("mvp");
	timeUniform_ = program_->uniformLocation("time");
  modeUniform_ = program_->uniformLocation("mode");
  lerpUniform_ = program_->uniformLocation("lerp");
  enableManualUniform_ = program_->uniformLocation("enableManual");

	program_->release();
	vao_.release();
	vbo_.release();
	ibo_.release();
}

void Morth::render(Window * const wnd, const QMatrix4x4 & viewProjection)
{
	program_->bind();
	vao_.bind();

	QMatrix4x4 scale;
	scale.translate(0, 10, 20);
	scale.scale(5.0f);
	program_->setUniformValue(mvpUniform_, viewProjection * scale);
	program_->setUniformValue(timeUniform_, (float)clock() / CLOCKS_PER_SEC);
  program_->setUniformValue(modeUniform_, wnd->mode_);
  program_->setUniformValue(lerpUniform_, wnd->interpolation_);
  program_->setUniformValue(enableManualUniform_, wnd->enableManual_);

	// Activate texture unit and bind texture
	wnd->glActiveTexture(GL_TEXTURE0);

	// wnd->glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
	wnd->glDrawArrays(GL_POINTS, 0, vertexCount_);

	vao_.release();
	program_->release();
}

void Morth::release()
{
	program_.reset();
}
