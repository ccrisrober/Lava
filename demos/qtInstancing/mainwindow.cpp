#include "mainwindow.h"

#include "vulkanwindow.h"

#include <QApplication>
#include <QGridLayout>
#include <QtGlobal>

MainWindow::MainWindow( VulkanWindow * vw )
{
  QWidget* wrapper = QWidget::createWindowContainer( vw );
  wrapper->setFocusPolicy( Qt::StrongFocus );
  wrapper->setFocus( );

  slider = new QSlider( );
  slider->setFocusPolicy( Qt::NoFocus ); // do not interfere with vulkanWindow's keyboard input

  slider->setMinimum( 0 );
  slider->setMaximum( 50000 );

  slider->setValue( slider->maximum( ) );

  connect( slider,
    static_cast<void ( QSlider::* )( int )>( &QAbstractSlider::valueChanged ),
    vw, &VulkanWindow::changeNumInstancing );

  quitButton = new QPushButton( tr( "&Quit" ) );
  quitButton->setFocusPolicy( Qt::NoFocus );
  
  connect( quitButton, &QPushButton::clicked, qApp, &QCoreApplication::quit );

  QGridLayout* layout = new QGridLayout( );

  layout->addWidget( slider, 0, 2 );
  layout->addWidget( quitButton, 1, 2 );
  layout->addWidget( wrapper, 0, 0, 7, 2 );
  setLayout( layout );
}
