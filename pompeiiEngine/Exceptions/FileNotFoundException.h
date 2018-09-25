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

#ifndef __POMPEIIENGINE_FILE_NOT_FOUND_EXCEPTION__
#define __POMPEIIENGINE_FILE_NOT_FOUND_EXCEPTION__

#include <pompeiiEngine/api.h>

#include "Exception.h"

namespace pompeii
{
	namespace engine
	{
	  class FileNotFoundException: public Exception
	  {
	  public:
	    POMPEIIENGINE_API
	    FileNotFoundException( std::string filePath )
	    : Exception( "Cannot find file " + filePath )
	    {
	    }
	  };
	}
}

#endif /* __POMPEIIENGINE_FILE_NOT_FOUND_EXCEPTION__ */
