#include "VulkanDrawable.h"
#include <assert.h>

namespace lava
{
  VulkanDrawable::VulkanDrawable( VulkanRenderer* parent ) {
    memset( &VertexBuffer, 0, sizeof( VertexBuffer ) );
    rendererObj = parent;
    // Prepare the semaphore create info data structure
    VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo;
    presentCompleteSemaphoreCreateInfo.sType =
      VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    presentCompleteSemaphoreCreateInfo.pNext = NULL;
    presentCompleteSemaphoreCreateInfo.flags = 0;
    VkSemaphoreCreateInfo drawingCompleteSemaphoreCreateInfo;
    drawingCompleteSemaphoreCreateInfo.sType =
      VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    drawingCompleteSemaphoreCreateInfo.pNext = NULL;
    drawingCompleteSemaphoreCreateInfo.flags = 0;
    VulkanDevice* deviceObj = VulkanApplication::GetInstance( )->
      deviceObj;
    vkCreateSemaphore( deviceObj->device,
      &presentCompleteSemaphoreCreateInfo, NULL,
      &presentCompleteSemaphore );
    vkCreateSemaphore( deviceObj->device,
      &drawingCompleteSemaphoreCreateInfo, NULL,
      &drawingCompleteSemaphore );
  }  void VulkanDrawable::destroySynchronizationObjects( )
  {
    VulkanApplication* appObj = VulkanApplication::GetInstance( );
    VulkanDevice* deviceObj = appObj->deviceObj;
    vkDestroySemaphore( deviceObj->device,
      presentCompleteSemaphore, NULL );
    vkDestroySemaphore( deviceObj->device,
      drawingCompleteSemaphore, NULL );
  }
  void VulkanDrawable::recordCommandBuffer( int currentImage, 
    VkCommandBuffer* cmdDraw )
  {
    // Specify the clear color value
    VkClearValue clearValues[ 2 ];
    clearValues[ 0 ].color.float32[ 0 ] = 0.0f;
    clearValues[ 0 ].color.float32[ 1 ] = 0.0f;
    clearValues[ 0 ].color.float32[ 2 ] = 0.0f;
    clearValues[ 0 ].color.float32[ 3 ] = 0.0f;
    // Specify the depth/stencil clear value
    clearValues[ 1 ].depthStencil.depth = 1.0f;
    clearValues[ 1 ].depthStencil.stencil = 0;
    VkRenderPassBeginInfo renderPassBegin;
    renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBegin.pNext = nullptr;
    //renderPassBegin.renderPass = rendererObj->renderPass;
    //renderPassBegin.framebuffer = rendererObj->framebuffers[ currentImage ];

    renderPassBegin.renderArea.offset = { 0, 0 };
    //renderPassBegin.renderArea.extent = { rendererObj->width, rendererObj->height };
    renderPassBegin.clearValueCount = 2;
    renderPassBegin.pClearValues = clearValues;

    // Start recording the render pass instance
    vkCmdBeginRenderPass( *cmdDraw, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE );
      // Bound the command buffer with the graphics pipeline
      vkCmdBindPipeline( *cmdDraw, VK_PIPELINE_BIND_POINT_GRAPHICS,
        *pipeline );
      // Bound the command buffer with the graphics pipeline
      const VkDeviceSize offsets[ 1 ] = { 0 };
      vkCmdBindVertexBuffers( *cmdDraw, 0, 1, &VertexBuffer.buf,
        offsets );
      // Define the dynamic viewport here
      initViewports( cmdDraw );
      // Define the scissoring
      initScissors( cmdDraw );
      // Issue the draw command with 3 vertex, 1 instance starting
      // from first vertex
      vkCmdDraw( *cmdDraw, 3, 1, 0, 0 );
    // End of render pass instance recording
    vkCmdEndRenderPass( *cmdDraw );
  }
  void VulkanDrawable::initViewports( VkCommandBuffer* cmd )
  {
    viewport.height = ( float ) rendererObj->height;
    viewport.width = ( float ) rendererObj->width;
    viewport.minDepth = ( float ) 0.0f;
    viewport.maxDepth = ( float ) 1.0f;
    viewport.x = 0;
    viewport.y = 0;
    vkCmdSetViewport( *cmd, 0, 1, &viewport );
  }
  void VulkanDrawable::initScissors( VkCommandBuffer* cmd )
  {
    scissor.extent.width = rendererObj->width;
    scissor.extent.height = rendererObj->height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vkCmdSetScissor( *cmd, 0, 1, &scissor );
  }
  void VulkanDrawable::render( )
  {
    VulkanDevice* deviceObj = rendererObj->getDevice( );
    VulkanSwapChain* swapChainObj = rendererObj->getSwapChain( );
    uint32_t&currentColorImage = swapChainObj->
      scPublicVars.currentColorBuffer;
    VkSwapchainKHR& swapChain = swapChainObj->
      scPublicVars.swapChain;
    VkFence nullFence = VK_NULL_HANDLE;
    // Get the index of the next available swapchain image:
    VkResult result = swapChainObj->fpAcquireNextImageKHR(
      deviceObj->device, swapChain, UINT64_MAX,
      presentCompleteSemaphore, VK_NULL_HANDLE,
      &currentColorImage );
    VkPipelineStageFlags submitPipelineStages =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // Prepare the submit into control structure
    VkSubmitInfo submitInfo = { };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
    submitInfo.pWaitDstStageMask = &submitPipelineStages;
    submitInfo.commandBufferCount = ( uint32_t )sizeof( &vecCmdDraw
      [ currentColorImage ] ) / sizeof( VkCommandBuffer );
    submitInfo.pCommandBuffers = &vecCmdDraw[ currentColorImage ];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &drawingCompleteSemaphore;
    // Queue the command buffer for execution
    CommandBufferMgr::submitCommandBuffer( deviceObj->queue,
      &cmdDraw[ currentColorImage ], &submitInfo );
    // Present the image in the window
    VkPresentInfoKHR present;
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pNext = nullptr;
    present.swapchainCount = 1;
    present.pSwapchains = &swapChain;
    present.pImageIndices = &currentColorImage;
    present.pWaitSemaphores = &drawingCompleteSemaphore;
    present.waitSemaphoreCount = 1;
    present.pResults = nullptr;
    // Queue the image for presentation,
    result = swapChainObj->fpQueuePresentKHR
      ( deviceObj->queue, &present );
    assert( result == VK_SUCCESS );
  }
}