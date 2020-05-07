#pragma once

#include "d3d9_include.h"

#include "d3d9_util.h"

#include "../dxvk/dxvk_hash.h"

#include "../util/util_math.h"

namespace dxvk {

  struct D3D9SamplerKey {
    D3DTEXTUREADDRESS AddressU;
    D3DTEXTUREADDRESS AddressV;
    D3DTEXTUREADDRESS AddressW;
    D3DTEXTUREFILTERTYPE MagFilter;
    D3DTEXTUREFILTERTYPE MinFilter;
    D3DTEXTUREFILTERTYPE MipFilter;
    DWORD MaxAnisotropy;
    float MipmapLodBias;
    DWORD MaxMipLevel;
    float BorderColor[4];
  };

  struct D3D9SamplerKeyHash {
    size_t operator () (const D3D9SamplerKey& key) const;
  };

  struct D3D9SamplerKeyEq {
    bool operator () (const D3D9SamplerKey& a, const D3D9SamplerKey& b) const;
  };

  inline void NormalizeSamplerKey(D3D9SamplerKey& key) {
    key.AddressU = std::clamp(key.AddressU, D3DTADDRESS_WRAP, D3DTADDRESS_MIRRORONCE);
    key.AddressV = std::clamp(key.AddressV, D3DTADDRESS_WRAP, D3DTADDRESS_MIRRORONCE);
    key.AddressW = std::clamp(key.AddressW, D3DTADDRESS_WRAP, D3DTADDRESS_MIRRORONCE);

    key.MagFilter = std::clamp(key.MagFilter, D3DTEXF_NONE, D3DTEXF_ANISOTROPIC);
    key.MinFilter = std::clamp(key.MinFilter, D3DTEXF_NONE, D3DTEXF_ANISOTROPIC);
    key.MipFilter = std::clamp(key.MipFilter, D3DTEXF_NONE, D3DTEXF_ANISOTROPIC);

    key.MaxAnisotropy = std::min<DWORD>(key.MaxAnisotropy, 16);

    if (key.MipFilter == D3DTEXF_NONE) {
      // May as well try and keep slots down.
      key.MipmapLodBias = 0;
    }
    else {
      // Games also pass NAN/INF here, this accounts for that.
      if (unlikely(std::isnan(key.MipmapLodBias)))
        key.MipmapLodBias = 0.0f;

      // Clamp between -15.0f and 15.0f, matching mip limits of d3d9.
      key.MipmapLodBias = std::clamp(key.MipmapLodBias, -15.0f, 15.0f);

      // Round to the nearest .5
      // Fixes sampler leaks in UE3 games w/ mip streaming
      // eg. Borderlands 2
      key.MipmapLodBias = std::round(key.MipmapLodBias * 2.0f) / 2.0f;
    }

    if (key.AddressU != D3DTADDRESS_BORDER
     && key.AddressV != D3DTADDRESS_BORDER
     && key.AddressW != D3DTADDRESS_BORDER) {
      for (auto& val : key.BorderColor)
        val = 0.0f;
    }
    else {
      for (auto& val : key.BorderColor)
        val = val >= 0.5f ? 1.0f : 0.0f;
    }
  }

}