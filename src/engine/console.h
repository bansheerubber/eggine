#pragma once

#include <stdarg.h>

namespace console {
	int print(const char* buffer, ...);
	int warning(const char* buffer, ...);
	int error(const char* buffer, ...);

	int vprint(const char* buffer, va_list args);
	int vwarning(const char* buffer, va_list args);
	int verror(const char* buffer, va_list args);
};
