#include "Easing.h"

#include <lavaEngine/Mathematics/Mathf.h>
#include <cmath>

namespace lava
{
  namespace easing
  {
    // Sine functions
    /**
      * Easing equation for a sinusoidal (sin(t)) ease-in,
      * accelerating from zero velocity.
      * @param  {number} t Time
      * @return {number}
      */
    float sine::easeIn( const float& t )
    {
      return std::sin(Mathf::PI_2 * t);
    }
    /**
      * Easing equation for a sinusoidal (sin(t)) ease-out,
      *     decelerating from zero velocity.
      * @param  {number} t Time
      * @return {number}
      */
    float sine::easeOut( const float& t )
    {
      return 1.0f + std::sin(Mathf::PI_2 * (t - 1.0f));
    }
    /**
      * Easing equation for a sinusoidal (sin(t)) ease-in/out,
      *     accelerating until halfway, then decelerating.
      * @param  {number} t Time
      * @return {number}
      */
    float sine::easeInOut( const float& t )
    {
      return 0.5f * (1.0f + std::sin(Mathf::PI * (t - 0.5f)));
    }

    // Quad functions
    namespace quad
    {
      /**
       * Easing equation for a quadratic (t^2) ease-in,
       *     accelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      float easeIn(const float& t)
      {
        return t * t;
      }
      /**
       * Easing equation for a quadratic (t^2) ease-out,
       *     decelerating to zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      float easeOut(const float& t)
      {
        return t * (2.0f - t);
      }
      /**
       * Easing equation for a quadratic (t^2) ease-in/out,
       *     accelerating until halfway, then decelerating.
       * @param  {number} t Time
       * @return {number}
       */

      float easeInOut(const float& t)
      {
        return t < 0.5f ? 2.0f * t * t : t * (4.0f - 2.0f * t) - 1.0f;
      }
    }

    // Cubic functions
    namespace cubic
    {
      /**
       * Easing equation function for a cubic (t^3) ease-in,
       *     accelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      float easeIn(const float& t)
      {
        return t * t * t;
      }
      /**
       * Easing equation for a cubic (t^3) ease-out,
       * decelerating to zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      float easeOut(const float& t)
      {
        return 1.0f + (t - 1.0f) * t * t;
      }
      /**
       * Easing equation for a cubic (t^3) ease-in/out,
       *     accelerating until halfway, then decelerating.
       * @param  {number} t Time
       * @return {number}
       */
      float easeInOut(const float& t)
      {
        return t < 0.5f ? 4.0f * t * t * t : 1.0f + (t - 1.0f) *
            (2.0f * (t - 1.0f)) * (2.0f * t);
      }
    }

    // Quart functions
    namespace quart
    {
      /**
       * Easing equation for a quartic (t^4) ease-in,
       * accelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      float easeIn(const float& t0)
      {
        float t = t0;
        t *= t;
        return t * t;
      }
      /**
       * Easing equation for a quartic (t^4) ease-out,
       *     decelerating to zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      float easeOut(const float& t0)
      {
        float t = t0;
        t = (t - 1.0f) * t;
        return 1.0f - t * t;
      }
      /**
       * Easing equation for a quartic (t^4) ease-in/out,
       * accelerating until halfway, then decelerating.
       * @param  {number} t Time
       * @return {number}
       */
      float easeInOut(const float& t0)
      {
        float t = t0;
        if (t < 0.5f)
        {
          t *= t;
          return 8.0f * t * t;
        }
        else
        {
          t = (t - 1.0f) * t;
          return 1.0f - 8.0f * t * t;
        }
      }
    }

    // Quint functions
    namespace quint
    {
      /**
       * Easing equation function for a quintic (t^5) ease-in,
       *     accelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      float easeIn(const float& t)
      {
        float t2 = t * t;
        return t * t2 * t2;
      }
      /**
       * Easing equation for a quintic (t^5) ease-out,
       *     decelerating to zero velocity..
       * @param  {number} t Time
       * @return {number}
       */
      float easeOut(const float& t)
      {
        float t2 = (t - 1.0f) * t;
        return 1.0f + t * t2 * t2;
      }
      /**
       * Easing equation for a quintic (t^5) ease-in/out,
       *     accelerating until halfway, then decelerating.
       * @param  {number} t Time
       * @return {number}
       */
      float easeInOut(const float& t)
      {
        float t2;
        if (t < 0.5f)
        {
          t2 = t * t;
          return 16.0f * t * t2 * t2;
        }
        else
        {
          t2 = (t - 1.0f) * t;
          return 1.0f + 16.0f * t * t2 * t2;
        }
      }
    }

    // Expo functions
    namespace expo
    {
      /**
       * Easing equation for an exponential (2^t) ease-in,
       *     accelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      float easeIn(const float& t)
      {
        return (std::pow(2.0f, 8.0f * t) - 1.0f) / 255.0f;
      }
      /**
       * Easing equation for an exponential (2^t) ease-out,
       *     decelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      float easeOut(const float& t)
      {
        return 1 - std::pow(2.0f, -8.0f * t);
      }
      /**
       * Easing equation for an exponential (2^t) ease-in/out,
       *     accelerating until halfway, then decelerating.
       * @param  {number} t Time
       * @return {number}
       */
      float easeInOut(const float& t)
      {
        if (t < 0.5f)
        {
          return (std::pow(2.0f, 16.0f * t) - 1.0f) / 510.0f;
        }
        else
        {
          return 1.0f - 0.5f * std::pow(2.0f, -16.0f * (t - 0.5f));
        }
      }
    }

    // Circ functions
    namespace circ
    {
      /**
       * Easing equation for a circular (sqrt(1-t^2)) ease-in,
       *     accelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      float easeIn(const float& t)
      {
        return 1.0f - std::sqrt(1.0f - t);
      }
      /**
       * Easing equation for a circular (sqrt(1-t^2)) ease-out,
       *     decelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      float easeOut(const float& t)
      {
        return std::sqrt(t);
      }
      /**
       * Easing equation for a circular (sqrt(1-t^2)) ease-in/out,
       *     accelerating until halfway, then decelerating.
       * @param  {number} t Time
       * @return {number}
       */
      float easeInOut(const float& t)
      {
        if (t < 0.5f)
        {
          return (1.0f - std::sqrt(1.0f - 2.0f * t)) * 0.5f;
        }
        else
        {
          return (1.0f + std::sqrt(2.0f * t - 1.0f)) * 0.5f;
        }
      }
    }

    // Back functions
    namespace back
    {
      /**
       * Easing equation for a back (overshooting cubic easing:
       *     (s+1)*t^3 - s*t^2) ease-in, accelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      float easeIn(const float& t)
      {
        return t * t * (2.70158 * t - 1.70158);
      }
      /**
       * Easing equation for a back (overshooting cubic easing:
       *     (s+1)*t^3 - s*t^2) ease-out, decelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      float easeOut(const float& t)
      {
        return 1.0f + (t - 1.0f) * t * (2.70158 * t + 1.70158);
      }
      /**
       *  Easing equation for a back (overshooting cubic easing:
       *      (s+1)*t^3 - s*t^2) ease-in/out, accelerating until halfway,
       *      then decelerating.
       * @param  {number} t Time
       * @return {number}
       */
      float easeInOut(const float& t)
      {
        if (t < 0.5f)
        {
          return t * t * (7.0f * t - 2.5f) * 2.0f;
        }
        else
        {
          return 1.0f + (t - 1.0f) * t * 2.0f * (7.0f * t + 2.5f);
        }
      }
    }

    // Elastic functions
    namespace elastic
    {
      /**
       * Easing equation for an elastic (exponentially decaying
       *     sine wave) ease-in, accelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      float easeIn(const float& t)
      {
        float t2 = t * t;
        return t2 * t2 * std::sin(t * Mathf::PI * 4.5f);
      }
      /**
       * Easing equation for an elastic (exponentially decaying
       *     sine wave) ease-out, decelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      float easeOut(const float& t)
      {
        float t2 = (t - 1.0f) * (t - 1.0f);
        return 1.0f - t2 * t2 * std::cos(t * Mathf::PI * 4.5f);
      }
      /**
       * Easing equation for an elastic (exponentially decaying
       *     sine wave) ease-out/in, decelerating until halfway,
       *     then accelerating.
       * @param  {number} t Time
       * @return {number}
       */
      float easeInOut(const float& t)
      {
        float t2;
        if (t < 0.45)
        {
          t2 = t * t;
          return 8.0f * t2 * t2 * std::sin(t * Mathf::PI * 9.0f);
        }
        else if (t < 0.5f)
        {
          return 0.5f + 0.75 * std::sin(t * Mathf::PI * 4.0f);
        }
        else
        {
          t2 = (t - 1.0f) * (t - 1.0f);
          return 1.0f - 8.0f * t2 * t2 * std::sin(t * Mathf::PI * 9.0f);
        }
      }
    }

    // Bounce functions
    namespace bounce
    {
      /**
       * Easing equation for a bounce (exponentially decaying parabolic
       *     bounce) ease-in, accelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      float easeIn(const float& t)
      {
        return std::pow(2.0f, 6.0f * (t - 1.0f)) * std::abs(std::sin(t * Mathf::PI * 3.5f));
      }
      /**
       * Easing equation for a bounce (exponentially decaying parabolic
       *     bounce) ease-out, decelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      float easeOut(const float& t)
      {
        return 1.0f - std::pow(2.0f, -6.0f * t) * std::abs(std::cos(t * Mathf::PI * 3.5f));
      }
      /**
       * Easing equation for a bounce (exponentially decaying parabolic
       *     bounce) ease-in/out, accelerating until halfway, then decelerating.
       * @param  {number} t Time
       * @return {number}
       */
      float easeInOut(const float& t)
      {
        if (t < 0.5f)
        {
          return 8.0f * std::pow(2.0f, 8.0f * (t - 1.0f)) * std::abs(std::sin(t * Mathf::PI * 7.0f));
        }
        else
        {
          return 1.0f - 8.0f * std::pow(2.0f, -8.0f * t) * std::abs(std::sin(t * Mathf::PI * 7.0f));
        }
      }
    }
  }
}
