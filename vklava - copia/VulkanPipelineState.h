#pragma once

#include "VulkanResource.h"
#include "RenderAPICapabilites.h"

#include <fstream>

namespace lava
{
  class VulkanPipeline : public VulkanResource
  {
  public:
    VulkanPipeline( VulkanDevicePtr device, VkPipeline pip )
      : VulkanResource( device )
      , _pipeline( pip )
    {
    }
    VkPipeline getHandle( void ) const
    {
      return _pipeline;
    }
    ~VulkanPipeline( void )
    {
      vkDestroyPipeline( _device->getLogical( ), _pipeline, nullptr );
    }
  protected:
    VkPipeline _pipeline;
  };

  class GpuProgram
  {
  public:
    bool isCompiled( void ) const
    {
      return _isCompiled;
    }

    const std::string getCompileErrorMessage( void ) const;

    bool _isCompiled = false;
  };
  class VulkanShaderModule : public VulkanResource
  {
  public:
    VulkanShaderModule( VulkanDevicePtr device, VkShaderModule module );
    ~VulkanShaderModule( void );

    VkShaderModule getHandle( void ) const
    {
      return _module;
    }

  private:
    VkShaderModule _module;
  };
  struct GPU_PROGRAM_DESC
  {
    std::string file;
    GpuProgramType type;
    GPU_PROGRAM_DESC( const std::string& file, GpuProgramType type )
    {
      this->file = file;
      this->type = type;
    }
  };
  class VulkanGpuProgram : public GpuProgram
  {
  public:
    VulkanGpuProgram( const GPU_PROGRAM_DESC& desc )
      : _desc( desc )
    {
      initialize( );
    }
    void initialize( void );
    ~VulkanGpuProgram( void );
    VulkanShaderModule* getShaderModule( /*UINT32 deviceIdx*/ ) const
    {
      return _module;
    }
  protected:
    std::vector< char > readFile( const std::string& filename )
    {
      std::ifstream file( filename, std::ios::ate | std::ios::binary );

      if ( !file.is_open( ) )
      {
        throw std::runtime_error( "failed to open file!" );
      }

      size_t fileSize = ( size_t ) file.tellg( );
      std::vector<char> buffer( fileSize );

      file.seekg( 0 );
      file.read( buffer.data( ), fileSize );

      file.close( );

      return buffer;
    }
    GPU_PROGRAM_DESC _desc;
    VulkanShaderModule* _module;
  };

  // Descriptor structure used for initializing a GPU pipeline state.
  struct PipelineStateDesc
  {
    // BlendState* blendState;
    // RasterizerState* rasterizerState;
    // DepthStencilState* depthStencilState;

    GpuProgram* vertexProgram;
    GpuProgram* fragmentProgram;
    GpuProgram* geometryProgram;
    GpuProgram* tessCtrlProgram;
    GpuProgram* tessEvalProgram;
  };
  typedef PipelineStateDesc StateDescType;
  
  class TGraphicsPipelineState
  {
  public:
    bool hasVertexProgram( void ) const
    {
      return _data.vertexProgram != nullptr;
    }
    bool hasFragmentProgram( void ) const
    {
      return _data.fragmentProgram != nullptr;
    }
    bool hasGeometryProgram( void ) const
    {
      return _data.geometryProgram != nullptr;
    }
    bool hasTessCtrlProgram( void ) const
    {
      return _data.tessCtrlProgram != nullptr;
    }
    bool hasTessEvalProgram( void ) const
    {
      return _data.tessEvalProgram != nullptr;
    }

    const GpuProgram* getVertexProgram( void ) const
    {
      return _data.vertexProgram;
    }
    const GpuProgram* getFragmentProgram( void ) const
    {
      return _data.fragmentProgram;
    }
    const GpuProgram* getGeometryProgram( void ) const
    {
      return _data.geometryProgram;
    }
    const GpuProgram* getTessCtrlProgram( void ) const
    {
      return _data.tessCtrlProgram;
    }
    const GpuProgram* getTessEvalProgram( void ) const
    {
      return _data.tessEvalProgram;
    }
    virtual ~TGraphicsPipelineState( void ) { }
  protected:
    TGraphicsPipelineState( const StateDescType& desc );
    StateDescType _data;
  };

  class VulkanGraphicsPipelineState : public TGraphicsPipelineState
  {
  public:
    VulkanGraphicsPipelineState( VulkanDevicePtr device, const StateDescType& desc );
    virtual ~VulkanGraphicsPipelineState( void ) { }

    VkPipelineLayout getPipelineLayout( /*uint32_t deviceIdx*/ ) const;

    VulkanPipeline* createPipeline( void );

    VkPipelineShaderStageCreateInfo _shaderStageInfos[ 5 ];
    VkPipelineInputAssemblyStateCreateInfo _inputAssemblyInfo;
    VkPipelineTessellationStateCreateInfo _tesselationInfo;
    VkPipelineViewportStateCreateInfo _viewportInfo;
    VkPipelineRasterizationStateCreateInfo _rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo _multiSampleInfo;
    VkPipelineDepthStencilStateCreateInfo _depthStencilInfo;
    VkPipelineColorBlendAttachmentState _attachmentBlendStates;// [ NUM_RENDER_TARGETS ];
    VkPipelineColorBlendStateCreateInfo _colorBlendStateInfo;
    VkPipelineDynamicStateCreateInfo _dynamicStateInfo;
    VkDynamicState _dynamicStates[ 3 ];
    VkGraphicsPipelineCreateInfo _pipelineInfo;
  protected:
    VulkanDevicePtr _device;
  };

  class VulkanComputePipelineState
  {
  public:
    VulkanComputePipelineState( );
    ~VulkanComputePipelineState( void );
    VulkanPipeline* getPipeline( /*uint32_t deviceIdx*/ ) const;
    VkPipelineLayout getPipelineLayout( /*uint32_t deviceIdx*/ ) const;
  protected:
    VulkanDevicePtr _device;
    VulkanPipeline* _pipeline;
    VkPipelineLayout _pipelineLayout;
  };
}