#include "particle_view_window.h"
#include "particles/particles.h"
#include "tier2/camerautils.h"
#include "toolframework2/particle_parse.h"
#include <QDialog>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QWheelEvent>

CParticleViewWindow::CParticleViewWindow( QWindow *pParent, const char *particlePath ) : CMaterialSystemSurface( pParent ), m_particlePath( particlePath )
{
	PrecacheParticleSystem( m_particlePath );
	CParticleCollection *pParticle = g_pParticleSystemMgr->CreateParticleCollection( m_particlePath );
	m_pParticleSystem = pParticle;
	Vector mins, maxs;
	m_pParticleSystem->GetBounds( &mins, &maxs );
	m_campos = ( mins + maxs ) * 0.5f;
	m_pParticleSystem->StartEmission();
}

void CParticleViewWindow::paint()
{
	paint_begin();
	g_pParticleSystemMgr->SetLastSimulationTime( Plat_FloatTime() );
	m_pParticleSystem->Simulate( frame_time, false );

	int vx = this->x();
	int vy = this->y();
	int vw = this->width();
	int vh = this->height();

	CMatRenderContextPtr ctx( g_pMaterialSystem );

	int viewx, viewy, vieww, viewh;
	ctx->GetViewport( viewx, viewy, vieww, viewh );

	// Make us a nice camera
	VMatrix viewMatrix;
	VMatrix projMatrix;

	Vector forward;
	AngleVectors( m_camang, &forward );
	Camera_t cam = { m_campos + forward * -m_camzoom, m_camang, 65, 1.0f, 20000.0f };
	ComputeViewMatrix( &viewMatrix, cam );
	ComputeProjectionMatrix( &projMatrix, cam, vw, vh );

	// Apply the camera
	ctx->MatrixMode( MATERIAL_PROJECTION );
	ctx->LoadMatrix( projMatrix );
	ctx->MatrixMode( MATERIAL_VIEW );
	ctx->LoadMatrix( viewMatrix );

	ctx->MatrixMode( MATERIAL_MODEL );
	ctx->PushMatrix();
	ctx->LoadIdentity();

	// Begin 3D Rendering
	static QAngle ang = { 0, 0, 0 };
	static Vector pos = { 0, 0, 0 };
	m_pParticleSystem->Render( ctx );

	if ( m_pParticleSystem->IsFinished() )
	{
		delete m_pParticleSystem;
		m_pParticleSystem = g_pParticleSystemMgr->CreateParticleCollection( m_particlePath );
	}

	paint_end();

	//requestUpdate();
}

void CParticleViewWindow::mousePressEvent( QMouseEvent *ev )
{
	if ( ev->button() & Qt::MouseButton::LeftButton )
	{
		m_rotateStartCamAng = m_camang;
		m_dragStartPos = ev->pos();
		m_mouseMode = MouseMode::ROTATE;
		requestUpdate();
		ev->accept();
	}
	else if ( ev->button() & Qt::MouseButton::MiddleButton )
	{
		m_moveStartCamPos = m_campos;
		m_dragStartPos = ev->pos();
		m_mouseMode = MouseMode::MOVE;
		requestUpdate();
		ev->accept();
	}
	else
	{
		ev->ignore();
	}
}

void CParticleViewWindow::mouseReleaseEvent( QMouseEvent *ev )
{
	QPoint delta = ev->pos() - m_dragStartPos;
	if ( m_mouseMode == MouseMode::ROTATE )
	{
		m_mouseMode = MouseMode::NONE;
		m_camang = m_rotateStartCamAng + QAngle { delta.y() * ( 360.0f / height() ), -delta.x() * ( 360.0f / width() ), 0 };
		requestUpdate();
		ev->accept();
	}
	else if ( m_mouseMode == MouseMode::MOVE )
	{
		m_mouseMode = MouseMode::NONE;
		float dispperzoom = m_camzoom / 80.0f * 64.0f;
		Vector right, up;
		AngleVectors( m_camang, nullptr, &right, &up );
		m_campos = m_moveStartCamPos + right * ( -delta.x() * ( dispperzoom / width() ) ) + up * ( delta.y() * ( dispperzoom / height() ) );
		requestUpdate();
		ev->accept();
	}
	else
	{
		ev->ignore();
	}
}

void CParticleViewWindow::mouseMoveEvent( QMouseEvent *ev )
{
	QPoint delta = ev->pos() - m_dragStartPos;
	if ( m_mouseMode == MouseMode::ROTATE )
	{
		m_camang = m_rotateStartCamAng + QAngle { delta.y() * ( 360.0f / height() ), -delta.x() * ( 360.0f / width() ), 0 };
		requestUpdate();
		ev->accept();
	}
	else if ( m_mouseMode == MouseMode::MOVE )
	{
		float dispperzoom = m_camzoom / 80.0f * 64.0f;
		Vector right, up;
		AngleVectors( m_camang, nullptr, &right, &up );
		m_campos = m_moveStartCamPos + right * ( -delta.x() * ( dispperzoom / width() ) ) + up * ( delta.y() * ( dispperzoom / height() ) );
		requestUpdate();
		ev->accept();
	}
	else
	{
		ev->ignore();
	}
}

void CParticleViewWindow::wheelEvent( QWheelEvent *ev )
{
	// Get the change in wheel angle/steps (normalized for consistency)
	// Use angleDelta().y() and divide by 15 for standard steps (Qt 5.14+)
	const int steps = ev->angleDelta().y() / 15;

	// Determine the zoom factor based on the wheel direction
	const float zoomScale = 1.15;
	const double scaleFactor = steps > 0 ? zoomScale : -zoomScale;

	m_camzoom -= m_camzoom / 100.0f * scaleFactor;
	ev->accept();
	requestUpdate();
}
