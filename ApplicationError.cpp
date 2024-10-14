
#include "ApplicationError.hpp"

ApplicationError::ApplicationError(
	HRESULT _hr,
	std::string _fileName,
	std::string _functionName,
	int _lineNumber
) {

	this->errorCode = _hr;
	this->fileName = _fileName;
	this->functionName = _functionName;
	this->lineNumber = _lineNumber;

}

