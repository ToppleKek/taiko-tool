#ifndef TAIKO_TOOL_GZIP_H
#define TAIKO_TOOL_GZIP_H

#include <sstream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/copy.hpp>

namespace GZip {
    static std::string decompress(const std::string &data) {
        std::stringstream compressed(data);
        std::stringstream result;

        boost::iostreams::filtering_streambuf<boost::iostreams::input> output;
        output.push(boost::iostreams::gzip_decompressor());
        output.push(compressed);
        boost::iostreams::copy(output, result);

        return result.str();
    }

    static std::string compress(const std::string &data) {
        std::stringstream compressed;
        std::stringstream decompressed(data);

        boost::iostreams::filtering_streambuf<boost::iostreams::input> output;
        output.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(boost::iostreams::gzip::best_compression)));
        output.push(decompressed);
        boost::iostreams::copy(output, compressed);

        return compressed.str();
    }
}
#endif //TAIKO_TOOL_GZIP_H
