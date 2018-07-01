#include <iostream>

#include <qtLava/qtLava.h>

#include <QApplication>

#include "mainwindow.h"
#include "vulkanwindow.h"

int main( int argc, char** argv )
{
  QApplication app( argc, argv );

  VulkanWindow* vw = new VulkanWindow( );
  MainWindow w ( vw );
  w.resize( 1024, 768 );
  w.show( );

  return app.exec( );
}