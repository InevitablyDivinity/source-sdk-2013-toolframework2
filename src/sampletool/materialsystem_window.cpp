#include "materialsystem_window.h"

#include <QWindow>
#include <QPaintEvent>

// Source tends to do funny stuff with defines. Let's just include it last just to be safe.
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/itexture.h"
#include "materialsystem/materialsystem_config.h"
#include "tier2/tier2.h"

#ifdef USE_SDL
#include <SDL2/SDL.h>
#endif

CMaterialSystemWindow::CMaterialSystemWindow( QWindow *pParent ) : QWindow( pParent )
{
#ifdef USE_SDL
	create();
	native_handle = SDL_CreateWindowFrom((void*)winId());
#else
	native_handle = (void*)winId();
#endif
}

CMaterialSystemSurface::CMaterialSystemSurface( QWindow *pParent ) : CMaterialSystemWindow( pParent )
{
	frame_time = 0.0f;
}

void CMaterialSystemSurface::activate_window_render_context()
{
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	const int vx = this->x();
	const int vy = this->y();
	const int vw = this->width();
	const int vh = this->height();

	g_pMaterialSystem->SetView( native_handle );
	pRenderContext->ClearColor3ub(0x30, 0x30, 0x30);
	pRenderContext->ClearBuffers(true, true);
	pRenderContext->Viewport(vx, vy, vw, vh);
}

void CMaterialSystemSurface::deactivate_window_render_context()
{
}

void CMaterialSystemSurface::paint_begin()
{
	activate_window_render_context();
	g_pMaterialSystem->BeginFrame( 0.0f );
}

void CMaterialSystemSurface::paint_end()
{
	deactivate_window_render_context();

	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	pRenderContext->Flush();
	g_pMaterialSystem->EndFrame();
	g_pMaterialSystem->SwapBuffers();
	frame_time = Plat_FloatTime() - frame_time;
}

void CMaterialSystemSurface::paint()
{
	paint_begin();

	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	pRenderContext->ClearColor3ub( 0xFF, 0x00, 0xFF );
	pRenderContext->ClearBuffers( true, false );

	paint_end();
}

void CMaterialSystemSurface::paintEvent( QPaintEvent *ev )
{
	paint();
	ev->accept();
}

bool CMaterialSystemSurface::event( QEvent *ev )
{
	if ( ev->type() == QEvent::UpdateRequest )
	{
		paint();
		return true;
	}

	return QWindow::event( ev );
}

void CMaterialSystemSurface::resizeEvent( QResizeEvent *ev )
{
	requestUpdate();
}
