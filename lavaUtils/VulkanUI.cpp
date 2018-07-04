#include "VulkanUI.h"

namespace lava
{
  namespace utility
  {
    UIOverlay::UIOverlay( UIOverlayCreateInfo createInfo )
    {
      this->createInfo = createInfo;
      this->renderPass = createInfo.renderPass;

#if defined(__ANDROID__)		
      if ( vks::android::screenDensity >= ACONFIGURATION_DENSITY_XXHIGH ) {
        scale = 3.5f;
      }
      else if ( vks::android::screenDensity >= ACONFIGURATION_DENSITY_XHIGH ) {
        scale = 2.5f;
      }
      else if ( vks::android::screenDensity >= ACONFIGURATION_DENSITY_HIGH ) {
        scale = 2.0f;
      };
#endif

      // Init ImGui
      // Color scheme
      ImGuiStyle& style = ImGui::GetStyle( );
      style.Colors[ ImGuiCol_TitleBg ] = ImVec4( 1.0f, 0.0f, 0.0f, 1.0f );
      style.Colors[ ImGuiCol_TitleBgActive ] = ImVec4( 1.0f, 0.0f, 0.0f, 1.0f );
      style.Colors[ ImGuiCol_TitleBgCollapsed ] = ImVec4( 1.0f, 0.0f, 0.0f, 0.1f );
      style.Colors[ ImGuiCol_MenuBarBg ] = ImVec4( 1.0f, 0.0f, 0.0f, 0.4f );
      style.Colors[ ImGuiCol_Header ] = ImVec4( 0.8f, 0.0f, 0.0f, 0.4f );
      style.Colors[ ImGuiCol_HeaderActive ] = ImVec4( 1.0f, 0.0f, 0.0f, 0.4f );
      style.Colors[ ImGuiCol_HeaderHovered ] = ImVec4( 1.0f, 0.0f, 0.0f, 0.4f );
      style.Colors[ ImGuiCol_CheckMark ] = ImVec4( 1.0f, 0.0f, 0.0f, 0.8f );
      // Dimensions
      ImGuiIO& io = ImGui::GetIO( );
      io.DisplaySize = ImVec2( ( float ) ( createInfo.width ), ( float ) ( createInfo.height ) );
      io.FontGlobalScale = scale;

      cmdBuffers.resize( createInfo.framebuffers.size( ) );
      prepareResources( );
      if ( createInfo.renderPass == VK_NULL_HANDLE ) {
        prepareRenderPass( );
      }
      preparePipeline( );
    }

    UIOverlay::~UIOverlay( void )
    {
		vertexBuffer.reset();
		indexBuffer.reset();
		fontView.reset();
		fontImage.reset();
		descriptorSetLayout.reset();
		descriptorPool.reset();
		pipelineLayout.reset();
		pipeline.reset();
		if (createInfo.renderPass)
		{
			createInfo.renderPass.reset();
		}
		cmdBuffers.clear();
		commandPool.reset();
		fence.reset();
    }

    void UIOverlay::update( void )
    {
      ImDrawData* imDrawData = ImGui::GetDrawData( );
      bool updateCmdBuffers = false;

      if ( !imDrawData ) { return; };

      // Note: Alignment is done inside buffer creation
      vk::DeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof( ImDrawVert );
      vk::DeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof( ImDrawIdx );

      // Update buffers only if vertex or index count has been changed compared to current buffer size

      // Vertex buffer
      if ( ( !vertexBuffer ) || ( vertexCount != ( uint32_t ) imDrawData->TotalVtxCount ) )
      {
        vertexBuffer.reset( );
        vertexBuffer = createInfo.device->createBuffer( 
          vertexBufferSize, 
          vk::BufferUsageFlagBits::eVertexBuffer, 
          vk::MemoryPropertyFlagBits::eHostVisible
        );
        vertexCount = imDrawData->TotalVtxCount;
        updateCmdBuffers = true;
      }

      // Index buffer
      VkDeviceSize indexSize = imDrawData->TotalIdxCount * sizeof( ImDrawIdx );
      if ( ( !indexBuffer ) || ( indexCount < (uint32_t) imDrawData->TotalIdxCount ) )
      {
        indexBuffer.reset( );

        indexBuffer = createInfo.device->createBuffer(
          indexBufferSize,
          vk::BufferUsageFlagBits::eIndexBuffer,
          vk::MemoryPropertyFlagBits::eHostVisible
        );
        indexCount = imDrawData->TotalIdxCount;
        updateCmdBuffers = true;
      }

      // Upload data
      ImDrawVert* vtxDst = ( ImDrawVert* ) vertexBuffer->map( 0, vertexBufferSize );
      ImDrawIdx* idxDst = ( ImDrawIdx* ) indexBuffer->map( 0, indexBufferSize );

      for ( int n = 0; n < imDrawData->CmdListsCount; ++n )
      {
        const ImDrawList* cmd_list = imDrawData->CmdLists[ n ];
        memcpy( vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof( ImDrawVert ) );
        memcpy( idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof( ImDrawIdx ) );
        vtxDst += cmd_list->VtxBuffer.Size;
        idxDst += cmd_list->IdxBuffer.Size;
      }

      // Flush to make writes visible to GPU
      vertexBuffer->flush( 0, vertexBufferSize );
      indexBuffer->flush( 0, indexBufferSize );

      if ( updateCmdBuffers )
      {
        updateCommandBuffers( );
      }
    }

    void UIOverlay::resize( uint32_t width, uint32_t height, 
      std::vector<std::shared_ptr<Framebuffer>> framebuffers )
    {
      ImGuiIO& io = ImGui::GetIO( );
      io.DisplaySize = ImVec2( ( float ) ( width ), ( float ) ( height ) );
      createInfo.width = width;
      createInfo.height = height;
      createInfo.framebuffers = framebuffers;
      updateCommandBuffers( );
    }

    void UIOverlay::submit( std::shared_ptr<Queue> queue, 
      uint32_t bufferindex, SubmitInfo submitInfo )
    {
      if ( !visible ) return;
      std::vector<std::shared_ptr<CommandBuffer>> v;
      v.push_back( cmdBuffers[ bufferindex ] );
      submitInfo.commandBuffers = v;
      
      queue->submit( submitInfo, fence );
	  static uint32_t timeout = std::numeric_limits<uint64_t>::max();
      Fence::waitForFences( fence, VK_TRUE, timeout);
      Fence::resetFences( fence );
    }

    bool UIOverlay::header( const char* caption )
    {
      return ImGui::CollapsingHeader( caption, ImGuiTreeNodeFlags_DefaultOpen );
    }

    bool UIOverlay::checkBox( const char* caption, bool* value )
    {
      return ImGui::Checkbox( caption, value );
    }

    bool UIOverlay::checkBox( const char* caption, int32_t* value )
    {
      bool val = ( *value == 1 );
      bool res = ImGui::Checkbox( caption, &val );
      *value = val;
      return res;
    }

    bool UIOverlay::inputFloat( const char* caption, float* value, 
      float step, uint32_t precision )
    {
      return ImGui::InputFloat( caption, value, step, step * 10.0f, precision );
    }

    bool UIOverlay::sliderFloat( const char* caption, float* value, 
      float min, float max )
    {
      return ImGui::SliderFloat( caption, value, min, max );
    }

    bool UIOverlay::sliderInt( const char* caption, int32_t* value, 
      int32_t min, int32_t max )
    {
      return ImGui::SliderInt( caption, value, min, max );
    }

    bool UIOverlay::comboBox( const char* caption, int32_t* itemindex, 
      const std::vector<std::string>& items )
    {
      if ( items.empty( ) )
      {
        return false;
      }
      std::vector<const char*> charitems;
      charitems.reserve( items.size( ) );
      size_t l = items.size( );
      for ( size_t i = 0; i < l; ++i )
      {
        charitems.push_back( items[ i ].c_str( ) );
      }
      uint32_t itemCount = static_cast<uint32_t>( l );
      return ImGui::Combo( caption, itemindex, &charitems[ 0 ], 
        itemCount, itemCount );
    }

    bool UIOverlay::button( const char* caption )
    {
      return ImGui::Button( caption );
    }

    void UIOverlay::text( const char* formatstr, ... )
    {
      va_list args;
      va_start( args, formatstr );
      ImGui::TextV( formatstr, args );
      va_end( args );
    }

    void UIOverlay::prepareResources( void )
    {
      commandPool = createInfo.device->createCommandPool( 
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer );

      ImGuiIO& io = ImGui::GetIO( );

      // Create font texture
      unsigned char* fontData;
      int texWidth, texHeight;
      io.Fonts->GetTexDataAsRGBA32( &fontData, &texWidth, &texHeight );
      vk::DeviceSize uploadSize = texWidth*texHeight * 4 * sizeof( char );

      // Create target image for copy
      fontImage = createInfo.device->createImage( { }, vk::ImageType::e2D, 
        vk::Format::eR8G8B8A8Unorm, vk::Extent3D( texWidth, texHeight, 1 ), 1, 
        1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, 
        vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
        vk::SharingMode::eExclusive, { }, vk::ImageLayout::eUndefined, 
        vk::MemoryPropertyFlagBits::eDeviceLocal );

      fontView = fontImage->createImageView( vk::ImageViewType::e2D, 
        vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor );

      // Staging buffers for font data upload
      auto stagingBuffer = createInfo.device->createBuffer( uploadSize, 
        vk::BufferUsageFlagBits::eTransferSrc, 
        vk::MemoryPropertyFlagBits::eHostVisible | 
        vk::MemoryPropertyFlagBits::eHostCoherent );

      stagingBuffer->set( fontData );

      // Copy buffer data to font image
      auto copyCmd = commandPool->allocateCommandBuffer( );

      // Prepare for transfer
      lava::utils::transitionImageLayout( copyCmd, fontImage,
        vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal, vk::PipelineStageFlagBits::eHost, 
        vk::PipelineStageFlagBits::eTransfer );

      // Copy
      vk::BufferImageCopy bufferCopyRegion;
      bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
      bufferCopyRegion.imageSubresource.layerCount = 1;
      bufferCopyRegion.imageExtent.width = texWidth;
      bufferCopyRegion.imageExtent.height = texHeight;
      bufferCopyRegion.imageExtent.depth = 1;

      copyCmd->copyBufferToImage( stagingBuffer, fontImage, 
        vk::ImageLayout::eTransferDstOptimal, bufferCopyRegion );

      // Prepare for shader read
      lava::utils::transitionImageLayout( copyCmd, fontImage, 
        vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eTransferDstOptimal, 
        vk::ImageLayout::eShaderReadOnlyOptimal, 
        vk::PipelineStageFlagBits::eTransfer, 
        vk::PipelineStageFlagBits::eFragmentShader );

      // Font texture sampler
      sampler = createInfo.device->createSampler( 
        vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
        vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge,
        vk::SamplerAddressMode::eClampToEdge, 0.0f, false, 0.0f, false, { }, 
        0.0f, 0.0f, { }, false );

      for ( size_t i = 0, l = cmdBuffers.size( ); i < l; ++i )
      {
        cmdBuffers[ i ] = commandPool->allocateCommandBuffer( );
      }

      // Descriptor pool
      descriptorPool = createInfo.device->createDescriptorPool( 1, {
        vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 )
      } );

      // Descriptor set layout
      descriptorSetLayout = createInfo.device->createDescriptorSetLayout(
        DescriptorSetLayoutBinding( 0, vk::DescriptorType::eCombinedImageSampler,
          vk::ShaderStageFlagBits::eFragment )
      );

      // Descriptor set
      descriptorSet = createInfo.device->allocateDescriptorSet( descriptorPool, 
        descriptorSetLayout );

      createInfo.device->updateDescriptorSets( {
        WriteDescriptorSet(
          descriptorSet, 1, 0, vk::DescriptorType::eCombinedImageSampler, 1,
          DescriptorImageInfo( vk::ImageLayout::eShaderReadOnlyOptimal,
            fontView, sampler ), nullptr )
      }, { } );

      // Pipeline cache
      pipelineCache = createInfo.device->createPipelineCache( 0, nullptr );

      // Pipeline layout
      // Push constants for UI rendering parameters
      vk::PushConstantRange pushConstantRange(
        vk::ShaderStageFlagBits::eVertex, 0, sizeof( PushConstantBlock )
      );
      pipelineLayout = createInfo.device->createPipelineLayout(
        descriptorSetLayout, pushConstantRange );

      // Command buffer execution fence
      fence = createInfo.device->createFence( false );
    }
    void UIOverlay::preparePipeline( void )
    {
      // Vertex bindings an attributes based on ImGui vertex definition
      vk::VertexInputBindingDescription binding( 0, 
        sizeof( ImDrawVert ), vk::VertexInputRate::eVertex );

      PipelineVertexInputStateCreateInfo vertexInput( binding, {
        // Location 0: Position
        vk::VertexInputAttributeDescription(
          0, 0, vk::Format::eR32G32Sfloat, offsetof( ImDrawVert, pos )
        ),
        // Location 1: UV
        vk::VertexInputAttributeDescription(
          1, 0, vk::Format::eR32G32Sfloat, offsetof( ImDrawVert, uv )
        ),
        // Location 0: Color
        vk::VertexInputAttributeDescription(
          2, 0, vk::Format::eR8G8B8A8Unorm, offsetof( ImDrawVert, col )
        )
      } );

      vk::PipelineInputAssemblyStateCreateInfo assembly( { },
        vk::PrimitiveTopology::eTriangleList, VK_FALSE );
      PipelineViewportStateCreateInfo viewport( 1, 1 );

      PipelineDynamicStateCreateInfo dynamic( {
        vk::DynamicState::eViewport, vk::DynamicState::eScissor
      } );

      vk::PipelineRasterizationStateCreateInfo rasterization( { }, false,
        false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
        vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );

      PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
        false, 0.0f, nullptr, false, false );

      vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
        vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
        0, 0, 0 );
      vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true,
        vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
        stencilOpState, 0.0f, 0.0f );

      // Enable blending
      std::vector<vk::PipelineColorBlendAttachmentState> blendStates( 
        createInfo.attachmentCount );
      for ( uint32_t i = 0; i < createInfo.attachmentCount; ++i )
      {
        blendStates[ i ].blendEnable = VK_TRUE;
        blendStates[ i ].colorWriteMask =
          vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
        blendStates[ i ].srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        blendStates[ i ].dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        blendStates[ i ].colorBlendOp = vk::BlendOp::eAdd;
        blendStates[ i ].srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        blendStates[ i ].dstAlphaBlendFactor = vk::BlendFactor::eZero;
        blendStates[ i ].alphaBlendOp = vk::BlendOp::eAdd;
      }

      PipelineColorBlendStateCreateInfo colorBlend( true, vk::LogicOp::eNoOp,
        blendStates, { 1.0f, 1.0f, 1.0f, 1.0f }
      );

      pipeline = createInfo.device->createGraphicsPipeline( pipelineCache, { }, 
        createInfo.shaders, vertexInput, assembly, nullptr,
        viewport, rasterization, multisample, depthStencil, colorBlend, 
        dynamic, pipelineLayout, renderPass );
    }
    void UIOverlay::prepareRenderPass( void )
    {
      std::array<vk::AttachmentDescription, 2> attachments;

      // Color attachment
      attachments[ 0 ].format = createInfo.colorformat;
      attachments[ 0 ].samples = vk::SampleCountFlagBits::e1;
      attachments[ 0 ].loadOp = vk::AttachmentLoadOp::eLoad;
      attachments[ 0 ].storeOp = vk::AttachmentStoreOp::eStore;
      attachments[ 0 ].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
      attachments[ 0 ].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
      attachments[ 0 ].initialLayout = vk::ImageLayout::ePresentSrcKHR;
      attachments[ 0 ].finalLayout = vk::ImageLayout::ePresentSrcKHR;

      // Depth attachment
      attachments[ 1 ].format = createInfo.depthformat;
      attachments[ 1 ].samples = vk::SampleCountFlagBits::e1;
      attachments[ 1 ].loadOp = vk::AttachmentLoadOp::eDontCare;
      attachments[ 1 ].storeOp = vk::AttachmentStoreOp::eDontCare;
      attachments[ 1 ].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
      attachments[ 1 ].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
      attachments[ 1 ].initialLayout = vk::ImageLayout::eUndefined;
      attachments[ 1 ].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

      vk::AttachmentReference colorReference = { };
      colorReference.attachment = 0;
      colorReference.layout = vk::ImageLayout::eColorAttachmentOptimal;

      vk::AttachmentReference depthReference = { };
      depthReference.attachment = 1;
      depthReference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

      std::array<vk::SubpassDependency, 2> subpassDependencies;

      // Transition from final to initial (VK_SUBPASS_EXTERNAL refers to 
      //    all commmands executed outside of the actual renderpass)
      subpassDependencies[ 0 ].srcSubpass = VK_SUBPASS_EXTERNAL;
      subpassDependencies[ 0 ].dstSubpass = 0;
      subpassDependencies[ 0 ].srcStageMask = 
        vk::PipelineStageFlagBits::eBottomOfPipe;
      subpassDependencies[ 0 ].dstStageMask = 
        vk::PipelineStageFlagBits::eColorAttachmentOutput;
      subpassDependencies[ 0 ].srcAccessMask = vk::AccessFlagBits::eMemoryRead;
      subpassDependencies[ 0 ].dstAccessMask = 
        vk::AccessFlagBits::eColorAttachmentRead | 
        vk::AccessFlagBits::eColorAttachmentWrite;
      subpassDependencies[ 0 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

      // Transition from initial to final
      subpassDependencies[ 1 ].srcSubpass = 0;
      subpassDependencies[ 1 ].dstSubpass = VK_SUBPASS_EXTERNAL;
      subpassDependencies[ 1 ].srcStageMask = 
        vk::PipelineStageFlagBits::eColorAttachmentOutput;
      subpassDependencies[ 1 ].dstStageMask = 
        vk::PipelineStageFlagBits::eBottomOfPipe;
      subpassDependencies[ 1 ].srcAccessMask = 
        vk::AccessFlagBits::eColorAttachmentRead | 
        vk::AccessFlagBits::eColorAttachmentWrite;
      subpassDependencies[ 1 ].dstAccessMask = vk::AccessFlagBits::eMemoryRead;
      subpassDependencies[ 1 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

      vk::SubpassDescription subpassDescription;
      subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
      subpassDescription.inputAttachmentCount = 0;
      subpassDescription.pInputAttachments = nullptr;
      subpassDescription.colorAttachmentCount = 1;
      subpassDescription.pColorAttachments = &colorReference;
      subpassDescription.pResolveAttachments = nullptr;
      subpassDescription.pDepthStencilAttachment = &depthReference;
      subpassDescription.preserveAttachmentCount = 0;
      subpassDescription.pPreserveAttachments = nullptr;

      renderPass = createInfo.device->createRenderPass( attachments, 
        subpassDescription, subpassDependencies );
    }
    void UIOverlay::updateCommandBuffers( void )
    {
      ImGuiIO& io = ImGui::GetIO( );

      vk::Rect2D area;
      area.extent.width = createInfo.width;
      area.extent.height = createInfo.height;

      int i = 0;
      for ( auto& cmd : cmdBuffers )
      {
        cmd->begin( );

        cmd->beginRenderPass( renderPass, createInfo.framebuffers[ i ], 
          area, createInfo.clearValues, vk::SubpassContents::eInline );

        cmd->bindGraphicsPipeline( pipeline );
        cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics, 
          pipelineLayout, 0, descriptorSet, { } );
        cmd->bindVertexBuffer( 0, vertexBuffer, 0 );
        cmd->bindIndexBuffer( indexBuffer, 0, vk::IndexType::eUint16 );

        cmd->setViewportScissors( area.extent );

        // UI scale and translate via push constants
        pushConstBlock.scale = glm::vec2( 
          2.0f / io.DisplaySize.x, 
          2.0f / io.DisplaySize.y );
        pushConstBlock.translate = glm::vec2( -1.0f );

        cmd->pushConstants<PushConstantBlock>( pipelineLayout, 
          vk::ShaderStageFlagBits::eVertex, 0, pushConstBlock );

        // Render commands
        ImDrawData* imDrawData = ImGui::GetDrawData( );
        int32_t vertexOffset = 0;
        int32_t indexOffset = 0;
        for ( int32_t j = 0; j < imDrawData->CmdListsCount; ++j )
        {
          const ImDrawList* cmd_list = imDrawData->CmdLists[ j ];
          for ( int32_t k = 0; k < cmd_list->CmdBuffer.Size; ++k )
          {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[ k ];
            vk::Rect2D scissorRect;
            scissorRect.offset.x = std::max( ( int32_t ) ( pcmd->ClipRect.x ), 0 );
            scissorRect.offset.y = std::max( ( int32_t ) ( pcmd->ClipRect.y ), 0 );
            scissorRect.extent.width = ( uint32_t ) ( pcmd->ClipRect.z - pcmd->ClipRect.x );
            scissorRect.extent.height = ( uint32_t ) ( pcmd->ClipRect.w - pcmd->ClipRect.y );
            cmd->setScissor( 0, scissorRect );
            cmd->drawIndexed( pcmd->ElemCount, 1, indexOffset, vertexOffset, 0 );
            indexOffset += pcmd->ElemCount;
          }
          vertexOffset += cmd_list->VtxBuffer.Size;
        }

        // Add empty subpasses if requested
        if ( createInfo.subpassCount > 1 )
        {
          for ( uint32_t j = 1; j < createInfo.subpassCount; ++j )
          {
            cmd->nextSubpass( vk::SubpassContents::eInline );
          }
        }

        cmd->endRenderPass( );

        cmd->end( );

        ++i;
      }
    }
  }
}