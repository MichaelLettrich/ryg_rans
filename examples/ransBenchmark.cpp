#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/prettywriter.h"

#include "docopt.h"

#include "librans/rans.h"

#include "libcommon/executionTimer.h"
#include "libcommon/helper.h"

#ifndef SOURCE_T
#define SOURCE_T uint8_t
#endif

// This is just the sample program. All the meat is in rans_byte.h.
namespace json = rapidjson;
using source_t = SOURCE_T;
static const uint REPETITIONS = 5;

////////////////////////////////////////////////////////////////
// use this definition for 32bit coder/decoder
#ifdef rans32
static const uint PROB_BITS = 14;
using coder_t = uint32_t;
using stream_t = uint8_t;
using Rans = rans::Coder<coder_t, stream_t>;
// using RansEncSymbol = rans::EncoderSymbol<coder_t>;
#else
////////////////////////////////////////////////////////////////
// use this definition for 64bit coder/decoder
static const uint PROB_BITS = 18;
using coder_t = uint64_t;
using stream_t = uint32_t;
using Rans = rans::Coder<coder_t, stream_t>;
// using RansEncSymbol = rans::EncoderSymbol<coder_t>;
#endif
////////////////////////////////////////////////////////////////

static const char USAGE[] =
    R"(ransBenchmark.

        Usage:
          ransBenchmark
          ransBenchmark <fileName> [-s <samples>] [-b <bits>] [-r <dict>] [-d <dict>] [-e <createdDict>] [-l <log> ]
          ransBenchmark (-h | --help)
          ransBenchmark --version

        Options:
          -h --help                         Show this screen.
          --version                         Show version.
          -s <samples> --samples <samples>  How many times do we repeat the measurements.
          -b <bits> --bits <bits>           Resample dictionary to Bits.
          -r <bits> --range <bits>          Range of the source data
          -d <dict> --dict <dict>           Dictionary.
          -e <path> --export <path>         Export dictionary.
          -l <log> --log <log>              Log in JSON format.    

    )";

int main(int argc, char* argv[]) {
  json::Document runSummary;
  runSummary.SetObject();

  ///////////////////////////////////////////////////////////////////////////////////////////
  // Parsing arguments
  //////////////////////////////////////////////////////////////////////////////////////////
  auto args =
      docopt::docopt(USAGE, {argv + 1, argv + argc}, true, "ransBenchmark-dev");

  const std::string filename = [&]() {
    if (args["<fileName>"].isString()) {
      return args["<fileName>"].asString();
    } else {
      const std::string name{"book1"};
      return name;
    }
  }();

  const uint32_t prob_bits = [&]() {
    try {
      return static_cast<uint32_t>(args["--bits"].asLong());
    } catch (std::runtime_error& e) {
      return PROB_BITS;
    }
  }();

  uint32_t symbolRangeBits = [&]() {
    try {
      return static_cast<uint32_t>(args["--range"].asLong());
    } catch (std::runtime_error& e) {
      return static_cast<uint32_t>(0);
    }
  }();

  const std::string dictPath = [&]() {
    if (args["--dict"].isString()) {
      return args["--dict"].asString();
    } else {
      return std::string();
    }
  }();

  const std::string exportDictPath = [&]() {
    if (args["--export"].isString()) {
      return args["--export"].asString();
    } else {
      return std::string();
    }
  }();

  const uint32_t repetitions = [&]() {
    try {
      return static_cast<uint32_t>(args["--samples"].asLong());
    } catch (std::runtime_error& e) {
      return REPETITIONS;
    }
  }();

  const std::string logPath = [&]() {
    if (args["--log"].isString()) {
      return args["--log"].asString();
    } else {
      return std::string("summary.json");
    }
  }();

  //////////////////////////////////////////////////////////////////////////////////////////

  const uint32_t prob_scale = 1 << prob_bits;

  std::cout << "Filename: " << filename << std::endl;
  std::cout << "Probability Bits: " << prob_bits << std::endl;
  std::cout << "Dictionary Path: " << dictPath << std::endl;
  std::cout << "Export Path: " << exportDictPath << std::endl;
  std::cout << "Repetitions: " << repetitions << std::endl;

  runSummary.AddMember(
      "Filename",
      json::Value().SetString(filename.c_str(), runSummary.GetAllocator()),
      runSummary.GetAllocator());
  runSummary.AddMember("ProbabilityBits", prob_bits, runSummary.GetAllocator());

  // Read in file to be compressed
  std::vector<source_t> tokens;
  read_file(filename, &tokens);
  std::cout << std::endl;
  std::cout << "Symbols:" << tokens.size() << std::endl;
  runSummary.AddMember("NumberOfSymbols", tokens.size(),
                       runSummary.GetAllocator());

  std::unique_ptr<rans::SymbolStatistics> stats(nullptr);
  // read in or create dictionary
  if (dictPath.empty()) {
    // no dict path, create tokens.
    stats = std::make_unique<rans::SymbolStatistics>(tokens, symbolRangeBits);
  } else {
    // open file
    std::ifstream dictFile(dictPath);
    json::IStreamWrapper dictReader(dictFile);
    json::Document statsJSON;
    statsJSON.ParseStream(dictReader);
    stats = std::make_unique<rans::SymbolStatistics>(statsJSON.GetObject());
  }

  stats->rescaleFrequencyTable(prob_scale);
  symbolRangeBits = stats->getSymbolRangeBits();
  std::cout << "Min: " << stats->minSymbol() << " Max: " << stats->maxSymbol()
            << " Range: " << symbolRangeBits << "Bit" << std::endl;

  runSummary.AddMember("SymbolRange", symbolRangeBits,
                       runSummary.GetAllocator());

  const size_t out_max_size = 256 << 20;  // 256MB
  const size_t out_max_elems = out_max_size / sizeof(stream_t);
  std::vector<stream_t> out_buf(out_max_elems);
  const stream_t* out_end = &out_buf.back();
  std::vector<source_t> dec_bytes(tokens.size(), 0xcc);

  // cumlative->symbol table
  // this is super brute force
  std::vector<source_t> cum2sym(prob_scale);
  // go over all symbols
  for (int symbol = stats->minSymbol(); symbol < stats->maxSymbol() + 1;
       symbol++)
    for (uint32_t cumulative = (*stats)[symbol].second;
         cumulative < (*stats)[symbol + 1].second; cumulative++) {
      cum2sym[cumulative] = symbol;
      //      std::cout << "cum2sym[" << cumulative << "]: " << symbol;
    }

  stream_t* rans_begin = nullptr;

  rans::SymbolTable<rans::EncoderSymbol<coder_t>> encoderSymbolTable(*stats,
                                                                     prob_bits);
  rans::SymbolTable<rans::DecoderSymbol> decoderSymbolTable(*stats, prob_bits);

  std::cout << "Source Size :"
            << static_cast<uint32_t>(std::ceil(1.0 * tokens.size() *
                                               symbolRangeBits / BYTE_TO_BITS))
            << " Bytes" << std::endl;

  // ---- regular rANS encode/decode. Typical usage.
  std::cout << std::endl << "Non-Interleaved:" << std::endl;
  json::Value nonInterleaved(json::kObjectType);

  nonInterleaved.AddMember(
      "Encode",
      timedRun(runSummary.GetAllocator(), symbolRangeBits * tokens.size(),
               ExecutionMode::NonInterleaved, CodingMode::Encode, repetitions,
               [&]() {
                 rans::State<coder_t> rans;
                 Rans::encInit(&rans);

                 stream_t* ptr =
                     const_cast<stream_t*>(out_end);  // *end* of output buffer
                 for (size_t i = tokens.size(); i > 0;
                      i--) {  // NB: working in reverse!
                   source_t s = tokens[i - 1];
                   //            std::cout << "s: " << s << ", esyns[" <<
                   //            normalized << "]: " << esyms[normalized].freq
                   //            << std::endl; Rans32::encPut(&rans, &ptr,
                   //            stats->cum_freqs[normalized],
                   //            stats->freqs[normalized], prob_bits);
                   Rans::encPutSymbol(&rans, &ptr, &encoderSymbolTable[s],
                                      prob_bits);
                 }
                 Rans::encFlush(&rans, &ptr);
                 rans_begin = ptr;
               }),
      runSummary.GetAllocator());

  nonInterleaved.AddMember(
      "Decode",
      timedRun(runSummary.GetAllocator(), symbolRangeBits * tokens.size(),
               ExecutionMode::NonInterleaved, CodingMode::Decode, repetitions,
               [&]() {
                 rans::State<coder_t> rans;
                 stream_t* ptr = rans_begin;
                 Rans::decInit(&rans, &ptr);

                 for (size_t i = 0; i < tokens.size(); i++) {
                   source_t s = cum2sym[Rans::decGet(&rans, prob_bits)];
                   dec_bytes[i] = s;
                   //            std::cout << "s: " << s << ", dsyms[" <<
                   //            normalized << "]: " << dsyms[normalized].freq
                   //            << std::endl;
                   Rans::decAdvanceSymbol(&rans, &ptr, &decoderSymbolTable[s],
                                          prob_bits);
                 }
               }),
      runSummary.GetAllocator());

  unsigned int encodeSize =
      static_cast<unsigned int>(&out_buf.back() - rans_begin) *
      sizeof(stream_t);
  std::cout << "Encode Size :" << encodeSize << " Bytes" << std::endl;
  nonInterleaved.AddMember("Size", encodeSize, runSummary.GetAllocator());

  runSummary.AddMember("NonInterleaved", nonInterleaved,
                       runSummary.GetAllocator());

  // check decode results
  if (memcmp(tokens.data(), dec_bytes.data(),
             tokens.size() * sizeof(source_t)) == 0)
    printf("Decoder passed tests.\n");
  else
    printf("ERROR: Decoder failed tests.\n");

  // ---- interleaved rANS encode/decode. This is the kind of thing you might do
  // to optimize critical paths.

  memset(dec_bytes.data(), 0xcc, tokens.size() * sizeof(source_t));

  // try interleaved rANS encode
  std::cout << std::endl << "Interleaved:" << std::endl;
  json::Value interleaved(json::kObjectType);

  interleaved.AddMember(
      "Encode",
      timedRun(runSummary.GetAllocator(), symbolRangeBits * tokens.size(),
               ExecutionMode::Interleaved, CodingMode::Encode, repetitions,
               [&]() {
                 rans::State<coder_t> rans0, rans1;
                 Rans::encInit(&rans0);
                 Rans::encInit(&rans1);

                 stream_t* ptr = const_cast<stream_t*>(out_end);

                 // odd number of bytes?
                 if (tokens.size() & 1) {
                   const int s = tokens.back();
                   Rans::encPutSymbol(&rans0, &ptr, &encoderSymbolTable[s],
                                      prob_bits);
                   //            Rans::encPut(&rans0, &ptr,
                   //            stats->cum_freqs[normalized],
                   //            stats->freqs[normalized], prob_bits);
                 }

                 for (size_t i = (tokens.size() & ~1); i > 0;
                      i -= 2) {  // NB: working in reverse!
                   const int s1 = tokens[i - 1];
                   const int s0 = tokens[i - 2];
                   Rans::encPutSymbol(&rans1, &ptr, &encoderSymbolTable[s1],
                                      prob_bits);
                   Rans::encPutSymbol(&rans0, &ptr, &encoderSymbolTable[s0],
                                      prob_bits);
                   //            Rans::encPut(&rans1, &ptr,
                   //            stats->cum_freqs[normalized1],
                   //            stats->freqs[normalized1], prob_bits);
                   //            Rans::encPut(&rans0, &ptr,
                   //            stats->cum_freqs[normalized0],
                   //            stats->freqs[normalized0], prob_bits);
                 }
                 Rans::encFlush(&rans1, &ptr);
                 Rans::encFlush(&rans0, &ptr);
                 rans_begin = ptr;
               }),
      runSummary.GetAllocator());

  interleaved.AddMember(
      "Decode",
      timedRun(runSummary.GetAllocator(), symbolRangeBits * tokens.size(),
               ExecutionMode::Interleaved, CodingMode::Decode, repetitions,
               [&]() {
                 rans::State<coder_t> rans0, rans1;
                 stream_t* ptr = rans_begin;
                 Rans::decInit(&rans0, &ptr);
                 Rans::decInit(&rans1, &ptr);

                 for (size_t i = 0; i < (tokens.size() & ~1); i += 2) {
                   const uint32_t s0 = cum2sym[Rans::decGet(&rans0, prob_bits)];
                   const uint32_t s1 = cum2sym[Rans::decGet(&rans1, prob_bits)];
                   dec_bytes[i + 0] = s0;
                   dec_bytes[i + 1] = s1;
                   Rans::decAdvanceSymbolStep(&rans0, &decoderSymbolTable[s0],
                                              prob_bits);
                   Rans::decAdvanceSymbolStep(&rans1, &decoderSymbolTable[s1],
                                              prob_bits);
                   Rans::decRenorm(&rans0, &ptr);
                   Rans::decRenorm(&rans1, &ptr);
                 }

                 // last byte, if number of bytes was odd
                 if (tokens.size() & 1) {
                   const uint32_t s0 = cum2sym[Rans::decGet(&rans0, prob_bits)];
                   dec_bytes[tokens.size() - 1] = s0;
                   Rans::decAdvanceSymbol(&rans0, &ptr, &decoderSymbolTable[s0],
                                          prob_bits);
                 }
               }),
      runSummary.GetAllocator());

  encodeSize = static_cast<unsigned int>(&out_buf.back() - rans_begin) *
               sizeof(stream_t);
  std::cout << "Encode Size :" << encodeSize << " Bytes" << std::endl;
  interleaved.AddMember("Size", encodeSize, runSummary.GetAllocator());
  runSummary.AddMember("Interleaved", interleaved, runSummary.GetAllocator());

  // check decode results
  if (memcmp(tokens.data(), dec_bytes.data(),
             tokens.size() * sizeof(source_t)) == 0)
    printf("Decoder passed tests.\n");
  else
    printf("ERROR: Decoder failed tests.\n");

  std::ofstream f(logPath);
  json::OStreamWrapper osw(f);
  json::PrettyWriter<json::OStreamWrapper> writer(osw);
  runSummary.Accept(writer);

  // shall we serialize the symbol statistics?
  if (!exportDictPath.empty()) {
    json::Document d;
    std::ofstream f(exportDictPath);
    json::OStreamWrapper osw(f);
    json::PrettyWriter<json::OStreamWrapper> writer(osw);
    stats->serialize(d.GetAllocator()).Accept(writer);
  }

  return 0;
}
