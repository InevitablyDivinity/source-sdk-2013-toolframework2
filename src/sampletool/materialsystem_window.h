#pragma once
#include <QWindow>
#include <QImage>
#include <QLabel>

class ITexture;

class CMaterialSystemWindow : public QWindow
{
	Q_OBJECT;
public:

	CMaterialSystemWindow( QWindow *pParent );

	inline void *get_native_handle() { return native_handle; }

protected:
	void* native_handle;
};

class CMaterialSystemSurface : public CMaterialSystemWindow
{
	Q_OBJECT;
public:
	CMaterialSystemSurface( QWindow *pParent );

	// Paints to the window
	virtual void paint();

protected:
	virtual void paint_begin();
	virtual void paint_end();
	void activate_window_render_context();
	void deactivate_window_render_context();

	bool event( QEvent *ev ) override;
	void resizeEvent( QResizeEvent *ev ) override;
	void paintEvent( QPaintEvent *ev ) override;

	float frame_time;
};