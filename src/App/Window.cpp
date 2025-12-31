#include "Window.h"

#include <QCheckBox>
#include <QDir>
#include <QGroupBox>
#include <QLabel>
#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QSlider>
#include <QVBoxLayout>

#include <array>
#include <iostream>

#include "Duck.h"
#include "Morth.h"

#include <tinygltf/tiny_gltf.h>

Window::Window() noexcept
{
	const auto formatFPS = [](const auto value) {
		return QString("FPS: %1").arg(QString::number(value));
	};

	auto fps = new QLabel(formatFPS(0), this);
	fps->setStyleSheet("QLabel { color : white; }");

	auto spotLayout = new QHBoxLayout();

	auto spotEnableCheck = new QCheckBox("Spot", this);
	spotEnableCheck->setStyleSheet("QCheckBox { color: white; min-width: 120px; }");
	spotEnableCheck->setChecked(true);
	connect(spotEnableCheck, &QCheckBox::toggled, [this](bool checked) {
		enableSpotLight_ = checked;
		update();
	});

	auto spotLatLabel = new QLabel("Latitude:", this);
	spotLatLabel->setStyleSheet("QLabel { color: white; min-width: 120px; }");

	auto spotLatSlider = new QSlider(Qt::Horizontal, this);
	spotLatSlider->setRange(0, 314);
	spotLatSlider->setValue(57);
	spotLatSlider->setFixedWidth(100);
	connect(spotLatSlider, &QSlider::valueChanged, [this](int value) {
		spotLightLatitude_ = value / 100.0f;
		update();
	});

	auto spotLonLabel = new QLabel("Longitude:", this);
	spotLonLabel->setStyleSheet("QLabel { color: white; min-width: 120px; }");

	auto spotLonSlider = new QSlider(Qt::Horizontal, this);
	spotLonSlider->setRange(0, 628);
	spotLonSlider->setValue(314);
	spotLonSlider->setFixedWidth(100);
	connect(spotLonSlider, &QSlider::valueChanged, [this](int value) {
		spotLightLongitude_ = value / 100.0f;
		update();
	});

	spotLayout->addWidget(spotEnableCheck);
	spotLayout->addWidget(spotLatLabel);
	spotLayout->addWidget(spotLatSlider);
	spotLayout->addWidget(spotLonLabel);
	spotLayout->addWidget(spotLonSlider);
	spotLayout->addStretch();

	auto pointLayout = new QHBoxLayout();

	auto pointEnableCheck = new QCheckBox("Point", this);
	pointEnableCheck->setStyleSheet("QCheckBox { color: white; min-width: 120px; }");
	pointEnableCheck->setChecked(true);
	connect(pointEnableCheck, &QCheckBox::toggled, [this](bool checked) {
		enableDotLight_ = checked;
		update();
	});

	auto pointAngleLabel = new QLabel("Fov angle:", this);
	pointAngleLabel->setStyleSheet("QLabel { color: white; min-width: 120px; }");

	auto pointAngleSlider = new QSlider(Qt::Horizontal, this);
	pointAngleSlider->setRange(10, 314);
	pointAngleSlider->setValue(157);
	pointAngleSlider->setFixedWidth(100);
	connect(pointAngleSlider, &QSlider::valueChanged, [this](int value) {
		dotLightAngle_ = value / 100.0f;
		update();
	});

	auto pointHeightLabel = new QLabel("Height:", this);
	pointHeightLabel->setStyleSheet("QLabel { color: white; min-width: 120px; }");

	auto pointHeightSlider = new QSlider(Qt::Horizontal, this);
	pointHeightSlider->setRange(0, 50);
	pointHeightSlider->setValue(15);
	pointHeightSlider->setFixedWidth(100);
	connect(pointHeightSlider, &QSlider::valueChanged, [this](int value) {
		dotLightHeight_ = value;
		update();
	});

	pointLayout->addWidget(pointEnableCheck);
	pointLayout->addWidget(pointAngleLabel);
	pointLayout->addWidget(pointAngleSlider);
	pointLayout->addWidget(pointHeightLabel);
	pointLayout->addWidget(pointHeightSlider);
	pointLayout->addStretch();

	auto morthingLayout = new QHBoxLayout();

	auto morthingManual = new QCheckBox("Manual lerp:", this);
	morthingManual->setStyleSheet("QCheckBox { color: white; min-width: 120px; }");
	morthingManual->setChecked(false);
	connect(morthingManual, &QCheckBox::toggled, [this](bool checked) {
		enableManual_ = checked;
		update();
	});

	auto morthingModeLabel = new QLabel("Mode:", this);
	morthingModeLabel->setStyleSheet("QLabel { color: white; min-width: 120px; }");

	auto morthingMode = new QSlider(Qt::Horizontal, this);
	morthingMode->setRange(1, 3);
	morthingMode->setValue(1);
	morthingMode->setFixedWidth(100);
	connect(morthingMode, &QSlider::valueChanged, [this](int value) {
		mode_ = value;
		update();
	});

	auto morthingLerpK = new QLabel("Lerp coef:", this);
	morthingLerpK->setStyleSheet("QLabel { color: white; min-width: 120px; }");

	auto morthingInterpolation = new QSlider(Qt::Horizontal, this);
	morthingInterpolation->setRange(-1000, 2000);
	morthingInterpolation->setValue(500);
	morthingInterpolation->setFixedWidth(100);
	connect(morthingInterpolation, &QSlider::valueChanged, [this](int value) {
		interpolation_ = value / 1000.0;
		update();
	});

	morthingLayout->addWidget(morthingManual);
	morthingLayout->addWidget(morthingModeLabel);
	morthingLayout->addWidget(morthingMode);
	morthingLayout->addWidget(morthingLerpK);
	morthingLayout->addWidget(morthingInterpolation);
	morthingLayout->addStretch();

	auto layout = new QVBoxLayout();
	layout->addWidget(fps, 1);
	layout->addLayout(spotLayout);
	layout->addLayout(pointLayout);
	layout->addLayout(morthingLayout);

	setLayout(layout);

	spotLightLatitude_ = 0.57f;
	spotLightLongitude_ = 3.14f;
	enableSpotLight_ = true;

	dotLightAngle_ = 1.57f;
	dotLightHeight_ = 15.0f;
	enableDotLight_ = true;

	mode_ = 1;
	enableManual_ = false;
	interpolation_ = 0.5f;


	timer_.start();
	timerMove_.start();

	connect(this, &Window::updateUI, [=] {
		fps->setText(formatFPS(ui_.fps));
	});

	duck_ = std::make_unique<Duck>();
	morth_ = std::make_unique<Morth>();
}

Window::~Window()
{
	{
		// Free resources with context bounded.
		const auto guard = bindContext();
		duck_->release();
		morth_->release();
	}
}

void Window::onInit()
{
	duck_->init(this);
	morth_->init(this);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(0.30 * 0.3, 0.47 * 0.3, 0.8 * 0.3, 1.0);

	// Clear all FBO buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::onRender()
{
	// update position:
	float dt = timerMove_.restart() / 1000.0f;
	static constexpr float speed = 10.0f;

	userPos_ += QVector3D(userDir_.x(), 0, userDir_.z()).normalized() * dt * speed * moveForward_;
	userPos_ += QVector3D(userRight_.x(), 0, userRight_.z()).normalized() * dt * speed * moveRight_;

	const auto guard = captureMetrics();

	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Calculate MVP matrix
	view_.setToIdentity();
	view_.lookAt(userPos_, userPos_ + userDir_, userUp_);
	const auto vp = projection_ * view_;

	// render all entities:
	duck_->render(this, vp);
	morth_->render(this, vp);

	++frameCount_;

	// Request redraw if animated
	if (animated_)
	{
		update();
	}
}

void Window::onResize(const size_t width, const size_t height)
{
	// Configure viewport
	glViewport(0, 0, static_cast<GLint>(width), static_cast<GLint>(height));

	// Configure matrix
	const auto aspect = static_cast<float>(width) / static_cast<float>(height);
	projection_.setToIdentity();
	projection_.perspective(fov_, aspect, zNear_, zFar_);
}

Window::PerfomanceMetricsGuard::PerfomanceMetricsGuard(std::function<void()> callback)
	: callback_{std::move(callback)}
{
}

Window::PerfomanceMetricsGuard::~PerfomanceMetricsGuard()
{
	if (callback_)
	{
		callback_();
	}
}

auto Window::captureMetrics() -> PerfomanceMetricsGuard
{
	return PerfomanceMetricsGuard{
		[&] {
			if (timer_.elapsed() >= 1000)
			{
				const auto elapsedSeconds = static_cast<float>(timer_.restart()) / 1000.0f;
				ui_.fps = static_cast<size_t>(std::round(frameCount_ / elapsedSeconds));
				frameCount_ = 0;
				emit updateUI();
			}
		}};
}

void Window::mousePressEvent(QMouseEvent * event)
{
	isPressed_ = true;
	lastMousePos_ = event->pos();
}

void Window::mouseMoveEvent(QMouseEvent * event)
{
	if (isPressed_)
	{
		QPoint delta = event->pos() - lastMousePos_;
		lastMousePos_ = event->pos();

		float dx = -(float)delta.x() / 100.0;
		float dy = -(float)delta.y() / 100.0;

		userRight_ = QVector3D::crossProduct(userUp_, userDir_);
		userRight_.normalize();

		QVector3D up = QVector3D::crossProduct(QVector3D::crossProduct(userDir_, userUp_), userDir_);
		if (up.lengthSquared() <= EPSILON_SQUARED)
			up = QVector3D(0, 1, 0);
		else
			up.normalize();

		userDir_ += userRight_ * dx + up * dy;
		userDir_.normalize();
		userRight_ = QVector3D::crossProduct(userDir_, userUp_);
		userRight_.normalize();
	}
}

void Window::mouseReleaseEvent([[maybe_unused]] QMouseEvent * event)
{
	isPressed_ = false;
}

void Window::wheelEvent(QWheelEvent * event)
{
	float delta = event->angleDelta().y() / 500.0f;
	userPos_ += userUp_ * delta;
}

void Window::keyPressEvent(QKeyEvent * event)
{
	int key = event->key();

	switch (key)
	{
		case Qt::Key_W:
			moveForward_ = 1.0f;
			break;
		case Qt::Key_S:
			moveForward_ = -1.0f;
			break;
		case Qt::Key_D:
			moveRight_ = 1.0f;
			break;
		case Qt::Key_A:
			moveRight_ = -1.0f;
			break;
	}
}

void Window::keyReleaseEvent(QKeyEvent * event)
{
	moveForward_ = 0.0f;
	moveRight_ = 0.0f;
}
