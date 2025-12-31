#include "Duck.h"

#include <array>
#include <iostream>

#include <QFile>
#include <QMatrix4x4>

void Duck::render(Window * const wnd, const QMatrix4x4 & viewProjection)
{
	// Bind VAO and shader program
	program_->bind();
	vao_.bind();

	// Update uniform values
	QMatrix4x4 scale;
	// scale.setToIdentity();
	scale.scale(0.1f);
	program_->setUniformValue(mvpUniform_, viewProjection * scale);
	program_->setUniformValue(timeUniform_, (float)clock() / CLOCKS_PER_SEC);
	program_->setUniformValue(userPosUniform_, wnd->userPos_);

	program_->setUniformValue(dotLightAngleUniform_, wnd->dotLightAngle_);
	program_->setUniformValue(dotLightHeightUniform_, wnd->dotLightHeight_);
	program_->setUniformValue(enableDotLightUniform_, wnd->enableDotLight_);
	program_->setUniformValue(spotLightLatitudeUniform_, wnd->spotLightLatitude_);
	program_->setUniformValue(spotLightLongitudeUniform_, wnd->spotLightLongitude_);
	program_->setUniformValue(enableSpotLightUniform_, wnd->enableSpotLight_);

	// Activate texture unit and bind texture
	wnd->glActiveTexture(GL_TEXTURE0);
	texture_->bind();

	// Draw
	if (indexCount_ != 0)
	{
		wnd->glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
	}
	else
	{
		wnd->glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
	}

	// Release VAO and shader program
	texture_->release();
	vao_.release();
	program_->release();
}

void Duck::release()
{
	texture_.reset();
	program_.reset();
}

void Duck::init(Window * const wnd)
{
	program_ = std::make_unique<QOpenGLShaderProgram>(wnd);
	program_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/diffuse.vs");
	program_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/diffuse.fs");
	program_->link();

	texture_ = std::make_unique<QOpenGLTexture>(QImage(":/Textures/Duck.png"));
	texture_->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
	texture_->setWrapMode(QOpenGLTexture::WrapMode::Repeat);

	// load model
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	std::string err, warn;
	std::string filename = ":/Models/Duck.glb";

	QFile modelFile = QFile(QString::fromStdString(filename));
	if (!modelFile.open(QIODevice::ReadOnly))
	{
		return;
	}

	QByteArray modelData = modelFile.readAll();
	modelFile.close();

	if (modelData.isEmpty())
	{
		return;
	}

	bool res = loader.LoadBinaryFromMemory(&model, &err, &warn,
										   reinterpret_cast<const unsigned char *>(modelData.constData()), modelData.size());

	if (!res)
	{
		std::cout << "Failed to load gltf: " << filename << ": " << err << std::endl;
		return;
	}

	std::cout << "Loaded gltf: " << filename << std::endl;

	vao_.create();
	vao_.bind();

	vbo_.create();
	vbo_.bind();

	ibo_.create();

	std::vector<GLfloat> vertices;
	std::vector<GLuint> indices;

	auto & mesh = model.meshes[0];
	auto & primitive = mesh.primitives[0];

	bool hasNormals = primitive.attributes.find("NORMAL") != primitive.attributes.end();
	bool hasTexCoords = primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end();

	int cnt = 3;
	if (hasNormals)
		cnt += 3;
	if (hasTexCoords)
		cnt += 2;

	size_t vertexCount = model.accessors[primitive.attributes.at("POSITION")].count;
	vertices.resize(vertexCount * cnt);

	loadInterleavedData(model, primitive, vertices.data(), cnt, hasNormals);

	if (primitive.indices >= 0)
	{
		loadIndices(model, primitive, indices);
	}

	vbo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
	vbo_.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(GLfloat)));

	if (!indices.empty())
	{
		ibo_.bind();
		ibo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
		ibo_.allocate(indices.data(), static_cast<int>(indices.size() * sizeof(GLuint)));
	}

	program_->bind();

	int fullSize = cnt * sizeof(GLfloat);
	int offset = 0;

	// pos (location=0)
	program_->enableAttributeArray(0);
	program_->setAttributeBuffer(0, GL_FLOAT, offset, 3, fullSize);
	offset += 3 * sizeof(GLfloat);

	// norm (location=1)
	if (hasNormals)
	{
		program_->enableAttributeArray(1);
		program_->setAttributeBuffer(1, GL_FLOAT, offset, 3, fullSize);
		offset += 3 * sizeof(GLfloat);
	}

	// tex (location=2)
	if (hasTexCoords)
	{
		program_->enableAttributeArray(2);
		program_->setAttributeBuffer(2, GL_FLOAT, offset, 2, fullSize);
	}

	vertexCount_ = vertexCount;
	indexCount_ = indices.size();

	mvpUniform_ = program_->uniformLocation("mvp");
	timeUniform_ = program_->uniformLocation("time");
	userPosUniform_ = program_->uniformLocation("userPos");
	dotLightAngleUniform_ = program_->uniformLocation("dotLightAngle");
	dotLightHeightUniform_ = program_->uniformLocation("dotLightHeight");
	enableDotLightUniform_ = program_->uniformLocation("enableDotLight");
	spotLightLatitudeUniform_ = program_->uniformLocation("spotLightLatitude");
	spotLightLongitudeUniform_ = program_->uniformLocation("spotLightLongitude");
	enableSpotLightUniform_ = program_->uniformLocation("enableSpotLight");

	program_->release();
	vao_.release();
	vbo_.release();

	if (!indices.empty())
	{
		ibo_.release();
	}
}

void Duck::loadInterleavedData(const tinygltf::Model & model,
							  const tinygltf::Primitive & primitive,
							  GLfloat * vertices, int cnt, bool hasNormals)
{
	auto & posAccessor = model.accessors[primitive.attributes.at("POSITION")];
	auto & posView = model.bufferViews[posAccessor.bufferView];
	auto & posBuffer = model.buffers[posView.buffer];
	const float * posData = reinterpret_cast<const float *>(
		posBuffer.data.data() + posView.byteOffset + posAccessor.byteOffset);

	// pos
	for (size_t i = 0; i < posAccessor.count; i++)
	{
		vertices[i * cnt + 0] = posData[i * 3 + 0];
		vertices[i * cnt + 1] = posData[i * 3 + 1];
		vertices[i * cnt + 2] = posData[i * 3 + 2];
	}

	// norm
	if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
	{
		auto & normAccessor = model.accessors[primitive.attributes.at("NORMAL")];
		auto & normView = model.bufferViews[normAccessor.bufferView];
		auto & normBuffer = model.buffers[normView.buffer];
		const float * normData = reinterpret_cast<const float *>(
			normBuffer.data.data() + normView.byteOffset + normAccessor.byteOffset);

		for (size_t i = 0; i < normAccessor.count; i++)
		{
			vertices[i * cnt + 3] = normData[i * 3 + 0];
			vertices[i * cnt + 4] = normData[i * 3 + 1];
			vertices[i * cnt + 5] = normData[i * 3 + 2];
		}
	}

	// tex
	if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
	{
		auto & texAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
		auto & texView = model.bufferViews[texAccessor.bufferView];
		auto & texBuffer = model.buffers[texView.buffer];
		const float * texData = reinterpret_cast<const float *>(
			texBuffer.data.data() + texView.byteOffset + texAccessor.byteOffset);

		int texOffset = hasNormals ? 6 : 3;
		for (size_t i = 0; i < texAccessor.count; i++)
		{
			vertices[i * cnt + texOffset + 0] = texData[i * 2 + 0];
			vertices[i * cnt + texOffset + 1] = texData[i * 2 + 1];
		}
	}
}

void Duck::loadIndices(const tinygltf::Model & model,
					  const tinygltf::Primitive & primitive,
					  std::vector<GLuint> & indices)
{
	auto & idxAccessor = model.accessors[primitive.indices];
	auto & idxView = model.bufferViews[idxAccessor.bufferView];
	auto & idxBuffer = model.buffers[idxView.buffer];

	const unsigned char * data = idxBuffer.data.data() + idxView.byteOffset + idxAccessor.byteOffset;

	indices.clear();
	indices.reserve(idxAccessor.count);

	if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
	{
		const unsigned short * idxData = reinterpret_cast<const unsigned short *>(data);
		for (size_t i = 0; i < idxAccessor.count; i++)
		{
			indices.push_back(idxData[i]);
		}
	}
	else if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
	{
		const unsigned int * idxData = reinterpret_cast<const unsigned int *>(data);
		indices.assign(idxData, idxData + idxAccessor.count);
	}
	else if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
	{
		const unsigned char * idxData = reinterpret_cast<const unsigned char *>(data);
		for (size_t i = 0; i < idxAccessor.count; i++)
		{
			indices.push_back(idxData[i]);
		}
	}
	else
	{
		std::cerr << "Unsupported index type: " << idxAccessor.componentType << std::endl;
	}
}
