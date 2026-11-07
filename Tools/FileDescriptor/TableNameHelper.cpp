#include "TableNameHelper.h"
#include "PathTool.h"
#include "Logger.h"
#include <QRegularExpression>

namespace Base32Helper{
constexpr const char* BASE32_ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

QString Encode(const QByteArray &data) {
  QString result;
  result.reserve((data.size() * 8 + 4) / 5);
  int buffer = 0;
  int bitsLeft = 0;
  for (char ch : data) {
    buffer = (buffer << 8) | (static_cast<unsigned char>(ch) & 0xFF);
    bitsLeft += 8;
    while (bitsLeft >= 5) {
      result.append(QLatin1Char(BASE32_ALPHABET[(buffer >> (bitsLeft - 5)) & 0x1F]));
      bitsLeft -= 5;
    }
  }
  if (bitsLeft > 0) {
    result.append(QLatin1Char(BASE32_ALPHABET[(buffer << (5 - bitsLeft)) & 0x1F]));
  }
  while (result.size() % 8 != 0) {
    result.append(QLatin1Char('='));
  }
  return result;
}

QByteArray Decode(const QString &input) {
  QByteArray result;
  int buffer = 0;
  int bitsLeft = 0;
  for (QChar ch : input) {
    if (ch == QLatin1Char('=') || ch == QLatin1Char('_')) {
      break;  // 填充符或已被替换为 _ 的填充符
    }
    const QChar upper = ch.toUpper();
    int val = -1;
    const ushort u = upper.unicode();
    if (u >= 'A' && u <= 'Z') {
      val = u - 'A';
    } else if (u >= '2' && u <= '7') {
      val = u - '2' + 26;
    }
    if (val < 0) {
      continue;  // 非法字符跳过（容错）
    }
    buffer = (buffer << 5) | val;
    bitsLeft += 5;
    if (bitsLeft >= 8) {
      result.append(static_cast<char>((buffer >> (bitsLeft - 8)) & 0xFF));
      bitsLeft -= 8;
    }
  }
  return result;
}
}


namespace TableNameHelper{
// 1.空路径代表总表 表名为 "MOVIE"
// 2 非空代表分表, 获取方法:
// 2.1 取path的文件夹名folderName, 去除[A-Za-z0~9]之外的字符得到folderNameNew
// 2.2 将path进行base32编码, 替换填充符"=", "+"等(以base64编码时的特殊符号为例)
// 2.3 folderNameNew+"_"+编码 即为所需

QString WorkPath2TableName(const QString &path) {
  if (path.isEmpty()) {
    return QStringLiteral("MOVIE");  // 总表
  }

  // 2.1 取文件夹名，去除 [A-Za-z0-9] 之外的字符
  QString folderName = PathTool::fileName(path);
  static const QRegularExpression kSanitize{"[^A-Za-z0-9]"};
  folderName.remove(kSanitize);
  // folderName 可能为空（文件夹名全是非法字符），此时表名以 "_" 开头，仍合法

  // 2.2 base32 编码，填充符 '=' 替换为 '_'，统一小写
  QString encoded = Base32Helper::Encode(path.toUtf8());
  encoded.replace(QLatin1Char('='), QLatin1Char('_'));
  encoded = encoded.toLower();

  // 2.3 folderNameNew + "_" + 编码
  return folderName + QLatin1Char('_') + encoded;
}

QString TableName2WorkPath(const QString &tableName, bool& isValid) {
  if (tableName == QLatin1String("MOVIE")) {
    isValid = true;
    return {};  // 总表
  }

  // folderName 里不含 '_'（已被 sanitize 移除），所以第一个 '_' 一定是分隔符
  const int sep = tableName.indexOf(QLatin1Char('_'));
  if (sep < 0) {
    LOG_W("[TableName2WorkPath] invalid table name: %s", qPrintable(tableName));
    isValid = false;
    return {};
  }
  isValid = true;

  QString encoded = tableName.mid(sep + 1);
  encoded.replace(QLatin1Char('_'), QLatin1Char('='));  // 还原填充符

  const QByteArray bytes = Base32Helper::Decode(encoded);
  return QString::fromUtf8(bytes);
}

}
