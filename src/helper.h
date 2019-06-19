#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <chrono>

#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

namespace json = rapidjson;

static constexpr uint32_t BIT_TO_BYTES = 8;
static constexpr uint32_t BIT_TO_MIB = 1048576 * BIT_TO_BYTES;

void panic(const char *fmt, ...);

struct cmd_args
{
	std::string filename;
	uint prob_bits;
};

void read_args(int argc, char** argv, cmd_args& args);

template <typename T>
void read_file(const std::string& filename, std::vector<T>* tokens ){
	std::ifstream is (filename, std::ios_base::binary|std::ios_base::in);
	if (is) {
		// get length of file:
		is.seekg (0, is.end);
		size_t length = is.tellg();
		is.seekg (0, is.beg);//	    for(int i = 0; i<static_cast<int>(freqs.size()); i++){
		//	    	std::cout << i << ": " << i + min << " " << freqs[i] << " " << cum_freqs[i] << std::endl;
		//	    }
		//	    std::cout <<  cum_freqs.back() << std::endl;

		// reserve size of tokens
		if (!tokens){
			throw std::runtime_error("Cannot read file into nonexistent vector");
		}

		if (length % sizeof(T)){
			throw std::runtime_error("Filesize is not a multiple of datatype.");
		}
		// size the vector appropriately
		size_t num_elems = length / sizeof(T);
		tokens->resize(num_elems);

		// read data as a block:
		is.read (reinterpret_cast<char*>(tokens->data()),length);
		is.close();
	}
}

template<typename Decorated>
auto executionTimer(Decorated && function)
{
	const auto t0 = std::chrono::high_resolution_clock::now();

	function();

	const auto t1 = std::chrono::high_resolution_clock::now();
	return std::chrono::duration<double>(t1-t0);
}

enum class ExecutionMode{NonInterleaved,Interleaved};
enum class CodingMode{Encode,Decode};

std::string toString(ExecutionMode mode);
std::string toString(CodingMode mode);

template<typename Decorated>
void timedRun(json::PrettyWriter<json::StringBuffer>& runSummary, size_t sizeUncompressed, ExecutionMode executionMode, CodingMode codingMode, size_t numberOfRuns, Decorated && function)
{
	const std::string execModeStr = toString(executionMode);
	const std::string codingModeStr = toString(codingMode);
	// run benchmark a certain amount of times
	runSummary.Key(codingModeStr.c_str());
	runSummary.StartArray();
	std::cout << "Bandwidth " << codingModeStr << ": [";
	for (size_t run=0; run < numberOfRuns; run++) {
		auto duration = executionTimer(function);
		//		results.push_back(1.0 * (runSummary["NumberOfSymbols"].GetUint() * runSummary["SymbolRange"].GetUint())/ (duration.count() * BIT_TO_MIB)); //Bit -> MiB

		const double bandwidth = 1.0* (sizeUncompressed / (duration.count() * BIT_TO_MIB)); //Bit -> MiB
		runSummary.Double(bandwidth);
		std::cout << std::setprecision(4) << bandwidth;
		if (run<numberOfRuns-1){
			std::cout << ", ";
		}
	}
	runSummary.EndArray();
	std::cout << "] MiB/s" << std::endl;
}
