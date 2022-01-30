#include "console.h"

#include <fstream>
#include <stdio.h>
#include <string.h>

#include "../test/developerGui.h"

std::ofstream file;
bool useFile = false;

void console::openFile(std::string fileName) {
	file.open(fileName, std::ios_base::out | std::ios_base::trunc);
	useFile = true;
}

int console::print(const char* buffer, ...) {
	va_list argptr;
	va_start(argptr, buffer);
	
	#ifdef EGGINE_DEVELOPER_MODE
	va_list argptrCopy;
	va_copy(argptrCopy, argptr);
	vDeveloperPrint(buffer, argptrCopy);
	va_end(argptrCopy);
	#endif

	if(useFile) {
		va_list argsCopy;
		va_copy(argsCopy, argptr);
		char output[1024];
		vsnprintf(output, 1024, buffer, argsCopy);
		file.write(output, strlen(output));
		file.flush();
		va_end(argsCopy);
	}

	vprintf(buffer, argptr);

	va_end(argptr);

	return 0;
}

int console::warning(const char* buffer, ...) {
	va_list argptr;
	va_start(argptr, buffer);
	
	#ifdef EGGINE_DEVELOPER_MODE
	va_list argptrCopy;
	va_copy(argptrCopy, argptr);
	vDeveloperWarning(buffer, argptrCopy);
	va_end(argptrCopy);
	#endif

	if(useFile) {
		va_list argsCopy;
		va_copy(argsCopy, argptr);
		char output[1024];
		vsnprintf(output, 1024, buffer, argsCopy);
		file.write(output, strlen(output));
		file.flush();
		va_end(argsCopy);
	}

	vprintf(buffer, argptr);

	va_end(argptr);

	return 0;
}

int console::error(const char* buffer, ...) {
	va_list argptr;
	va_start(argptr, buffer);
	
	#ifdef EGGINE_DEVELOPER_MODE
	va_list argptrCopy;
	va_copy(argptrCopy, argptr);
	vDeveloperError(buffer, argptrCopy);
	va_end(argptrCopy);
	#endif

	if(useFile) {
		va_list argsCopy;
		va_copy(argsCopy, argptr);
		char output[1024];
		vsnprintf(output, 1024, buffer, argsCopy);
		file.write(output, strlen(output));
		file.flush();
		va_end(argsCopy);
	}

	vprintf(buffer, argptr);

	va_end(argptr);

	return 0;
}

int console::vprint(const char* buffer, va_list args) {
	#ifdef EGGINE_DEVELOPER_MODE
	va_list argsCopy;
	va_copy(argsCopy, args);
	vDeveloperPrint(buffer, argsCopy);
	va_end(argsCopy);
	#endif

	if(useFile) {
		va_list argsCopy;
		va_copy(argsCopy, args);
		char output[1024];
		vsnprintf(output, 1024, buffer, argsCopy);
		file.write(output, strlen(output));
		file.flush();
		va_end(argsCopy);
	}

	vprintf(buffer, args);

	return 0;
}

int console::vwarning(const char* buffer, va_list args) {
	#ifdef EGGINE_DEVELOPER_MODE
	va_list argsCopy;
	va_copy(argsCopy, args);
	vDeveloperWarning(buffer, argsCopy);
	va_end(argsCopy);
	#endif

	if(useFile) {
		va_list argsCopy;
		va_copy(argsCopy, args);
		char output[1024];
		vsnprintf(output, 1024, buffer, argsCopy);
		file.write(output, strlen(output));
		file.flush();
		va_end(argsCopy);
	}

	vprintf(buffer, args);

	return 0;
}

int console::verror(const char* buffer, va_list args) {
	#ifdef EGGINE_DEVELOPER_MODE
	va_list argsCopy;
	va_copy(argsCopy, args);
	vDeveloperError(buffer, argsCopy);
	va_end(argsCopy);
	#endif

	if(useFile) {
		va_list argsCopy;
		va_copy(argsCopy, args);
		char output[1024];
		vsnprintf(output, 1024, buffer, argsCopy);
		file.write(output, strlen(output));
		file.flush();
		va_end(argsCopy);
	}

	vprintf(buffer, args);

	return 0;
}
