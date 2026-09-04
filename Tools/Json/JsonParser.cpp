#include "JsonParser.h"
#include "FileTool.h"
#include "MD5Calculator.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <cstring>

namespace JsonParser {
static constexpr int INDENT_SPACE_CNT = 4;  // 顶层字段固定缩进空格数

// 跳过所有空白字符（空格、换行、回车、制表符）
inline void skipWhitespace(const QByteArray& data, int& pos) {
  const int total = data.size();
  const char* ptr = data.constData();
  while (pos < total) {
    const char c = ptr[pos];
    if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
      ++pos;
    } else {
      break;
    }
  }
}

// 从pos位置（起始双引号处）解析JSON字符串，pos移动到结束双引号之后
// 未处理转义字符，适配无转义的固定格式JSON
inline QString parseString(const QByteArray& data, int& pos) {
  const int total = data.size();
  const char* ptr = data.constData();
  if (pos >= total || ptr[pos] != '"') return {};

  ++pos; // 跳过起始双引号
  const int start = pos;
  while (pos < total && ptr[pos] != '"') {
    ++pos;
  }
  if (pos >= total) return {};

  QString result = QString::fromUtf8(ptr + start, pos - start);
  ++pos; // 跳过结束双引号
  return result;
}

inline QByteArray parseMD5ByteArray(const QByteArray& data, int& pos) {
  const int total = data.size();
  const char* ptr = data.constData();
  // 起始位置必须是双引号
  if (pos >= total || ptr[pos] != '"') {
    return {};
  }
  ++pos; // 跳过起始双引号

  // 空 MD5 字符串 ""
  if (pos < total && ptr[pos] == '"') {
    ++pos;
    return {};
  }

  // MD5 固定 32 位十六进制字符，后续必须紧跟结束双引号
  constexpr int MD5_LEN = MD5Calculator::FIXED_MD5_LENGTH; // 可替换为
  if (pos + MD5_LEN >= total) {
    return {}; // 剩余长度不足 32 位 + 结束引号
  }
  if (ptr[pos + MD5_LEN] != '"') {
    return {}; // 长度不匹配，非标准 MD5 格式
  }
  QByteArray md5Result(ptr + pos, MD5_LEN);
  pos += MD5_LEN + 1; // 跳过 32 位字符 + 结束双引号
  return md5Result;
}

// 解析有符号64位整数，pos移动到数字末尾之后
template<typename NumberType>
NumberType parseNumber(const QByteArray& data, int& pos) {
  skipWhitespace(data, pos);
  const int total = data.size();
  const char* ptr = data.constData();
  if (pos >= total) return 0;

  bool negative = false;
  if (ptr[pos] == '-') {
    negative = true;
    ++pos;
  }

  NumberType num = 0;
  while (pos < total && ptr[pos] >= '0' && ptr[pos] <= '9') {
    num = num * 10 + (ptr[pos] - '0');
    ++pos;
  }
  return negative ? -num : num;
}

extern template int parseNumber<int>(const QByteArray& data, int& pos);
extern template qint64 parseNumber<qint64>(const QByteArray& data, int& pos);

template int parseNumber<int>(const QByteArray& data, int& pos);
template qint64 parseNumber<qint64>(const QByteArray& data, int& pos);

// 解析字符串数组，支持多行/任意缩进，pos移动到右方括号之后
inline QStringList parseStringArray(const QByteArray& data, int& pos) {
  QStringList result;
  skipWhitespace(data, pos);
  const int total = data.size();
  const char* ptr = data.constData();
  if (pos >= total || ptr[pos] != '[') return result;

  ++pos; // 跳过左方括号
  while (pos < total) {
    skipWhitespace(data, pos);
    if (ptr[pos] == ']') {
      ++pos;
      break;
    }

    if (ptr[pos] == '"') {
      result.append(parseString(data, pos));
    }

    skipWhitespace(data, pos);
    if (ptr[pos] == ',') {
      ++pos; // 跳过逗号
    } else if (ptr[pos] == ']') {
      continue; // 下一轮闭合数组
    } else {
      break; // 格式异常，终止解析
    }
  }
  return result;
}

JsonOp::ResultE ValidateSampleMd5AndVidName(const int localSampleMD5Size, const int localVidNameSize) {
  // 2个空
  if (localSampleMD5Size == 0 && localVidNameSize == 0) {
    return JsonOp::ResultE::IGNORE_NO_NEED_FURTHER_PROCESS;
  }
  // 有且仅有1个空 => 原因: 未计算MD5的json, 手动删去了vidName
  if (localSampleMD5Size == 0 || localVidNameSize == 0) {
    return JsonOp::ResultE::ERROR;
  }
  return JsonOp::ResultE::OK_NEED_FURTHER_PROCESS;
}

JsonOp::ResultE ParseEssentialFieldJson(const QString& jsonFilePath,
                             QByteArray* pSampleMd5Val, QString* pName, QString* pVidName,
                             qint64* pSize, int* pDuration,
                             QString* pStudio, QStringList* pCasts, QStringList* pTags,
                             int* pRate, QString* pDetail) {
  bool readOk = false;
  const QByteArray rawData = FileTool::ByteArrayReader(jsonFilePath, &readOk);
  if (!readOk || rawData.isEmpty()) {
    return JsonOp::ResultE::ERROR;
  }

  bool bNameMet = false;
  QString localVidName;
  QByteArray localSampleMD5;

  const int totalSize = rawData.size();
  const char* const rawPtr = rawData.constData();
  int pos = 0;

  while (pos < totalSize) {
    // 1. 定位当前行结尾，兼容 \n 和 \r\n
    int lineEnd = rawData.indexOf('\n', pos);
    if (lineEnd == -1) {
      lineEnd = totalSize; // 最后一行无换行
    }
    const int lineLen = lineEnd - pos;

    // 2. 快速判断是否为顶层键行：长度足够 + 前4个是空格 + 第5个是双引号
    if (lineLen < INDENT_SPACE_CNT + 2 && std::memcmp(rawPtr + pos, R"(    ")", 5) != 0) {
      // 非顶层键行（数组元素行、闭合括号行、空行等），直接跳过整行
      pos = lineEnd + 1;
      continue;
    }

    // 3. 提取键名起止位置，零拷贝匹配
    const int keyStart = pos + INDENT_SPACE_CNT + 1; // 跳过缩进和起始引号
    const int keyEnd = rawData.indexOf('"', keyStart);
    if (keyEnd == -1 || keyEnd >= lineEnd) {
      pos = lineEnd + 1;
      continue;
    }
    const int keyLen = keyEnd - keyStart;

    // 4. 定位值起始位置：跳过结束引号、冒号、后续空白
    int valuePos = keyEnd + 1;
    // 固定格式下冒号紧跟引号，此处做极简兼容
    if (valuePos < totalSize && rawPtr[valuePos] == ':') {
      ++valuePos;
    }
    skipWhitespace(rawData, valuePos);

    // 5. 按需解析目标字段，非目标字段直接跳整行
    if (keyLen == 3 && std::memcmp(rawPtr + keyStart, "MD5", 3) == 0) {
      localSampleMD5 = parseMD5ByteArray(rawData, valuePos);
    } if (!bNameMet && keyLen == 4 && std::memcmp(rawPtr + keyStart, "Name", 4) == 0) {
      bNameMet = true;
      if (pName) {
        *pName = parseString(rawData, valuePos);
      }
    } else if (keyLen == 7 && std::memcmp(rawPtr + keyStart, "VidName", 7) == 0) {
      localVidName = parseString(rawData, valuePos);
    } else if (pSize && keyLen == 4 && std::memcmp(rawPtr + keyStart, "Size", 4) == 0) {
      *pSize = parseNumber<qint64>(rawData, valuePos);
    } else if (pDuration && keyLen == 8 && std::memcmp(rawPtr + keyStart, "Duration", 8) == 0) {
      *pDuration = parseNumber<int>(rawData, valuePos);
    } else if (pStudio && keyLen == 6 && std::memcmp(rawPtr + keyStart, "Studio", 6) == 0) {
      *pStudio = parseString(rawData, valuePos);
    } else if (pCasts && keyLen == 4 && std::memcmp(rawPtr + keyStart, "Cast", 4) == 0) {
      *pCasts = parseStringArray(rawData, valuePos);
    } else if (pTags && keyLen == 4 && std::memcmp(rawPtr + keyStart, "Tags", 4) == 0) {
      *pTags = parseStringArray(rawData, valuePos);
    } else if (pRate && keyLen == 4 && std::memcmp(rawPtr + keyStart, "Rate", 4) == 0) {
      *pRate = parseNumber<int>(rawData, valuePos);
    } else if (pDetail && keyLen == 6 && std::memcmp(rawPtr + keyStart, "Detail", 6) == 0) {
      *pDetail = parseString(rawData, valuePos);
    }
    pos = lineEnd + 1;
  }


  if (!bNameMet) {
    return JsonOp::ResultE::IGNORE_NO_NEED_FURTHER_PROCESS;
  }

  const auto checkResult = ValidateSampleMd5AndVidName(localSampleMD5.size(), localVidName.size());
  if (checkResult != JsonOp::ResultE::OK_NEED_FURTHER_PROCESS) {
    return checkResult;
  }

  if (pSampleMd5Val) {
    pSampleMd5Val->swap(localSampleMD5);
  }
  if (pVidName) {
    pVidName->swap(localVidName);
  }
  return JsonOp::ResultE::OK_NEED_FURTHER_PROCESS;
}

int GetDurationFromJsonFile(const QString& jsonFullPath, bool* bSucceed, int defaultDurationValue) {
  bool bReadResult{false};
  QByteArray contents{FileTool::ByteArrayReader(jsonFullPath, &bReadResult)};
  if (!bReadResult) {
    if (bSucceed != nullptr) { *bSucceed = false; }
    return defaultDurationValue;
  }
  int durationIndex = contents.indexOf(R"("Duration":)");
  if (durationIndex == -1) {
    if (bSucceed != nullptr) { *bSucceed = false; }
    return defaultDurationValue;
  }
  durationIndex += sizeof(R"("Duration":)"); // "a": xxx
  if (bSucceed != nullptr) { *bSucceed = true; }
  return parseNumber<int>(contents, durationIndex);
}

int GetRateFromJsonFile(const QString& jsonFullPath, int defaultRateValue) {
  bool bReadResult{false};
  QByteArray contents{FileTool::ByteArrayReader(jsonFullPath, &bReadResult)};
  if (!bReadResult) {
    return defaultRateValue;
  }
  int rateIndex = contents.indexOf(R"("Rate":)");
  if (rateIndex == -1) {
    return defaultRateValue;
  }
  rateIndex += sizeof(R"("Rate":)");
  return parseNumber<int>(contents, rateIndex);
}

qint64 GetSizeFromJsonFile(const QString& jsonFullPath, qint64 defaultSizeValue) {
  bool bReadResult{false};
  QByteArray contents{FileTool::ByteArrayReader(jsonFullPath, &bReadResult)};
  if (!bReadResult) {
    return defaultSizeValue;
  }
  int sizeIndex = contents.indexOf(R"("Size":)");
  if (sizeIndex == -1) {
    return defaultSizeValue;
  }
  sizeIndex += sizeof(R"("Size":)");
  return parseNumber<qint64>(contents, sizeIndex);
}

QByteArray GetMD5FromJsonFile(const QString& jsonFullPath) {
  bool bReadResult{false};
  QByteArray contents{FileTool::ByteArrayReader(jsonFullPath, &bReadResult)};
  if (!bReadResult) {
    return {};
  }
  int md5Index = contents.indexOf(R"("MD5":)");
  if (md5Index == -1) {
    return {};
  }
  md5Index += sizeof(R"("MD5":)");
  return parseMD5ByteArray(contents, md5Index);
}

QStringList GetTagsFromJsonFile(const QString& jsonFullPath) {
  bool bReadResult{false};
  QByteArray contents{FileTool::ByteArrayReader(jsonFullPath, &bReadResult)};
  if (!bReadResult) {
    return {};
  }
  int tagsIndex = contents.indexOf(R"("Tags":)");
  if (tagsIndex == -1) {
    return {};
  }
  tagsIndex += sizeof(R"("Tags":)");
  return parseStringArray(contents, tagsIndex);
}

std::pair<int, QStringList> GetRateAndTagsFromJsonFile(const QString& jsonFullPath, int defaultRateValue) {
  bool bReadResult{false};
  QByteArray contents{FileTool::ByteArrayReader(jsonFullPath, &bReadResult)};
  if (!bReadResult) {
    return {defaultRateValue, {}};
  }
  int rateIndex = contents.indexOf(R"("Rate":)");
  if (rateIndex == -1) {
    return {defaultRateValue, {}};
  }
  rateIndex += sizeof(R"("Rate":)");
  int tagsIndex = contents.indexOf(R"("Tags":)");
  if (tagsIndex == -1) {
    return {};
  }
  tagsIndex += sizeof(R"("Tags":)");
  return {parseNumber<int>(contents, rateIndex), parseStringArray(contents, tagsIndex)};
}

}