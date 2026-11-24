#pragma once
#include <QComboBox>
#include <QWidget>

class CModelViewWindow;
class CParticleViewWindow;

class CMainWindow : public QWidget
{
	Q_OBJECT;

public:
	CMainWindow( QWidget *pParent );

	CModelViewWindow *m_model1;
	CParticleViewWindow *m_model2;
	QComboBox *m_sequencePicker;
};
