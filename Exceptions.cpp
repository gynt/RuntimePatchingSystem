#include "Exceptions.h"


char errorReport[1000] = {};
char intHex[4 + 1] = {};
static const char* const errorReportTooLong = "error report is too longer, longer than 1000 characters";

int errorFilterAndReporter(unsigned int code, struct _EXCEPTION_POINTERS* ep)
{
	std::stringstream errorReportBuilder;
	errorReportBuilder << "Exception occurred: code: 0x" << std::hex << code << " at 0x" << std::hex << ep->ExceptionRecord->ExceptionAddress;
	std::string r = errorReportBuilder.str();
	const char* data = r.c_str();
	size_t size = strnlen_s(data, 1000);
	if (size < 1000) {
		// memset((void*)errorReport, 0, 1000);
		memcpy((void*)errorReport, data, size + 1);
	}
	else {
		memcpy((void*)errorReport, (void*)errorReportTooLong, strnlen_s(errorReportTooLong, 1000));
	}
	return EXCEPTION_EXECUTE_HANDLER;
}