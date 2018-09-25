/**
 * Copyright (c) 2017 - 2018, Lava
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#ifndef __POMPEIIENGINE_EASING__
#define __POMPEIIENGINE_EASING__

#include <pompeiiEngine/api.h>

namespace pompeii
{
  namespace easing
  {
    // Sine functions
    namespace sine
    {
      /**
       * Easing equation for a sinusoidal (sin(t)) ease-in,
       * accelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeIn(const float& t);
      /**
       * Easing equation for a sinusoidal (sin(t)) ease-out,
       *     decelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeOut(const float& t);
      /**
       * Easing equation for a sinusoidal (sin(t)) ease-in/out,
       *     accelerating until halfway, then decelerating.
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeInOut(const float& t);
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
      POMPEIIENGINE_API
      float easeIn(const float& t);
      /**
       * Easing equation for a quadratic (t^2) ease-out,
       *     decelerating to zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeOut(const float& t);
      /**
       * Easing equation for a quadratic (t^2) ease-in/out,
       *     accelerating until halfway, then decelerating.
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeInOut(const float& t);
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
      POMPEIIENGINE_API
      float easeIn(const float& t);
      /**
       * Easing equation for a cubic (t^3) ease-out,
       * decelerating to zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeOut(const float& t);
      /**
       * Easing equation for a cubic (t^3) ease-in/out,
       *     accelerating until halfway, then decelerating.
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeInOut(const float& t);
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
      POMPEIIENGINE_API
      float easeIn(const float& t);
      /**
       * Easing equation for a quartic (t^4) ease-out,
       *     decelerating to zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeOut(const float& t);
      /**
       * Easing equation for a quartic (t^4) ease-in/out,
       * accelerating until halfway, then decelerating.
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeInOut(const float& t);
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
      POMPEIIENGINE_API
      float easeIn(const float& t);
      /**
       * Easing equation for a quintic (t^5) ease-out,
       *     decelerating to zero velocity..
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeOut(const float& t);
      /**
       * Easing equation for a quintic (t^5) ease-in/out,
       *     accelerating until halfway, then decelerating.
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeInOut(const float& t);
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
      POMPEIIENGINE_API
      float easeIn(const float& t);
      /**
       * Easing equation for an exponential (2^t) ease-out,
       *     decelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeOut(const float& t);
      /**
       * Easing equation for an exponential (2^t) ease-in/out,
       *     accelerating until halfway, then decelerating.
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeInOut(const float& t);
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
      POMPEIIENGINE_API
      float easeIn(const float& t);
      /**
       * Easing equation for a circular (sqrt(1-t^2)) ease-out,
       *     decelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeOut(const float& t);
      /**
       * Easing equation for a circular (sqrt(1-t^2)) ease-in/out,
       *     accelerating until halfway, then decelerating.
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeInOut(const float& t);
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
      POMPEIIENGINE_API
      float easeIn(const float& t);
      /**
       * Easing equation for a back (overshooting cubic easing:
       *     (s+1)*t^3 - s*t^2) ease-out, decelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeOut(const float& t);
      /**
       *  Easing equation for a back (overshooting cubic easing:
       *      (s+1)*t^3 - s*t^2) ease-in/out, accelerating until halfway,
       *      then decelerating.
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeInOut(const float& t);
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
      POMPEIIENGINE_API
      float easeIn(const float& t);
      /**
       * Easing equation for an elastic (exponentially decaying
       *     sine wave) ease-out, decelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeOut(const float& t);
      /**
       * Easing equation for an elastic (exponentially decaying
       *     sine wave) ease-out/in, decelerating until halfway,
       *     then accelerating.
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeInOut(const float& t);
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
      POMPEIIENGINE_API
      float easeIn(const float& t);
      /**
       * Easing equation for a bounce (exponentially decaying parabolic
       *     bounce) ease-out, decelerating from zero velocity.
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeOut(const float& t);
      /**
       * Easing equation for a bounce (exponentially decaying parabolic
       *     bounce) ease-in/out, accelerating until halfway, then decelerating.
       * @param  {number} t Time
       * @return {number}
       */
      POMPEIIENGINE_API
      float easeInOut(const float& t);
    }
  }
}

#endif /* __POMPEIIENGINE_EASING__ */
