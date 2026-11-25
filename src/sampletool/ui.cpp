#include "ui.h"

#include "model_view_window.h"
#include "particle_view_window.h"
#include <QDialog>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWindow>

// Model view window
// Draws a studio model to the window

#define MODEL_NAME "models/barney.mdl"
#define PARTICLE_NAME  "fire_medium_01"

// Main Window
// Embeds two matsyswindows into the same window, using container widgers

CMainWindow::CMainWindow( QWidget *pParent ) : QWidget( pParent )
{
	this->setWindowTitle( tr( "Sample Tool" ) );
	auto pViewerLayout = new QVBoxLayout( this );

	// Barney window
	m_model1 = new CModelViewWindow( 0, MODEL_NAME );
	QWidget *container1 = QWidget::createWindowContainer( m_model1, this );
	container1->setMinimumSize( 256, 256 );
	container1->setFocusPolicy( Qt::TabFocus );
	pViewerLayout->addWidget( container1 );

	// Anim sequence picker
	m_sequencePicker = new QComboBox();
	pViewerLayout->addWidget( m_sequencePicker );

	// Cart window
	m_model2 = new CParticleViewWindow( 0, PARTICLE_NAME );

	QWidget *container2 = QWidget::createWindowContainer( m_model2, this );
	container2->setMinimumSize( 256, 256 );
	container2->setFocusPolicy( Qt::TabFocus );
	pViewerLayout->addWidget( container2 );

	auto label = new QLabel( this );
	label->setText( "hello!" );
	pViewerLayout->addWidget( label );

	connect( m_sequencePicker, QOverload<int>::of( &QComboBox::activated ),
			 [=]( int index )
			 {
				 m_model1->model->sequence = index;
			 } );

	this->setLayout( pViewerLayout );

	// Populate the picker
	m_sequencePicker->clear();
	for ( int i = 0; i < m_model1->model->studiohdr->GetNumSeq(); i++ )
	{
		auto &s = m_model1->model->studiohdr->pSeqdesc( i );
		m_sequencePicker->addItem( s.pszLabel() );
	}
}
