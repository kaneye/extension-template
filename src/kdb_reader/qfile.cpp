#include "qfile.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>
#include "zlib.h"
#include <string.h>


#define PLUGIN_NAME ""

using namespace std;

//////////////////////////////////////////////////////////////////////////////

// zlib inflation parameters
constexpr int    ZLib_FORMAT_DETECT = 32;
constexpr size_t ZLib_CHUNK_SIZE    = 1 << 14;

//////////////////////////////////////////////////////////////////////////////

// kdb::BinFile::BinFile(const string& path, const string& filename)
//   : filename_{filename}, fp_{nullptr} {
//     fp_ = fopen(path.c_str(), "rb");
// }

kdb::BinFile::BinFile(FILE *fp)
  :filename_(""), fp_(fp) {}

kdb::BinFile::~BinFile() {
  fp_ = nullptr;
  //std::cout<<"mem_debug,~BinFile()"<<std::endl;
    // if(fp_) {
    //     fclose(fp_);
    // }
}

kdb::BinFile::operator bool() const {
    return fp_ && !ferror(fp_);
}

size_t kdb::BinFile::readInto(vector<byte>& buffer) {
    if(!fp_) {
        //throw RuntimeException(PLUGIN_NAME ": " + filename_ + " access error.");
        std::cout<<filename_ + " access error."<<std::endl;
    }

    fseek(fp_, 0, SEEK_SET);
    vector<char> header(MAGIC_BYTES, '\0');
    const auto read = fread(header.data(), 1, header.size(), fp_);
    if(read < MAGIC_BYTES) {
        //throw RuntimeException(PLUGIN_NAME ": " "Read " + filename_ + " header failed.");
        std::cout<<"Read " + filename_ + " header failed."<<std::endl;
    }

    const string magic{header.cbegin(), header.cend()};
    if(magic == "kxzipped") {
        return inflateBody(buffer);
    } else {
        return readAll(buffer);
    }
}

size_t kdb::BinFile::getFileLen() const {
    assert(fp_);
    const auto p = ftell(fp_);
    //Defer restore{[this, p](){ fseek(fp_, p, SEEK_SET); }};

    fseek(fp_, 0, SEEK_END);
    const auto len = ftell(fp_);
    if(len < 0) {
        //throw RuntimeException(PLUGIN_NAME ": " "Read " + filename_ + " failed.");
        std::cout<<"Read " + filename_ + " failed."<<std::endl;        
    }

    fseek(fp_, p, SEEK_SET);
    return static_cast<size_t>(len);
}

size_t kdb::BinFile::getBodyLen() const {
    const size_t fileLen = getFileLen();
    if(fileLen < MAGIC_BYTES) {
        //throw RuntimeException(PLUGIN_NAME ": " "Empty or truncated " + filename_ + ".");
        std::cout<<"Empty or truncated " + filename_ + "."<<std::endl;
    }
    return fileLen - MAGIC_BYTES;
}

size_t kdb::BinFile::readAll(vector<byte>& buffer, ptrdiff_t offset) {
    const size_t len = getFileLen();
    assert(fp_);
    if(static_cast<ptrdiff_t>(len) < offset) {
        //throw RuntimeException(PLUGIN_NAME ": " "Load " + filename_ + " too little data.");
        std::cout<<"Load " + filename_ + " too little data."<<std::endl;
    }

    const auto initLen = buffer.size();
    buffer.resize(initLen + len);
    fseek(fp_, offset, SEEK_SET);
    const auto read = fread(buffer.data() + initLen, 1, len, fp_);
    if(offset + read < len) {
        //throw RuntimeException(PLUGIN_NAME ": " "Load " + filename_ + " data incomplete.");
        std::cout<< "Load " + filename_ + " data incomplete."<<std::endl;
    }
    return len;
}

#define FILE_BOUNDARY_CHECK(condition)      \
    if(UNLIKELY(condition)) {               \
        std::cout<<"Parsing failed, exceeding buffer bound"<<std::endl; \
    }

// decode short
short rh(unsigned char* src, long long pos) {
    return ((short*)(src+pos))[0];
}

// decode int
int ri(unsigned char* src, long long pos) {
    return ((int*)(src+pos))[0];
}

// decode long
long long rl(unsigned char* src, long long pos) {
    return ((long long*)(src+pos))[0];
}

// decode double
double rd(unsigned char* src, long long pos) {
    long long num = rl(src, pos);
    return reinterpret_cast<double&>(num);
}

enum kdbCompressType {
    KDB_NO_COMPRESS = 0,
    KDB_COMPRESS_Q_IPC = 1,
    KDB_COMPRESS_GZIP = 2,
    KDB_COMPRESS_SNAPPY = 3,
    KDB_COMPRESS_LZ4HC = 4
};


//#include "snappy.h"
//#include "lz4.h"
#include <zlib.h>

long long decompressPlainText(unsigned char *src, size_t srcLen, unsigned char *dest, size_t destLen) {
    memcpy(dest, src, srcLen);
    return srcLen;
}

long long decompressQIpc(unsigned char *src, size_t srcLen, unsigned char *dest, size_t destLen) {
    //throw RuntimeException(PLUGIN_NAME "unsupported compress type: q IPC");
    std::cout<<"unsupported compress type: q IPC"<<std::endl;
}

// parameters for gzip process
#define WINDOWS_BITS 15
#define ENABLE_ZLIB_GZIP 32
#define GZIP_ENCODING 16

long long decompressGzip(unsigned char *src, size_t srcLen, unsigned char *dest, size_t destLen) {
    if(UNLIKELY(src == nullptr)) {
        //throw RuntimeException(PLUGIN_NAME "parse col failed.");
        std::cout<<"parse col failed."<<std::endl;
    }
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = src;
    strm.avail_in = srcLen;
    strm.next_out = dest;
    strm.avail_out = destLen;
    if (inflateInit2 (& strm, WINDOWS_BITS | ENABLE_ZLIB_GZIP) < 0){
        //throw RuntimeException(PLUGIN_NAME "gzip decompress: init inflate failed.");
        std::cout<<"gzip decompress: init inflate failed."<<std::endl;;
    }
    long long res = inflate (& strm, Z_NO_FLUSH);

    if (UNLIKELY(res < 0)){
        //throw RuntimeException(PLUGIN_NAME "gzip decompress: inflate failed.");
        std::cout<<"gzip decompress: inflate failed."<<std::endl;
    }
    inflateEnd (& strm);
    return destLen - strm.avail_out;
}


long long decompressSnappy(unsigned char *src, size_t srcLen, unsigned char *dest, size_t destLen) {
    std::cout<<"error,snappy"<<std::endl;
    return 0;
}


/*
long long decompressSnappy(unsigned char *src, size_t srcLen, unsigned char *dest, size_t destLen) {
    // string destStr;
    size_t length;
    bool getLengthSuccess = snappy::GetUncompressedLength(reinterpret_cast<char *>(src), srcLen, &length);
    if(!getLengthSuccess) {
        throw RuntimeException(PLUGIN_NAME "snappy get uncompressed length failed.");
    }

    bool success = snappy::RawUncompress(reinterpret_cast<char *>(src), srcLen, reinterpret_cast<char *>(dest));
    if(!success) {
        throw RuntimeException(PLUGIN_NAME "snappy decompress failed.");
    }
    return length;
}
*/

long long decompressLz4hc(unsigned char *src, size_t srcLen, unsigned char *dest, size_t destLen) {
/*    
    auto ret = LZ4_decompress_safe(reinterpret_cast<char *>(src), reinterpret_cast<char *>(dest), srcLen, destLen);
    if (ret < 0) {
        throw RuntimeException(PLUGIN_NAME "lz4hc decompress failed.");
    }
    return ret;
*/

    std::cout<<"error,decompressLz4hc"<<std::endl;
    return 0;
}

std::size_t kdb::BinFile::inflateBody(std::vector<byte>& buffer) {
    assert(fp_);
    const auto initLen = buffer.size();

    fseek(fp_, 0, SEEK_END);
    int64_t fileLen = ftell(fp_)-int64_t(MAGIC_BYTES);

    fseek(fp_, MAGIC_BYTES, SEEK_SET);
    vector<unsigned char> srcVec;
    srcVec.resize(fileLen);
    unsigned char *src = srcVec.data();
    size_t bytesRead = fread(src, 1, fileLen, fp_);
    FILE_BOUNDARY_CHECK((long long)bytesRead != fileLen)

    // read meta data of compressed file
    // read block num of compressed file
    FILE_BOUNDARY_CHECK(fileLen < int64_t(LONG_BYTES))
    int64_t blockSize = rl(src, fileLen-int64_t(LONG_BYTES));
    if (blockSize < 0 || fileLen < blockSize) {
        //throw RuntimeException(PLUGIN_NAME "invalid kxzipped blockSize " + std::to_string(blockSize));
        std::cout<<"invalid kxzipped blockSize " + std::to_string(blockSize)<<std::endl;
    }

    FILE_BOUNDARY_CHECK(fileLen < int64_t(LONG_BYTES * (1+4+blockSize)))
    int64_t bufPos = fileLen-int64_t(LONG_BYTES) * (1+4+blockSize);

    // read compress type&level
    bufPos+=4;
    // int compressType = src[bufPos];
    bufPos+=1;
    // int compressLevel = src[bufPos]; // currently no use
    // read uncompress size
    bufPos+=3;
    int64_t originSize = rl(src, bufPos);
    bufPos+=LONG_BYTES;
    // long long compressSize = rl(src, bufPos); // currently no use
    bufPos+=LONG_BYTES;
    int64_t originBlockSize = rl(src, bufPos);

    // read every compress block size
    vector<pair<size_t, size_t>> blockVec(blockSize);
    for(int64_t i = 0; i < blockSize; i++) {
        bufPos+=LONG_BYTES;
        FILE_BOUNDARY_CHECK(bufPos+ int64_t(LONG_BYTES) > fileLen)
        size_t len = ri(src, bufPos);
        size_t type = ri(src, bufPos+4);
        blockVec[i] = pair<size_t, size_t>{len, type};
    }

    buffer.resize(originBlockSize * blockSize);
    size_t offset = 0;
    for(int64_t i = 0; i < blockSize; i++) {
        int64_t decompressSize = -1;
        FILE_BOUNDARY_CHECK(srcVec.size() - (src- srcVec.data()) < blockVec[i].first);
        switch(blockVec[i].second) {
            case kdbCompressType::KDB_COMPRESS_Q_IPC:
                decompressSize = decompressQIpc(src, blockVec[i].first, buffer.data()+offset, originBlockSize);
                break;
            case kdbCompressType::KDB_COMPRESS_GZIP:
                decompressSize = decompressGzip(src, blockVec[i].first, buffer.data()+offset, originBlockSize);
                break;
            case kdbCompressType::KDB_COMPRESS_SNAPPY:
                decompressSize = decompressSnappy(src, blockVec[i].first, buffer.data()+offset, originBlockSize);
                break;
            case kdbCompressType::KDB_COMPRESS_LZ4HC:
                decompressSize = decompressLz4hc(src, blockVec[i].first, buffer.data()+offset, originBlockSize);
                break;
            case kdbCompressType::KDB_NO_COMPRESS:
            default:
                decompressSize = decompressPlainText(src, blockVec[i].first, buffer.data()+offset, originBlockSize);
        }

        if (UNLIKELY(decompressSize < 0)) {
            //throw RuntimeException(PLUGIN_NAME "invalid decompression.");
            std::cout<<"invalid decompression."<<std::endl;
        }
        src+=blockVec[i].first;
        offset+=decompressSize;
    }
    buffer.resize(offset);

    //PLUGIN_LOG_WARN("Expected decompressed file size: ", offset, ", Actually: ", originSize);

    assert(buffer.size() >= initLen);
    return buffer.size() - initLen;
}




//////////////////////////////////////////////////////////////////////////////
