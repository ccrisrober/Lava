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

  sliderAmount = new QSlider( );
  sliderAmount->setFocusPolicy( Qt::NoFocus ); // do not interfere with vulkanWindow's keyboard input

  sliderAmount->setMinimum( -100 );
  sliderAmount->setMaximum( 100 );

  sliderAmount->setValue( 50 );

  connect( sliderAmount,
    static_cast<void ( QSlider::* )( int )>( &QAbstractSlider::valueChanged ),
    vw, &VulkanWindow::changeAmount );


  sliderTessLevel = new QSlider( );
  sliderTessLevel->setFocusPolicy( Qt::NoFocus ); // do not interfere with vulkanWindow's keyboard input

  sliderTessLevel->setMinimum( 0 );
  sliderTessLevel->setMaximum( 1000 );

  sliderTessLevel->setValue( 500 );

  connect( sliderTessLevel,
    static_cast<void ( QSlider::* )( int )>( &QAbstractSlider::valueChanged ),
    vw, &VulkanWindow::changeTessLevel );

  quitButton = new QPushButton( tr( "&Quit" ) );
  quitButton->setFocusPolicy( Qt::NoFocus );
  
  connect( quitButton, &QPushButton::clicked, qApp, &QCoreApplication::quit );

  enableWire = new QCheckBox( tr( "&Enable wireframe" ) );
  enableWire->setFocusPolicy( Qt::NoFocus ); // do not interfere with vulkanWindow's keyboard input

  enableWire->setChecked( true );

  connect( enableWire, &QCheckBox::clicked, vw, &VulkanWindow::enableWireframe );

  QGridLayout* layout = new QGridLayout( );

  layout->addWidget( sliderAmount, 0, 2 );
  layout->addWidget( sliderTessLevel, 1, 2 );
  layout->addWidget( enableWire, 2, 2 );
  layout->addWidget( quitButton, 3, 2 );
  layout->addWidget( wrapper, 0, 0, 7, 2 );
  setLayout( layout );
}
