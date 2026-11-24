#include "model_view_window.h"
#include "KeyValues.h"
#include "istudiorender.h"
#include "materialsystem/imesh.h"
#include "materialsystem/itexture.h"
#include "tier2/camerautils.h"
#include <QWheelEvent>

CModelViewWindow::CModelViewWindow( QWindow *pParent, const char *modelPath ) : CMaterialSystemSurface( pParent ), m_modelPath( modelPath )
{
	// White out our cubemap and lightmap, as we don't have either
	ITexture *whiteTexture = g_pMaterialSystem->FindTexture( "white", NULL, true );
	whiteTexture->AddRef();
	g_pMaterialSystem->GetRenderContext()->BindLocalCubemap( whiteTexture );
	g_pMaterialSystem->GetRenderContext()->BindLightmapTexture( whiteTexture );

	// If we don't do this, all models will render black
	int samples = g_pStudioRender->GetNumAmbientLightSamples();
	Vector *m_ambientLightColors = new Vector[samples];
	for ( int i = 0; i < samples; i++ )
		m_ambientLightColors[i] = { 1, 1, 1 };
	g_pStudioRender->SetAmbientLightColors( m_ambientLightColors );

	g_pStudioRender->SetAlphaModulation( 1.0f );

	model = new CStudioModel( m_modelPath );

	m_campos = ( model->studiohdr->hull_min() + model->studiohdr->hull_max() ) * 0.5f;
}

void CModelViewWindow::paint()
{
	paint_begin();

	int vx = this->x();
	int vy = this->y();
	int vw = this->width();
	int vh = this->height();

	CMatRenderContextPtr ctx( g_pMaterialSystem );

	int viewx, viewy, vieww, viewh;
	ctx->GetViewport( viewx, viewy, vieww, viewh );

	Msg( "vw: %d vh: %d\n", vw, vh );
	Msg( "vieww: %d viewh: %d\n", vieww, viewh );

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
	model->m_time = Plat_FloatTime();
	model->Draw( pos, ang );

	paint_end();

	requestUpdate();
}

void CModelViewWindow::mousePressEvent( QMouseEvent *ev )
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

void CModelViewWindow::mouseReleaseEvent( QMouseEvent *ev )
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

void CModelViewWindow::mouseMoveEvent( QMouseEvent *ev )
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

void CModelViewWindow::wheelEvent( QWheelEvent *ev )
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

#include "model_view_window.h.moc"
