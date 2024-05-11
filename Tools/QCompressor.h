#ifndef QCOMPRESSOR_H
#define QCOMPRESSOR_H

#include <zlib.h>
#include <QByteArray>

constexpr int GZIP_WINDOWS_BIT = 15 + 16;
constexpr int GZIP_CHUNK_SIZE = 32 * 1024;

class QCompressor {
 public:
  static bool originCompress(const QByteArray& data, QByteArray& output, int level = -1);
  static bool gzipCompress(QByteArray input, QByteArray& output, int level = -1);
  static bool gzipDecompress(QByteArray input, QByteArray& output);
};

#endif  // QCOMPRESSOR_H
