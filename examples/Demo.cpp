#include <lavaEngine/lavaEngine.h>

#include <iostream>

std::ostream &operator<< ( std::ostream &out, const glm::mat4 &mat ) {
  out << "{" << "\n"
    << mat[ 0 ][ 0 ] << ", " << mat[ 1 ][ 0 ] << ", " << mat[ 2 ][ 0 ] << ", " << mat[ 3 ][ 0 ] << ",\n"
    << mat[ 0 ][ 1 ] << ", " << mat[ 1 ][ 1 ] << ", " << mat[ 2 ][ 1 ] << ", " << mat[ 3 ][ 1 ] << ",\n"
    << mat[ 0 ][ 2 ] << ", " << mat[ 1 ][ 2 ] << ", " << mat[ 2 ][ 2 ] << ", " << mat[ 3 ][ 2 ] << ",\n"
    << mat[ 0 ][ 3 ] << ", " << mat[ 1 ][ 3 ] << ", " << mat[ 2 ][ 3 ] << ", " << mat[ 3 ][ 3 ] << ",\n"
    << "}";

  return out;
}

lava::engine::Geometry* generateGeom( const lava::engine::Color& c )
{
  auto geom = new lava::engine::Geometry( );
  geom->addPrimitive( std::make_shared<lava::engine::Primitive>( ) );

  return geom;
}

#include <random>

// Returns random values uniformly distributed in the range [a, b]
float _random( )
{
  return static_cast <float> ( rand( ) ) / static_cast <float> ( RAND_MAX );
}

lava::engine::Group* addCube( void )
{
  float cubeSize = std::ceil( _random( ) * 3.0f );
  auto cubeGeometry = generateGeom( glm::vec3( 1.0f, 0.0f, 0.0f ) );
  cubeGeometry->scale( glm::vec3( cubeSize, cubeSize, cubeSize ) );
  auto cube = new lava::engine::Group( "cube" );
  cube->addChild( cubeGeometry );

  glm::vec3 pos = cube->getAbsolutePosition( );
  pos.x = -30.0f + std::round( _random( ) * 100.0f );
  pos.y = std::round( _random( ) * 5 );
  pos.z = -20.0f + std::round( _random( ) * 100.0f );

  cube->setPosition( pos );

  return cube;
}

lava::engine::Group* createScene( void )
{
  auto scene = new lava::engine::Group( "scene" );

  auto camera = new lava::engine::Camera( 75.0f, 500 / 500, 0.03f, 1000.0f );
  camera->translate( glm::vec3( 0.0f, 10.0f, 50.0f ) );

  //camera->addComponent( new mb::FreeCameraComponent( ) );
  scene->addChild( camera );

  for ( int i = 0; i < 185; ++i )
  {
    scene->addChild( addCube( ) );
  }

  return scene;
}

int main( )
{
  /*auto parent = new lava::engine::Group( "Parent" );
  auto leftChild = new lava::engine::Node( "LeftChild" );
  auto rightChild = new lava::engine::Node( "RightChild" );

  parent->addChild( leftChild );
  parent->addChild( rightChild );

  parent->translate( glm::vec3( -2.5f, 0.0f, 0.0f ) );
  glm::mat4 transform = leftChild->getTransform( );
  std::cout << "ORIGINAL TRANSFORM : " << transform << std::endl;

  //parent->scale( glm::vec3( 2.5f, 1.5f, 0.5f ) );
  //transform = parent->getTransform( );
  //std::cout << "SCALE TRANSFORM : " << transform << std::endl;


  transform = leftChild->getTransform( );
  std::cout << "LEFT TRANSFORM : " << transform << std::endl;

  leftChild->translate( glm::vec3( 2.5f, 0.0f, 0.0f ), lava::engine::Node::TransformSpace::World );
  transform = leftChild->getTransform( );
  std::cout << "LEFT 2 TRANSFORM : " << transform << std::endl;*/

  auto scene = createScene( );
  scene->getTransform( );

  lava::engine::FetchCameras fetchCameras;
  scene->perform( fetchCameras );
  std::vector<lava::engine::Camera*> cameras;
  fetchCameras.forEachCameras( [ & ]( lava::engine::Camera* c )
  {
    if ( lava::engine::Camera::getMainCamera( ) == nullptr || c->isMainCamera( ) )
    {
      lava::engine::Camera::setMainCamera( c );
    }
    cameras.push_back( c );
  } );

  std::vector< std::shared_ptr< lava::engine::BatchQueue > > bqCollection;

  for ( auto c : cameras )
  {
    if ( c != nullptr && c->isEnabled( ) )
    {
      auto bq = std::make_shared<lava::engine::BatchQueue>( );
      lava::engine::ComputeBatchQueue cbq( c, bq );
      scene->perform( cbq );
      bqCollection.push_back( bq );
    }
  }

  auto solidRenderables = bqCollection[ 0 ]->renderables( lava::engine::BatchQueue::RenderableType::OPAQUE );
  for ( const auto& r : solidRenderables )
  {
    std::cout << r.modelTransform << std::endl;
  }
  return 0;
}