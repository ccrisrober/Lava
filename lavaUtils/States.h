#pragma once

#include <lava/lava.h>

namespace lava
{
  struct RasterizationState : vk::PipelineRasterizationStateCreateInfo
  {
    RasterizationState(vk::PolygonMode polygonMode,
      vk::CullModeFlags cullMode,
      vk::FrontFace frontFace,
      bool depthClampEnable = false,
      bool rasterizerDiscardEnable = false) noexcept;
  };
  namespace states
  {
    extern const RasterizationState fillCullNoneCCW;
    extern const RasterizationState fillCullBackCCW;
    extern const RasterizationState fillCullFrontCCW;
    extern const RasterizationState fillCullFrontAndBackCCW;

    extern const RasterizationState lineCullNoneCCW;
    extern const RasterizationState lineCullBackCCW;
    extern const RasterizationState lineCullFrontCCW;
    extern const RasterizationState lineCullFrontAndBackCCW;

    extern const RasterizationState pointCullNoneCCW;
    extern const RasterizationState pointCullBackCCW;
    extern const RasterizationState pointCullFrontCCW;
    extern const RasterizationState pointCullFrontAndBackCCW;

    extern const RasterizationState fillCullNoneCW;
    extern const RasterizationState fillCullBackCW;
    extern const RasterizationState fillCullFrontCW;
    extern const RasterizationState fillCullFrontAndBackCW;

    extern const RasterizationState lineCullNoneCW;
    extern const RasterizationState lineCullBackCW;
    extern const RasterizationState lineCullFrontCW;
    extern const RasterizationState lineCullFrontAndBackCW;

    extern const RasterizationState pointCullNoneCW;
    extern const RasterizationState pointCullBackCW;
    extern const RasterizationState pointCullFrontCW;
    extern const RasterizationState pointCullFrontAndBackCW;
  }

  struct StencilOpState : vk::StencilOpState
  {
    StencilOpState(vk::StencilOp failOp,
      vk::StencilOp passOp,
      vk::StencilOp depthFailOp,
      vk::CompareOp compareOp,
      uint32_t compareMask = 0x0,
      uint32_t writeMask = 0x0,
      uint32_t reference = 0x0) noexcept;
  };

  namespace states
  {
    extern const StencilOpState stencilAlwaysDontWrite;
  }

  /* The stencil test conditionally disables coverage of a sample
  based on the outcome of a comparison between the stencil value
  in the depth/stencil attachment at fragment location and a reference value.
  The stencil test also updates the value in the stencil attachment,
  depending on the test state, the stencil value and the stencil write masks. */

  struct DepthStencilState : vk::PipelineDepthStencilStateCreateInfo
  {
    DepthStencilState(vk::CompareOp depthCompareOp,
      bool depthWriteEnable,
      const StencilOpState& front = states::stencilAlwaysDontWrite,
      const StencilOpState& back = states::stencilAlwaysDontWrite) noexcept;
  };

  namespace states
  {
    extern const DepthStencilState depthLess;
    extern const DepthStencilState depthEqual;
    extern const DepthStencilState depthLessOrEqual;
    extern const DepthStencilState depthGreater;
    extern const DepthStencilState depthNoEqual;
    extern const DepthStencilState depthGreaterOrEqual;
    extern const DepthStencilState depthAlways;
    extern const DepthStencilState depthNever;

    extern const DepthStencilState depthLessDontWrite;
    extern const DepthStencilState depthEqualDontWrite;
    extern const DepthStencilState depthLessOrEqualDontWrite;
    extern const DepthStencilState depthGreaterDontWrite;
    extern const DepthStencilState depthNoEqualDontWrite;
    extern const DepthStencilState depthGreaterOrEqualDontWrite;
    extern const DepthStencilState depthAlwaysDontWrite;
  }
  struct InputAssemblyState : vk::PipelineInputAssemblyStateCreateInfo
  {
    InputAssemblyState(vk::PrimitiveTopology topology,
      bool primitiveRestartEnable = false) noexcept;
  };

  namespace states
  {
    extern const InputAssemblyState pointList;
    extern const InputAssemblyState lineList;
    extern const InputAssemblyState lineStrip;
    extern const InputAssemblyState triangleList;
    extern const InputAssemblyState triangleStrip;
    extern const InputAssemblyState triangleFan;
    extern const InputAssemblyState lineListWithAdjacency;
    extern const InputAssemblyState lineStripWithAdjacency;
    extern const InputAssemblyState triangleListWithAdjacency;
    extern const InputAssemblyState triangleStripWithAdjacency;
    extern const InputAssemblyState patchList;

    extern const InputAssemblyState pointListRestart;
    extern const InputAssemblyState lineListRestart;
    extern const InputAssemblyState lineStripRestart;
    extern const InputAssemblyState triangleListRestart;
    extern const InputAssemblyState triangleStripRestart;
    extern const InputAssemblyState triangleFanRestart;
    extern const InputAssemblyState lineListWithAdjacencyRestart;
    extern const InputAssemblyState lineStripWithAdjacencyRestart;
    extern const InputAssemblyState triangleListWithAdjacencyRestart;
    extern const InputAssemblyState triangleStripWithAdjacencyRestart;
    extern const InputAssemblyState patchListRestart;
  }

  struct SamplerState : vk::SamplerCreateInfo
  {
    SamplerState(vk::Filter magFilter,
      vk::Filter minFilter,
      vk::SamplerMipmapMode mipmapMode,
      vk::SamplerAddressMode addressMode) noexcept;
    SamplerState(float maxAnisotropy,
      vk::SamplerAddressMode addressMode) noexcept;
  };

  namespace samplers
  {
    extern const SamplerState nearestMipmapNearestRepeat;
    extern const SamplerState linearMipmapNearestRepeat;
    extern const SamplerState linearMipmapLinearRepeat;
    extern const SamplerState anisotropicRepeat;

    extern const SamplerState nearestMipmapNearestMirroredRepeat;
    extern const SamplerState linearMipmapNearestMirroredRepeat;
    extern const SamplerState linearMipmapLinearMirroredRepeat;
    extern const SamplerState anisotropicMirroredRepeat;

    extern const SamplerState nearestMipmapNearestClampToEdge;
    extern const SamplerState linearMipmapNearestClampToEdge;
    extern const SamplerState linearMipmapLinearClampToEdge;
    extern const SamplerState anisotropicClampToEdge;

    extern const SamplerState nearestMipmapNearestClampToBorder;
    extern const SamplerState linearMipmapNearestClampToBorder;
    extern const SamplerState linearMipmapLinearClampToBorder;
    extern const SamplerState anisotropicClampToBorder;

    extern const SamplerState nearestMipmapNearestMirrorClampToEdge;
    extern const SamplerState linearMipmapNearestMirrorClampToEdge;
    extern const SamplerState linearMipmapLinearMirrorClampToEdge;
    extern const SamplerState anisotropicMirrorClampToEdge;
  }
}