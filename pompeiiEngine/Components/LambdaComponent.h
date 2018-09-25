/**
 * Copyright (c) 2017 - 2018, Pompeii
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

#ifndef __POMPEIIENGINE_LAMBDA_COMPONENT__
#define __POMPEIIENGINE_LAMBDA_COMPONENT__

#include "Component.h"
#include <pompeiiEngine/api.h>
#include <functional>

namespace pompeii
{
  namespace engine
  {
    typedef std::function< void( mb::Node*, const /*mb::Clock&*/float clock ) > LambdaCallback;
    class LambdaComponent
      : public Component
    {
      IMPLEMENT_COMPONENT( LambdaComponent )
    public:
      POMPEIIENGINE_API
      LambdaComponent( LambdaCallback cb );
      //MB_API
      //virtual void update( const mb::Clock& clock ) override;
    protected:
      LambdaCallback _callback;
    };
  }
}

#endif /* __POMPEIIENGINE_LAMBDA_COMPONENT__ */
