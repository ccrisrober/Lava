/**
 * Copyright (c) 2017 - 2018, Pompeii
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#include <iostream>

#include <glfwPompeii/glfwPompeii.h>
using namespace pompeii;

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
  }

  virtual void nextFrame( void )
  {
    auto cmd = _window->currentCommandBuffer( );

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast< std::chrono::milliseconds >(
      currentTime - startTime ).count( ) / 1000.0f;

    float _red = sin( time ) * 0.5f + 0.5f;
    float _blue = cos( time ) * 0.5f + 0.5f;

    std::array<vk::ClearValue, 2 > clearValues;
    clearValues[ 0 ] = pompeii::utils::getClearValueColor( _red, 0.0f, _blue );
    clearValues[ 1 ] = pompeii::utils::getClearValueDepth( );

    vk::Extent2D extent = _window->swapchainImageSize( );

    cmd->beginRenderPass( _window->renderPass( ),
      _window->framebuffer( ),
      vk::Rect2D( { 0, 0 }, extent ), clearValues,
      vk::SubpassContents::eInline );

    cmd->endRenderPass( );

    _window->frameReady( );
  }
};

class VulkanWindow : public glfw::VulkanWindow
{
public:
  explicit VulkanWindow( int width, int height,
    const std::string& title, bool enableLayers )
    : glfw::VulkanWindow( width, height, title, enableLayers )
  {
  }
  virtual glfw::VulkanWindowRenderer* createRenderer( void ) override
  {
    return new MainWindowRenderer( this );
  }

  virtual void setupRenderPass( void ) override
  {
    // First attachment is the color attachment - clear at the beginning of the
    // renderpass and transition layout to PRESENT_SRC_KHR at the end of
    // renderpass
    std::array<vk::AttachmentDescription, 2> attachments;
    attachments[ 0 ].format = colorFormat( );
    attachments[ 0 ].samples = vk::SampleCountFlagBits::e1;
    attachments[ 0 ].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[ 0 ].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[ 0 ].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[ 0 ].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[ 0 ].initialLayout = vk::ImageLayout::eUndefined;
    attachments[ 0 ].finalLayout = vk::ImageLayout::ePresentSrcKHR;

    // Second attachment is input attachment.  Once cleared it should have
    // width*height yellow pixels.  Doing a subpassLoad in the fragment shader
    // should give the shader the color at the fragments x,y location
    // from the input attachment
    attachments[ 1 ].format = colorFormat( );
    attachments[ 1 ].samples = vk::SampleCountFlagBits::e1;
    attachments[ 1 ].loadOp = vk::AttachmentLoadOp::eLoad;
    attachments[ 1 ].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[ 1 ].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[ 1 ].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[ 1 ].initialLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    attachments[ 1 ].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
  
    vk::AttachmentReference colorRef;
    colorRef.attachment = 0;
    colorRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference inputRef;
    inputRef.attachment = 1;
    inputRef.layout = vk::ImageLayout::eShaderReadOnlyOptimal;

    vk::SubpassDescription subpass;
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.inputAttachmentCount = 1;
    subpass.pInputAttachments = &inputRef;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.pResolveAttachments = nullptr;
    subpass.pDepthStencilAttachment = nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;

    _renderPass = device( )->createRenderPass( attachments, subpass, { } );
  }

  virtual void setupFramebuffer( void ) override
  {

  }
};


int main( int, char** )
{
  VulkanWindow app( 500, 500, "GLFWRenderer", true );
  app.show( );
  return EXIT_SUCCESS;
}