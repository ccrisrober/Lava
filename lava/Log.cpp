#include "Log.h"

#include "MemoryUtils.h"

namespace lava
{
	std::unique_ptr< Log::OutputHandler > Log::_outputHandler =
		make_unique< Log::ConsoleOutputHandler>();
	int Log::_level = Log::LogLevel::LOG_LEVEL_INFO;

  void Log::ConsoleOutputHandler::print( const std::string& src )
  {
    std::cout << src << std::endl;
  }
  void Log::FileOutputHandler::print( const std::string& src )
  {
    _out << src << std::endl;
  }
  void Log::NullOutputHandler::print( const std::string& )
  {
  }
}
