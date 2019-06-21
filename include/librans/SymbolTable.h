/*
 * SymbolTable.h
 *
 *  Created on: Jun 21, 2019
 *      Author: Michael Lettrich (michael.lettrich@cern.ch)
 */

#pragma once

#include <vector>

#include "SymbolStatistics.h"


namespace rans {

template <typename T>
class SymbolTable {
public:
	explicit SymbolTable(const SymbolStatistics& symbolStats, uint64_t probabiltyBits): min_(symbolStats.minSymbol())
	{
		symbolTable_.reserve(symbolStats.size());

		for(const auto& entry: symbolStats){
			symbolTable_.emplace_back(entry.second, entry.first, probabiltyBits);
		}
	}

	const T& operator[](size_t index) const
	{

		return symbolTable_[index];
	}

private:
	int min_;
	std::vector<T> symbolTable_;
};
}  // namespace rans
