#include <iostream>

#include <qtLava/qtLava.h>

#include <QApplication>
#include <QVulkanInstance>

#include "vulkanWindow.h"

int main( int argc, char** argv )
{
  QApplication app( argc, argv );

  VulkanWindow vw;
  vw.setWidth( 640 );
  vw.setHeight( 480 );
  vw.show( );

  return app.exec( );
}