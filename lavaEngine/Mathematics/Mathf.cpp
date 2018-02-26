#include "Mathf.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace lava
{
  float Mathf::lerp(float x, float x1, float x2, float q00, float q01)
  {
    return ((x2 - x) / (x2 - x1)) * q00 + ((x - x1) / (x2 - x1)) * q01;
  }
  float Mathf::biLerp(float x, float y, float q11, float q12,
  float q21, float q22, float x1, float x2, float y1, float y2)
  {
    float r1 = lerp(x, x1, x2, q11, q21);
    float r2 = lerp(x, x1, x2, q12, q22);
    return lerp(y, y1, y2, r1, r2);
  }
  float Mathf::triLerp(float x, float y, float z, float q000,
    float q001, float q010, float q011, float q100, float q101,
    float q110, float q111, float x1, float x2, float y1, float y2,
    float z1, float z2)
  {
    float x00 = lerp(x, x1, x2, q000, q100);
    float x10 = lerp(x, x1, x2, q010, q110);
    float x01 = lerp(x, x1, x2, q001, q101);
    float x11 = lerp(x, x1, x2, q011, q111);
    float r0 = lerp(y, y1, y2, x00, x01);
    float r1 = lerp(y, y1, y2, x10, x11);

    return lerp(z, z1, z2, r0, r1);
  }
  float Mathf::degToRad(float degs)
  {
    return degs * Deg2Rad;
  }
  float Mathf::radToDeg(float rads)
  {
    return rads * Rad2Deg;
  }
  bool Mathf::isPOT(unsigned int v)
  {
    return (v & (v - 1)) == 0 && v != 0;
  }
  unsigned int Mathf::nearestPOT(unsigned int v)
  {
    return std::pow(2, std::round(std::log(v) / std::log(2)));
  }
  float Mathf::clamp(float v, float min, float max)
  {
    return std::min(max, std::max(min, v));
  }
  float Mathf::clamp01(float v)
  {
    return std::min(1.0f, std::max(0.0f, v));
  }
  int Mathf::sign(int v)
  {
    if (v == 0)
    {
        return v;
    }
    return (v > 0) ? 1 : -1;
  }
  float Mathf::normalizeAngle(float radAngle)
  {
  float twoPi = 2.0f * PI;
  return radAngle - twoPi * std::floor(radAngle / twoPi);
  }
  float Mathf::smoothstep(float x, float min, float max)
  {
    if (x <= min) return 0.0f;
    if (x >= max) return 1.0f;

    x = (x - min) / (max - min);

    return x * x * (3.0f - 2.0f * x);
  }
  float Mathf::smootherstep(float x, float min, float max)
  {
    if (x <= min) return 0.0f;
    if (x >= max) return 1.0f;
    x = (x - min) / (max - min);
    return std::pow(x, 3.0f) * (x * (x * 6.0f - 15.0f) + 10.0f);
  }
  std::string Mathf::toHex( int i )
  {
    std::stringstream ss;
    ss << "0x"
       << std::setfill ( '0' ) << std::setw( sizeof( int ) * 2 )
       << std::hex << i;
    return ss.str();
  }
  const float Mathf::PI = 3.1415f;
  const float Mathf::PI_2 = PI * 0.5f;
  const float Mathf::TWO_PI = 2.0f * PI;
  const float Mathf::Deg2Rad = PI / 180.0f;
  const float Mathf::Rad2Deg = 180.0f / PI;
}
