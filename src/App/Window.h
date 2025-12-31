#pragma once

#include <Base/GLWidget.hpp>

#include <QDir>
#include <QElapsedTimer>
#include <QLabel>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QScreen>
#include <QVBoxLayout>

#include <functional>
#include <memory>

class Duck;
class Morth;

class Window final : public fgl::GLWidget
{
	Q_OBJECT
public:
	Window() noexcept;
	~Window() override;

public:// fgl::GLWidget
	void onInit() override;
	void onRender() override;
	void onResize(size_t width, size_t height) override;

private:
	class PerfomanceMetricsGuard final
	{
	public:
		explicit PerfomanceMetricsGuard(std::function<void()> callback);
		~PerfomanceMetricsGuard();

		PerfomanceMetricsGuard(const PerfomanceMetricsGuard &) = delete;
		PerfomanceMetricsGuard(PerfomanceMetricsGuard &&) = delete;

		PerfomanceMetricsGuard & operator=(const PerfomanceMetricsGuard &) = delete;
		PerfomanceMetricsGuard & operator=(PerfomanceMetricsGuard &&) = delete;

	private:
		std::function<void()> callback_;
	};

private:
	[[nodiscard]] PerfomanceMetricsGuard captureMetrics();

signals:
	void updateUI();

private:
	QMatrix4x4 view_;
	QMatrix4x4 projection_;

	QElapsedTimer timer_;
	QElapsedTimer timerMove_;
	size_t frameCount_ = 0;

	struct {
		size_t fps = 0;
	} ui_;

	bool animated_ = true;

	bool isPressed_ = false;
	QPoint lastMousePos_;

private:
	void mousePressEvent(QMouseEvent *) override;
	void mouseMoveEvent(QMouseEvent *) override;
	void mouseReleaseEvent(QMouseEvent *) override;
	void wheelEvent(QWheelEvent *) override;

private:
	void keyPressEvent(QKeyEvent *) override;
	void keyReleaseEvent(QKeyEvent *) override;
	float moveForward_ = 0.0f;
	float moveRight_ = 0.0f;

private:
	static constexpr float EPSILON = 1.0e-4f;
	static constexpr float EPSILON_SQUARED = EPSILON * EPSILON;

	// user
	float zNear_ = 0.1f;
	float zFar_ = 100.0f;
	float fov_ = 60.0f;

public:
	QVector3D userPos_ = QVector3D(25, 10, -10);
	QVector3D userDir_ = QVector3D(-20, -5, 10).normalized();
	QVector3D userUp_ = QVector3D(0, 1, 0);
	QVector3D userRight_ = QVector3D::crossProduct(userDir_, userUp_).normalized();

public:
	float spotLightLatitude_;
  float spotLightLongitude_;
	bool enableSpotLight_;
	float dotLightHeight_;
	float dotLightAngle_;
	bool enableDotLight_;

	int mode_;
	bool enableManual_;
	float interpolation_;

private:
	std::unique_ptr<Duck> duck_;
	std::unique_ptr<Morth> morth_;
};
