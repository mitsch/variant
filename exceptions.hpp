/// @file exceptions.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __VARIANT_EXCEPTIONS_HPP__
#define __VARIANT_EXCEPTIONS_HPP__

#include <stdexcept>

namespace variant
{

	class bad_variant_access : public std::logic_error
	{
		public:
			bad_variant_access(const std::string & reason) : std::logic_error(reason) {}
			bad_variant_access(const char * reason) : std::logic_error(reason) {}
	};

}

#endif

