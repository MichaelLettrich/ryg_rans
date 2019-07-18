/*
 * executionTimer.h
 *
 *  Created on: Jul 1, 2019
 *      Author: Michael Lettrich (michael.lettrich@cern.ch)
 */

#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <chrono>

#include "rapidjson/document.h"
namespace json = rapidjson;

#include "definitions.h"

enum class ExecutionMode{NonInterleaved,Interleaved};
enum class CodingMode{Encode,Decode};

std::string toString(ExecutionMode mode);
std::string toString(CodingMode mode);


template<typename Decorated>
auto executionTimer(Decorated && function)
{
	const auto t0 = std::chrono::high_resolution_clock::now();

	function();

	const auto t1 = std::chrono::high_resolution_clock::now();
	return std::chrono::duration<double>(t1-t0);
}

template<typename Decorated>
json::Value timedRun(json::Document::AllocatorType& runSummaryAllocator, size_t sizeUncompressed, ExecutionMode executionMode, CodingMode codingMode, size_t numberOfRuns, Decorated && function)
{
	const std::string execModeStr = toString(executionMode);
	const std::string codingModeStr = toString(codingMode);

	json::Value bandwidths(json::kArrayType);
	std::cout << "Bandwidth " << codingModeStr << ": [";

	// run benchmark a certain amount of times
	for (size_t run=0; run < numberOfRuns; run++) {
		auto duration = executionTimer(function);
		//		results.push_back(1.0 * (runSummary["NumberOfSymbols"].GetUint() * runSummary["SymbolRange"].GetUint())/ (duration.count() * BIT_TO_MIB)); //Bit -> MiB

		const double bandwidth = 1.0* (sizeUncompressed / (duration.count() * BIT_TO_MIB)); //Bit -> MiB
		bandwidths.PushBack(bandwidth,runSummaryAllocator);
		std::cout << std::setprecision(4) << bandwidth;
		if (run<numberOfRuns-1){
			std::cout << ", ";
		}
	}
	std::cout << "] MiB/s" << std::endl;
	return std::move(bandwidths);
}

