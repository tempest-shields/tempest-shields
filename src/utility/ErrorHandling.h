/* 
 * File:   ErrorHandling.h
 * Author: Gereon Kremer
 *
 * Created on March 15, 2013, 4:10 PM
 */

#ifndef ERRORHANDLING_H
#define	ERRORHANDLING_H

#include "src/utility/OsDetection.h"
#include <signal.h>


/*
 * Demangles the given string. This is needed for the correct display of backtraces.
 *
 * @param symbol The name of the symbol that is to be demangled.
 */
std::string demangle(char const* symbol) {
	int status;
    
	// Attention: sscanf format strings rely on the size being 128.
	char temp[128];
    
	// Check for C++ symbol, on Non-MSVC Only
	if (sscanf(symbol, "%*[^(]%*[^_]%127[^)+]", temp) == 1) {
#ifndef WINDOWS
		char* demangled;
		if (NULL != (demangled = abi::__cxa_demangle(temp, NULL, NULL, &status))) {
			std::string result(demangled);
			free(demangled);
			return result;
		}
#else
	DWORD  error;
	HANDLE hProcess;

	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);

	hProcess = GetCurrentProcess();

	if (!SymInitialize(hProcess, NULL, TRUE)) {
		// SymInitialize failed
		error = GetLastError();
		LOG4CPLUS_ERROR(logger, "SymInitialize returned error : " << error);
		return FALSE;
	} else {
		char demangled[1024];
		if (UnDecorateSymbolName(temp, demangled, sizeof(demangled), UNDNAME_COMPLETE)) {
			return std::string(demangled);
		} else {
			// UnDecorateSymbolName failed
			DWORD error = GetLastError();
			LOG4CPLUS_ERROR(logger, "UnDecorateSymbolName returned error: " << error);
		}
	}
#endif
	}

	// Check for C symbol.
	if (sscanf(symbol, "%127s", temp) == 1) {
		return temp;
	}
    
	// Return plain symbol if none of the above cases matched.
	return symbol;
}

/*
 * Handles the given signal. This will display the received signal and a backtrace.
 *
 * @param sig The code of the signal that needs to be handled.
 */
void signalHandler(int sig) {
#define SIZE 128
	LOG4CPLUS_FATAL(logger, "The program received signal " << sig << ". The following backtrace shows the status upon reception of the signal.");

#ifndef WINDOWS
	void *buffer[SIZE];
 	char **strings;
	int nptrs;
	nptrs = backtrace(buffer, SIZE);

    // Try to retrieve the backtrace symbols.
	strings = backtrace_symbols(buffer, nptrs);
	if (strings == nullptr) {
		std::cerr << "Obtaining the backtrace symbols failed." << std::endl;
		exit(2);
	}
    
    // Starting this for-loop at j=2 means that we skip the handler itself. Currently this is not
    // done.
	for (int j = 1; j < nptrs; j++) {
		LOG4CPLUS_FATAL(logger, nptrs-j << ": " << demangle(strings[j]));
	}
	free(strings);
#else
	LOG4CPLUS_WARN(logger, "No Backtrace Support available on Platform Windows!");
#endif
	LOG4CPLUS_FATAL(logger, "Exiting.");
	exit(2);
}

/*
 * Registers some signal handlers so that we can display a backtrace upon erroneuous termination.
 */
void installSignalHandler() {
	signal(SIGSEGV, signalHandler);
}

#endif	/* ERRORHANDLING_H */
