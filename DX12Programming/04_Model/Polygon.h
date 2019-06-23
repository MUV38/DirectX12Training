#pragma once

#include <cstdint>
#include <DirectXMath.h>

/// ’¸“_
struct Vertex
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 uv;
};