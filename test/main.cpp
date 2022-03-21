

//#include <vld.h>
#define CATCH_CONFIG_MAIN
#include "3rd/Catch2/catch.hpp"

#include "ara/ara_def.h"

#ifdef ARA_WIN32_VC_VER
	FILE _iob[] = { *stdin, *stdout, *stderr };
	extern "C" FILE * __cdecl __iob_func(void) { return _iob; }
#endif
