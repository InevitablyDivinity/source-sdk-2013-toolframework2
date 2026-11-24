#pragma once
#include "materialsystem_window.h"
#include "mathlib/vector.h"

class CParticleCollection;

class CParticleViewWindow : public CMaterialSystemSurface
{
	Q_OBJECT;
public:

	CParticleViewWindow(QWindow* pParent, const char* particlePath);

	virtual void paint();

	const char* m_particlePath;

	QAngle m_camang = { 0,0,0 };
	float m_camzoom = 80;
	Vector m_campos = { 0,0,0 };

protected:

	enum class MouseMode
	{
		NONE,
		ROTATE,
		MOVE
	};

	virtual void mousePressEvent(QMouseEvent* ev);
	virtual void mouseReleaseEvent(QMouseEvent* ev);
	virtual void mouseMoveEvent(QMouseEvent* ev);
	virtual void wheelEvent(QWheelEvent* ev);

	MouseMode m_mouseMode = MouseMode::NONE;
	QPoint m_dragStartPos = { 0,0 };
	QAngle m_rotateStartCamAng = { 0,0,0 };
	Vector m_moveStartCamPos = { 0,0,0 };
	CParticleCollection *m_pParticleSystem;
};