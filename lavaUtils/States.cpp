#include "States.h"

namespace lava
{
  RasterizationState::RasterizationState(vk::PolygonMode polygonMode_, 
    vk::CullModeFlags cullMode_, vk::FrontFace frontFace_,
    bool depthClampEnable_, 
    bool rasterizerDiscardEnable_) noexcept
  {
    this->depthClampEnable = depthClampEnable_ ? VK_TRUE : VK_FALSE;
    this->rasterizerDiscardEnable = rasterizerDiscardEnable_ ? VK_TRUE : VK_FALSE;
    this->polygonMode = polygonMode_;
    this->cullMode = cullMode_;
    this->frontFace = frontFace_;
    depthBiasEnable = VK_FALSE;
    depthBiasConstantFactor = 0.0f;
    depthBiasClamp = 0.0f;
    depthBiasSlopeFactor = 0.0f;
    lineWidth = 1.0f;
  }
  
  namespace states
  {
    const RasterizationState fillCullNoneCCW(vk::PolygonMode::eFill, 
      vk::CullModeFlagBits::eNone, vk::FrontFace::eCounterClockwise);
    const RasterizationState fillCullBackCCW(vk::PolygonMode::eFill, 
      vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise);
    const RasterizationState fillCullFrontCCW(vk::PolygonMode::eFill, 
      vk::CullModeFlagBits::eFront, vk::FrontFace::eCounterClockwise);
    const RasterizationState fillCullFrontAndBackCCW(vk::PolygonMode::eFill, 
      vk::CullModeFlagBits::eFrontAndBack, vk::FrontFace::eCounterClockwise);

    const RasterizationState lineCullNoneCCW(vk::PolygonMode::eLine, 
      vk::CullModeFlagBits::eNone, vk::FrontFace::eCounterClockwise);
    const RasterizationState lineCullBackCCW(vk::PolygonMode::eLine, 
      vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise);
    const RasterizationState lineCullFrontCCW(vk::PolygonMode::eLine, 
      vk::CullModeFlagBits::eFront, vk::FrontFace::eCounterClockwise);
    const RasterizationState lineCullFrontAndBackCCW(vk::PolygonMode::eLine, 
      vk::CullModeFlagBits::eFrontAndBack, vk::FrontFace::eCounterClockwise);

    const RasterizationState pointCullNoneCCW(vk::PolygonMode::ePoint, 
      vk::CullModeFlagBits::eNone, vk::FrontFace::eCounterClockwise);
    const RasterizationState pointCullBackCCW(vk::PolygonMode::ePoint, 
      vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise);
    const RasterizationState pointCullFrontCCW(vk::PolygonMode::ePoint, 
      vk::CullModeFlagBits::eFront, vk::FrontFace::eCounterClockwise);
    const RasterizationState pointCullFrontAndBackCCW(vk::PolygonMode::ePoint, 
      vk::CullModeFlagBits::eFrontAndBack, vk::FrontFace::eCounterClockwise);

    const RasterizationState fillCullNoneCW(vk::PolygonMode::eFill, 
      vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise);
    const RasterizationState fillCullBackCW(vk::PolygonMode::eFill, 
      vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise);
    const RasterizationState fillCullFrontCW(vk::PolygonMode::eFill, 
      vk::CullModeFlagBits::eFront, vk::FrontFace::eClockwise);
    const RasterizationState fillCullFrontAndBackCW(vk::PolygonMode::eFill, 
      vk::CullModeFlagBits::eFrontAndBack, vk::FrontFace::eClockwise);

    const RasterizationState lineCullNoneCW(vk::PolygonMode::eLine,
      vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise);
    const RasterizationState lineCullBackCW(vk::PolygonMode::eLine, 
      vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise);
    const RasterizationState lineCullFrontCW(vk::PolygonMode::eLine, 
      vk::CullModeFlagBits::eFront, vk::FrontFace::eClockwise);
    const RasterizationState lineCullFrontAndBackCW(vk::PolygonMode::eLine, 
      vk::CullModeFlagBits::eFrontAndBack, vk::FrontFace::eClockwise);

    const RasterizationState pointCullNoneCW(vk::PolygonMode::ePoint, 
      vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise);
    const RasterizationState pointCullBackCW(vk::PolygonMode::ePoint, 
      vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise);
    const RasterizationState pointCullFrontCW(vk::PolygonMode::ePoint, 
      vk::CullModeFlagBits::eFront, vk::FrontFace::eClockwise);
    const RasterizationState pointCullFrontAndBackCW(vk::PolygonMode::ePoint, 
      vk::CullModeFlagBits::eFrontAndBack, vk::FrontFace::eClockwise);
  }
  
  StencilOpState::StencilOpState(vk::StencilOp failOp_, 
    vk::StencilOp passOp_, vk::StencilOp depthFailOp_, 
    vk::CompareOp compareOp_,
    uint32_t compareMask_, uint32_t writeMask_, 
    uint32_t reference_ ) noexcept
  {
    this->failOp = failOp_;
    this->passOp = passOp_;
    this->depthFailOp = depthFailOp_;
    this->compareOp = compareOp_;
    this->compareMask = compareMask_;
    this->writeMask = writeMask_;
    this->reference = reference_;
  }

  DepthStencilState::DepthStencilState(vk::CompareOp depthCompareOp_, 
    bool depthWriteEnable_,
    const StencilOpState& front_, const StencilOpState& back_) noexcept
  {
    depthTestEnable = (depthCompareOp_ != vk::CompareOp::eAlways);
    this->depthWriteEnable = depthWriteEnable_ ? VK_TRUE : VK_FALSE;
    this->depthCompareOp = depthCompareOp_;
    this->front = front_;
    this->back = back_;
    stencilTestEnable = (front_.compareOp != vk::CompareOp::eAlways) || 
      (back_.compareOp != vk::CompareOp::eAlways);
    depthBoundsTestEnable = VK_FALSE;
    minDepthBounds = 0.f;
    maxDepthBounds = 1.f;
  }

  namespace states
  {
    const StencilOpState stencilAlwaysDontWrite(vk::StencilOp::eKeep, 
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, 
      vk::CompareOp::eAlways, 0x0, 0x0, 0x0);

    const DepthStencilState depthLess(vk::CompareOp::eLess, true);
    const DepthStencilState depthEqual(vk::CompareOp::eEqual, true);
    const DepthStencilState depthLessOrEqual(vk::CompareOp::eLessOrEqual, true);
    const DepthStencilState depthGreater(vk::CompareOp::eGreater, true);
    const DepthStencilState depthNoEqual(vk::CompareOp::eNotEqual, true);
    const DepthStencilState depthGreaterOrEqual(vk::CompareOp::eGreaterOrEqual, true);
    const DepthStencilState depthAlways(vk::CompareOp::eAlways, true);

    const DepthStencilState depthNeverDontWrite(vk::CompareOp::eNever, false);
    const DepthStencilState depthLessDontWrite(vk::CompareOp::eLess, false);
    const DepthStencilState depthEqualDontWrite(vk::CompareOp::eEqual, false);
    const DepthStencilState depthLessOrEqualDontWrite(vk::CompareOp::eLessOrEqual, false);
    const DepthStencilState depthGreaterDontWrite(vk::CompareOp::eGreater, false);
    const DepthStencilState depthNoEqualDontWrite(vk::CompareOp::eNotEqual, false);
    const DepthStencilState depthGreaterOrEqualDontWrite(vk::CompareOp::eGreaterOrEqual, false);
    const DepthStencilState depthAlwaysDontWrite(vk::CompareOp::eAlways, false);
  }
  
  InputAssemblyState::InputAssemblyState(vk::PrimitiveTopology topology_,
    bool primitiveRestartEnable_) noexcept
  {
    this->topology = topology_;
    this->primitiveRestartEnable = primitiveRestartEnable_ ? VK_TRUE : VK_FALSE;
  }

  namespace states
  {
    const InputAssemblyState pointList(vk::PrimitiveTopology::ePointList);
    const InputAssemblyState lineList(vk::PrimitiveTopology::eLineList);
    const InputAssemblyState lineStrip(vk::PrimitiveTopology::eLineStrip);
    const InputAssemblyState triangleList(vk::PrimitiveTopology::eTriangleList);
    const InputAssemblyState triangleStrip(vk::PrimitiveTopology::eTriangleStrip);
    const InputAssemblyState triangleFan(vk::PrimitiveTopology::eTriangleFan);
    const InputAssemblyState lineListWithAdjacency(
      vk::PrimitiveTopology::eLineListWithAdjacency);
    const InputAssemblyState lineStripWithAdjacency(
      vk::PrimitiveTopology::eLineStripWithAdjacency);
    const InputAssemblyState triangleListWithAdjacency(
      vk::PrimitiveTopology::eTriangleListWithAdjacency);
    const InputAssemblyState triangleStripWithAdjacency(
      vk::PrimitiveTopology::eTriangleStripWithAdjacency);
    const InputAssemblyState patchList(vk::PrimitiveTopology::ePatchList);

    const InputAssemblyState pointListRestart(vk::PrimitiveTopology::ePointList, 
      true);
    const InputAssemblyState lineListRestart(vk::PrimitiveTopology::eLineList, 
      true);
    const InputAssemblyState lineStripRestart(vk::PrimitiveTopology::eLineStrip, 
      true);
    const InputAssemblyState triangleListRestart(
      vk::PrimitiveTopology::eTriangleList, true);
    const InputAssemblyState triangleStripRestart(
      vk::PrimitiveTopology::eTriangleStrip, true);
    const InputAssemblyState triangleFanRestart(
      vk::PrimitiveTopology::eTriangleFan, true);
    const InputAssemblyState lineListWithAdjacencyRestart(
      vk::PrimitiveTopology::eLineListWithAdjacency, true);
    const InputAssemblyState lineStripWithAdjacencyRestart(
      vk::PrimitiveTopology::eLineStripWithAdjacency, true);
    const InputAssemblyState triangleListWithAdjacencyRestart(
      vk::PrimitiveTopology::eTriangleListWithAdjacency, true);
    const InputAssemblyState triangleStripWithAdjacencyRestart(
      vk::PrimitiveTopology::eTriangleStripWithAdjacency, true);
    const InputAssemblyState patchListRestart(vk::PrimitiveTopology::ePatchList, 
      true);
  }

  SamplerState::SamplerState(vk::Filter magFilter_, vk::Filter minFilter_, 
    vk::SamplerMipmapMode mipmapMode_, vk::SamplerAddressMode addressMode_) noexcept
  {
    this->magFilter = magFilter_;
    this->minFilter = minFilter_;
    this->mipmapMode = mipmapMode_;
    this->addressModeU = this->addressModeV = this->addressModeW = addressMode_;
    this->anisotropyEnable = false;
    this->maxAnisotropy = 0.0f;

    this->compareEnable = VK_FALSE;
    this->compareOp = vk::CompareOp::eNever;
    this->minLod = 0.0f;
    this->maxLod = std::numeric_limits<float>::max();
    this->borderColor = vk::BorderColor::eFloatTransparentBlack;
    this->unnormalizedCoordinates = VK_FALSE;
  }
  SamplerState::SamplerState(float maxAnisotropy_, 
    vk::SamplerAddressMode addressMode_) noexcept
  {
    this->magFilter = vk::Filter::eLinear;
    this->minFilter = vk::Filter::eLinear;
    this->mipmapMode = vk::SamplerMipmapMode::eLinear;
    this->addressModeU = this->addressModeV = this->addressModeW = addressMode_;
    this->anisotropyEnable = true;
    this->maxAnisotropy = maxAnisotropy_;

    this->compareEnable = VK_FALSE;
    this->compareOp = vk::CompareOp::eNever;
    this->minLod = 0.0f;
    this->maxLod = std::numeric_limits<float>::max();
    this->borderColor = vk::BorderColor::eFloatTransparentBlack;
    this->unnormalizedCoordinates = VK_FALSE;
  }

  namespace states
  {
    const SamplerState nearestMipmapNearestRepeat(vk::Filter::eNearest, 
      vk::Filter::eNearest, vk::SamplerMipmapMode::eNearest, 
      vk::SamplerAddressMode::eRepeat);
    const SamplerState linearMipmapNearestRepeat(vk::Filter::eLinear, 
      vk::Filter::eLinear, vk::SamplerMipmapMode::eNearest, 
      vk::SamplerAddressMode::eRepeat);
    const SamplerState linearMipmapLinearRepeat(vk::Filter::eLinear, 
      vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, 
      vk::SamplerAddressMode::eRepeat);
    const SamplerState anisotropicRepeat(16.f, vk::SamplerAddressMode::eRepeat);

    const SamplerState nearestMipmapNearestMirroredRepeat(vk::Filter::eNearest, 
      vk::Filter::eNearest, vk::SamplerMipmapMode::eNearest, 
      vk::SamplerAddressMode::eMirroredRepeat);
    const SamplerState linearMipmapNearestMirroredRepeat(vk::Filter::eLinear, 
      vk::Filter::eLinear, vk::SamplerMipmapMode::eNearest, 
      vk::SamplerAddressMode::eMirroredRepeat);
    const SamplerState linearMipmapLinearMirroredRepeat(vk::Filter::eLinear, 
      vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, 
      vk::SamplerAddressMode::eMirroredRepeat);
    const SamplerState anisotropicMirroredRepeat(16.f, 
      vk::SamplerAddressMode::eMirroredRepeat);

    const SamplerState nearestMipmapNearestClampToEdge(vk::Filter::eNearest, 
      vk::Filter::eNearest, vk::SamplerMipmapMode::eNearest, 
      vk::SamplerAddressMode::eClampToEdge);
    const SamplerState linearMipmapNearestClampToEdge(vk::Filter::eLinear, 
      vk::Filter::eLinear, vk::SamplerMipmapMode::eNearest, 
      vk::SamplerAddressMode::eClampToEdge);
    const SamplerState linearMipmapLinearClampToEdge(vk::Filter::eLinear, 
      vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, 
      vk::SamplerAddressMode::eClampToEdge);
    const SamplerState anisotropicClampToEdge(16.f, 
      vk::SamplerAddressMode::eClampToEdge);

    const SamplerState nearestMipmapNearestClampToBorder(vk::Filter::eNearest, 
      vk::Filter::eNearest, vk::SamplerMipmapMode::eNearest, 
      vk::SamplerAddressMode::eClampToBorder);
    const SamplerState linearMipmapNearestClampToBorder(vk::Filter::eLinear, 
      vk::Filter::eLinear, vk::SamplerMipmapMode::eNearest, 
      vk::SamplerAddressMode::eClampToBorder);
    const SamplerState linearMipmapLinearClampToBorder(vk::Filter::eLinear, 

      vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, 
      vk::SamplerAddressMode::eClampToBorder);
    const SamplerState anisotropicClampToBorder(16.f, 
      vk::SamplerAddressMode::eClampToBorder);

    const SamplerState nearestMipmapNearestMirrorClampToEdge(vk::Filter::eNearest, 
      vk::Filter::eNearest, vk::SamplerMipmapMode::eNearest, 
      vk::SamplerAddressMode::eMirrorClampToEdge);
    const SamplerState linearMipmapNearestMirrorClampToEdge(vk::Filter::eLinear, 
      vk::Filter::eLinear, vk::SamplerMipmapMode::eNearest, 
      vk::SamplerAddressMode::eMirrorClampToEdge);
    const SamplerState linearMipmapLinearMirrorClampToEdge(vk::Filter::eLinear, 
      vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, 
      vk::SamplerAddressMode::eMirrorClampToEdge);
    const SamplerState anisotropicMirrorClampToEdge(16.f, 
      vk::SamplerAddressMode::eMirrorClampToEdge);

  }
}
