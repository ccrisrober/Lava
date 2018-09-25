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

#ifndef __POMPEIIENGINE_UPDATE_COMPONENTS__
#define __POMPEIIENGINE_UPDATE_COMPONENTS__

#include "LambdaVisitor.h"
#include <pompeiiEngine/api.h>

#include <pompeiiEngine/Clock.h>

namespace pompeii
{
	namespace engine
	{
	  class UpdateComponents :
	    public LambdaVisitor
	  {
	  public:
	    POMPEIIENGINE_API
	    UpdateComponents( const pompeii::engine::Clock& clock );
	  };
	}
}

#endif /* __POMPEIIENGINE_UPDATE_COMPONENTS__ */
