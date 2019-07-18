/*
 * DecSymbol.h
 *
 *  Created on: May 21, 2019
 *      Author: Michael Lettrich (michael.lettrich@cern.ch)
 */

#pragma once

#include <stdint.h>

namespace rans {

// Decoder symbols are straightforward.
struct DecoderSymbol
{
	// Initialize a decoder symbol to start "start" and frequency "freq"
	DecoderSymbol(uint32_t start, uint32_t freq, uint32_t probabilityBits):start(start), freq(freq)
	{

		//TODO(lettrich): a check should be definitely done here.
		//		RansAssert(start <= (1 << 16));
		//		RansAssert(freq <= (1 << 16) - start);
	};

	uint32_t start;     // Start of range.
	uint32_t freq;      // Symbol frequency.
};

}  // namespace rans



