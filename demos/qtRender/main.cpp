#include <iostream>

#include <qtLava/qtLava.h>

#include <QApplication>
#include <QVulkanInstance>

#include "mainwindow.h"

int main( int argc, char** argv )
{
  QApplication app( argc, argv );

  /**
  VulkanWindow vw( engine );
  vw.setVkInstance( engine->GetVkInstance( ) );
  vw.setWidth( 640 );
  vw.setHeight( 480 );
  vw.show( );
  /**/

  /**/
  VulkanWindow* vw = new VulkanWindow( );
  std::vector< int > supportedSamples = vw->supportedSampleCounts( );
  vw->setSampleCountFlagBits( supportedSamples.at( 0 ) );
  MainWindow w ( vw );
  w.show( );

  /**/
  return app.exec( );
}