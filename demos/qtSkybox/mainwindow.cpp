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

  meshSwitch = new QCheckBox( tr( "&Reflect/Refract" ) );
  meshSwitch->setFocusPolicy( Qt::NoFocus ); // do not interfere with vulkanWindow's keyboard input

  pauseButton = new QPushButton( tr( "&Pause" ) );
  pauseButton->setFocusPolicy( Qt::NoFocus );

  quitButton = new QPushButton( tr( "&Quit" ) );
  quitButton->setFocusPolicy( Qt::NoFocus );

  comboModels = new QComboBox( );
  comboModels->addItems( QStringList( )
    << QString( "wolf" )
    << QString( "bunny" )
    << QString( "dragon" )
    << QString( "teapot" )
  );

  connect( meshSwitch, &QCheckBox::clicked, vw, &VulkanWindow::styleSwitched );
  connect( comboModels, static_cast<void ( QComboBox::* )( const QString& )>(&QComboBox::currentIndexChanged),
    vw, &VulkanWindow::changeModel );
  connect( pauseButton, &QPushButton::clicked, vw, &VulkanWindow::togglePaused );
  connect( quitButton, &QPushButton::clicked, qApp, &QCoreApplication::quit );

  meshSwitch->setChecked( true );

  comboModels->setCurrentIndex( 0 );

  QGridLayout* layout = new QGridLayout( );

  layout->addWidget( meshSwitch, 0, 2 );
  layout->addWidget( comboModels, 1, 2 );
  layout->addWidget( pauseButton, 2, 2 );
  layout->addWidget( quitButton, 3, 2 );
  layout->addWidget( wrapper, 0, 0, 7, 2 );
  setLayout( layout );
}
