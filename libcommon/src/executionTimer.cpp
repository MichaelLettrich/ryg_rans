/*
 * executionTimer.cpp
 *
 *  Created on: Jul 1, 2019
 *      Author: Michael Lettrich (michael.lettrich@cern.ch)
 */

#include <stdexcept>

#include "libcommon/executionTimer.h"

std::string toString(ExecutionMode mode)
{

	switch (mode) {
		case ExecutionMode::NonInterleaved:
			return "NonInterleaved";
			break;
		case ExecutionMode::Interleaved:
			return "Interleaved";
			break;
		default:
			throw std::runtime_error("unknown ExecutionMode");
			break;
	}
}

std::string toString(CodingMode mode){
	switch (mode) {
		case CodingMode::Encode:
			return "Encode";
			break;
		case CodingMode::Decode:
			return "Decode";
			break;
		default:
			throw std::runtime_error("unknown CodingMode");
			break;
	}
}
