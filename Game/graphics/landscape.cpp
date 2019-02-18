#include "landscape.h"

#include "rendererstorage.h"
#include "gothic.h"

using namespace Tempest;

Landscape::Landscape(const RendererStorage &storage)
  :storage(storage) {
  auto& device=storage.device;

  pf.reset(new PerFrame[device.maxFramesInFlight()]);
  for(size_t i=0;i<device.maxFramesInFlight();++i){
    pf[i].uboGpu = device.loadUbo(&uboCpu,sizeof(uboCpu));
    }
  }

void Landscape::setMatrix(uint32_t frameId, const Matrix4x4 &mat) {
  uboCpu.mvp = mat;
  pf[frameId].uboGpu.update(&uboCpu,0,sizeof(uboCpu));
  }

void Landscape::commitUbo(uint32_t /*frameId*/) {
  }

void Landscape::draw(Tempest::CommandBuffer &cmd, uint32_t frameId,const World& world) {
  PerFrame& pf      = this->pf[frameId];
  auto&     uboLand = pf.uboLand;
  auto&     blocks  = world.landBlocks();

  uboLand.resize(blocks.size());
  const Texture2d* prev=nullptr;
  for(size_t i=0;i<blocks.size();++i){
    auto& lnd=blocks [i];
    auto& ubo=uboLand[i];

    if(ubo.isEmpty())
      ubo = storage.device.uniforms(storage.uboLndLayout());
    if(!lnd.texture || lnd.texture->isEmpty())
      continue;

    if(lnd.texture!=prev) {
      prev=lnd.texture;
      ubo.set(0,pf.uboGpu,0,sizeof(uboCpu));
      ubo.set(1,*lnd.texture);

      uint32_t offset=0;
      if(lnd.alpha)
        cmd.setUniforms(storage.pLandAlpha,ubo,1,&offset); else
        cmd.setUniforms(storage.pLand,ubo,1,&offset);
      }
    cmd.draw(world.landVbo(),lnd.ibo);
    }
  }