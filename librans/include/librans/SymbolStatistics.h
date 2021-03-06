/*
 * SymbolStats.h
 *
 *  Created on: May 8, 2019
 *      Author: Michael Lettrich (michael.lettrich@cern.ch)
 *
 */
#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <vector>

#include "rapidjson/document.h"

namespace json = rapidjson;

namespace rans {

class SymbolStatistics {
 public:
  class Iterator {
   public:
    Iterator(size_t index, const SymbolStatistics& stats);

    const Iterator& operator++();

    std::pair<uint32_t, uint32_t> operator*() const;

    bool operator!=(const Iterator& other) const;

   private:
    size_t index_;
    const SymbolStatistics& stats_;
  };

 public:
  template <typename T>
  explicit SymbolStatistics(const std::vector<T>& tokens, size_t range = 0)
      : min_(0), max_(0), frequencyTable_(), cumulativeFrequencyTable_() {
    buildFrequencyTable(tokens, range);
    buildCumulativeFrequencyTable();
  }

  explicit SymbolStatistics(const json::Value& document);

  ~SymbolStatistics() = default;
  SymbolStatistics(const SymbolStatistics& stats) = default;
  SymbolStatistics(SymbolStatistics&& stats) = default;
  SymbolStatistics& operator=(const SymbolStatistics& stats) = default;
  SymbolStatistics& operator=(SymbolStatistics&& stats) = default;

  void rescaleFrequencyTable(uint32_t newCumulatedFrequency);

  size_t getSymbolRangeBits() const;

  int minSymbol() const;
  int maxSymbol() const;

  size_t size() const;

  std::pair<uint32_t, uint32_t> operator[](size_t index) const;

  SymbolStatistics::Iterator begin() const;
  SymbolStatistics::Iterator end() const;

  json::Value serialize(json::Document::AllocatorType& allocator) const;

  inline static const std::string FREQUENCY_TABLE_STR = "FrequencyTable";
  inline static const std::string MIN_STR = "min";
  inline static const std::string MAX_STR = "max";

 private:
  void buildCumulativeFrequencyTable();

  template <typename T>
  void buildFrequencyTable(const std::vector<T>& symbols, size_t range);

  int min_ = 0;
  int max_ = 0;
  std::vector<uint32_t> frequencyTable_;
  std::vector<uint32_t> cumulativeFrequencyTable_;
};

template <typename T>
void SymbolStatistics::buildFrequencyTable(const std::vector<T>& tokens,
                                           size_t range) {
  // find min_ and max_
  const auto minmax = std::minmax_element(tokens.begin(), tokens.end());

  if (range > 0) {
    min_ = 0;
    max_ = (1 << range) - 1;

    // do checks
    if (static_cast<unsigned int>(min_) > *minmax.first) {
      throw std::runtime_error("min of data too small for given minimum");
    }

    if (static_cast<unsigned int>(max_) < *minmax.second) {
      throw std::runtime_error("max of data too big for given maximum");
    }
  } else {
    min_ = *minmax.first;
    max_ = *minmax.second;
  }

  frequencyTable_.resize(std::abs(max_ - min_) + 1, 0);

  for (auto token : tokens) {
    frequencyTable_[token - min_]++;
  }
}

}  // namespace rans
