#ifndef TAIKO_TOOL_GZIP_H
#define TAIKO_TOOL_GZIP_H

#include <sstream>
#include <fstream>
#include <iostream>
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

        boost::iostreams::close(output);

        return result.str();
    }

    static void compress(const std::string &data, const std::string &outFile) {
        std::stringstream decompressed(data);
        std::ofstream outStream(outFile, std::ios_base::out);

        boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
        in.push(boost::iostreams::gzip_compressor());
        in.push(decompressed);
        boost::iostreams::copy(in, outStream);

        boost::iostreams::close(in);
    }
}
#endif //TAIKO_TOOL_GZIP_H
