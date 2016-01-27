
#ifndef ARA_INTERNAL_STRFORMAT_H
#define ARA_INTERNAL_STRFORMAT_H

#include <string>
#include <type_traits>

namespace ara {
	template<class T> 
	struct format_append {
		format_append(T & stream) : stream_(stream) {}

		void	append()

		T & stream_;
	};


	template<class T>
	struct format_append<T, typename std::enable_if<is_string<T>::value>::type> {
		format_append(T & str) : str_(str) {}

		T & str_;
	};

	template<class T>
	struct str_format 
	{
	
	};
}

#endif//ARA_INTERNAL_STRFORMAT_H
