#ifndef _QFILE_H_
#define _QFILE_H_

#include <type_traits>
#include <string>
#include <vector>

#define byte unsigned char
#if defined(__GNUC__) && __GNUC__ >= 4
#define LIKELY(x) (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#endif


namespace kdb {

class BinFile {
  public:
    static constexpr std::size_t MAGIC_BYTES = 8;
    constexpr static std::size_t LONG_BYTES = 8;
    constexpr static std::size_t INT_BYTES = 4;

  public:
    //BinFile(const std::string &path, const std::string &filename);
    BinFile(FILE *fp);
    ~BinFile();

    operator bool() const;

    std::size_t readInto(std::vector<byte> &buffer);

  public:
    std::size_t getFileLen() const;
    std::size_t getBodyLen() const;

    std::size_t readAll(std::vector<byte> &buffer, std::ptrdiff_t offset = 0);
    // std::size_t inflateBody(
    //     std::vector<byte>& buffer);
    std::size_t inflateBody(std::vector<byte> &buffer);

  private:
    std::string filename_;
    FILE *fp_;

    };//class BinFile

}//namespace kdb

#endif//_QFILE_H_