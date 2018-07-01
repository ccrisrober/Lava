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

  quitButton = new QPushButton( tr( "&Quit" ) );
  quitButton->setFocusPolicy( Qt::NoFocus );

  comboModels = new QComboBox( );
  comboModels->addItems( QStringList( )
    << QString( "sampler1" )
    << QString( "sampler2" )
    << QString( "sampler3" )
    << QString( "sampler4" )
  );

  connect( comboModels, 
    static_cast<void ( QComboBox::* )( int )>(&QComboBox::currentIndexChanged),
    vw, &VulkanWindow::changeSampler );
  connect( quitButton, &QPushButton::clicked, qApp, &QCoreApplication::quit );

  comboModels->setCurrentIndex( 0 );

  QGridLayout* layout = new QGridLayout( );

  layout->addWidget( comboModels, 0, 2 );
  layout->addWidget( quitButton, 1, 2 );
  layout->addWidget( wrapper, 0, 0, 7, 2 );
  setLayout( layout );
}
