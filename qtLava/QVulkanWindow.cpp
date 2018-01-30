#include "QVulkanWindowRenderer.h"
#include "QVulkanWindow.h"

namespace lava
{
  QVulkanWindowRenderer::~QVulkanWindowRenderer( void )
  {
  }
  void QVulkanWindowRenderer::initResources( void )
  {
  }
  void QVulkanWindowRenderer::initSwapChainResources( void )
  {
  }
  void QVulkanWindowRenderer::releaseSwapChainResources( void )
  {
  }
  void QVulkanWindowRenderer::releaseResources( void )
  {
  }
  QVulkanWindow::QVulkanWindow( QWindow * parent )
    : QWindow( parent )
  {
  }
  QVulkanWindow::~QVulkanWindow( void )
  {
  }
  void QVulkanWindow::beginFrame( void )
  {
  }
  void QVulkanWindow::endFrame( void )
  {
  }
  std::shared_ptr<CommandBuffer> QVulkanWindow::currentCommandBuffer( void ) const
  {
    return imageRes[ 0 /* TODO */].commandBuffer;
  }
  std::shared_ptr<PhysicalDevice> QVulkanWindow::physicalDevice( void ) const
  {
    return _physicalDevice;
  }
  const vk::PhysicalDeviceProperties QVulkanWindow::physicalDeviceProperties( void ) const
  {
    return physicalDevice( )->getDeviceProperties( );
  }
  std::shared_ptr<Device> QVulkanWindow::device( void ) const
  {
    return _device;
  }
  std::shared_ptr<Queue> QVulkanWindow::gfxQueue( void ) const
  {
    return _gfxQueue;
  }
  std::shared_ptr<CommandPool> QVulkanWindow::gfxCommandPool( void ) const
  {
    return _cmdPool;
  }
  std::shared_ptr<RenderPass> QVulkanWindow::defaultRenderPass( void ) const
  {
    return _renderPass;
  }
  vk::Format QVulkanWindow::colorFormat( void ) const
  {
    return _colorFormat;
  }
  vk::Format QVulkanWindow::depthStencilFormat( void ) const
  {
    return _dsFormat;
  }
  QVulkanWindowRenderer * QVulkanWindow::createRenderer( void )
  {
    return nullptr;
  }
}
