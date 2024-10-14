#pragma once

#include <Windows.h>
#include <string>

class ApplicationError {

private:
	HRESULT errorCode;
	std::string fileName;
	std::string functionName;
	int lineNumber;

public:
	ApplicationError(HRESULT _hr = 0, std::string _fileName = "", std::string _functionName = "", int _lineNumber = 0);
};

#ifndef ThrowAppError
#define ThrowAppError() {											\
	throw ApplicationError(0, __func__, __FILE__, __LINE__);		\
}
#endif

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)															\
{																					\
    HRESULT hr = (x);																\
    if(FAILED(hr)) { throw ApplicationError(hr, __func__, __FILE__, __LINE__); }	\
}
#endif											


