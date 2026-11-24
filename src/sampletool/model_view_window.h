#pragma once
#include "materialsystem_window.h"
#include "particle_view_window.h"
#include "toolframework2/studiomodel.h"

class QWheelEvent;
class QMouseEvent;

class CModelViewWindow : public CMaterialSystemSurface
{
	Q_OBJECT;

public:
	CModelViewWindow( QWindow *pParent, const char *modelPath );

	virtual void paint();

	const char *m_modelPath;
	CStudioModel *model;

	QAngle m_camang = { 0, 0, 0 };
	float m_camzoom = 80;
	Vector m_campos = { 0, 0, 0 };

protected:
	enum class MouseMode
	{
		NONE,
		ROTATE,
		MOVE
	};

	virtual void mousePressEvent( QMouseEvent *ev );
	virtual void mouseReleaseEvent( QMouseEvent *ev );
	virtual void mouseMoveEvent( QMouseEvent *ev );
	virtual void wheelEvent( QWheelEvent *ev );

	MouseMode m_mouseMode = MouseMode::NONE;
	QPoint m_dragStartPos = { 0, 0 };
	QAngle m_rotateStartCamAng = { 0, 0, 0 };
	Vector m_moveStartCamPos = { 0, 0, 0 };
};
