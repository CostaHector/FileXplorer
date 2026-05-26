#include "Par2Tools.h"
#include "Logger.h"
#include <QCryptographicHash>
#include <QFile>
#include <QDir>
#include <QRegularExpression>

const QByteArray PKT_PACK{"PAR2\x00PKT", 8};
const QByteArray MAIN_PACK{"PAR 2.0\x00Main\x00\x00\x00\x00", 16};
const QByteArray BUILD_IN_SRC_FILE{QByteArray("PAR 2.0\x00", 8) + QByteArray("FileDesc", 8)};

namespace Par2Tools {

QString ChopPostfixVolAndPar2(QString fileName) {
  // Another file need backup 2.txt.par2
  // Another file need backup 2.txt.vol0+1.par2
  // Another file need backup 2.txt.vol1+1.par2
  static const QRegularExpression suffixPattern{R"(\.vol\d+\+\d+\.par2|\.par2)"};
  return fileName.remove(suffixPattern);
}

bool Par2Info::isTrustable() const {
  if (setId.isEmpty()) {
    return false;
  }
  if (oldFileNames.size() != sourceFileCount) {
    LOG_W("src file count[%d] declared in Main Pack not match built-in src file list size[%d]", oldFileNames.size(), sourceFileCount);
    return false;
  }
  if (sourceFileCount != 1) {
    LOG_W("src file count should be exactly 1, but[%d]", sourceFileCount);
    return false;
  }
  return true;
}

std::pair<bool, QStringList> CheckInputParms(const QString& folderPath, const QString& par2FileName, const QStringList& volPar2Files) {
  if (!par2FileName.endsWith(".par2", Qt::CaseInsensitive)) {
    LOG_D("File[%s] is not par2", qPrintable(par2FileName));
    return {false, {}};
  }

  const QDir par2Dir(folderPath);
  if (!par2Dir.exists()) {
    LOG_D("Folder[%s] not exist", qPrintable(folderPath));
    return {false, {}};
  }

  if (volPar2Files.isEmpty()) {
    LOG_D("vol.X+Y.par2 filesList empty");
    return {false, {}};
  }

  QStringList par2FileList;
  par2FileList.push_back(par2Dir.absoluteFilePath(par2FileName));
  for (const QString& volPar2Files : volPar2Files) {
    par2FileList.push_back(par2Dir.absoluteFilePath(volPar2Files));
  }

  foreach (QString par2FileAbsPath, par2FileList) {
    if (!QFile::exists(par2FileAbsPath)) {
      LOG_D("file[%s] inexist", qPrintable(par2FileAbsPath));
      return {false, {}};
    }
  }
  return {true, par2FileList};
}

Par2Info ParsePar2File(const QString &par2FileAbsPath) {
  QByteArray setId;         // PAR2 集合唯一标识
  QStringList oldFileNames; // 原内嵌源文件名列表
  int sourceFileCount = 0;  // Main 包中声明的源文件数量

  // 打开第一个 PAR2 文件,  分块读取文件（初始读取 2MB，与原 Python 一致）
  QFile firstPar2File{par2FileAbsPath};
  if (!firstPar2File.open(QIODevice::ReadOnly)) {
    LOG_D("Read PAR2[%s] files failed", qPrintable(par2FileAbsPath));
    return {std::move(setId), std::move(oldFileNames), sourceFileCount};
  }
  QByteArray fileData = firstPar2File.read(2 * 1024 * 1024);

  int dataSize = fileData.size();
  int offset = 0;

  // 遍历解析 PAR2 包
  while (offset + 64 < dataSize) {
    // 校验包头部：PAR2\x00PKT
    if (fileData.mid(offset, 8) == PKT_PACK) {
      // 解析包大小（8字节 uint64_t，本机字节序）
      uint64_t packetSize = 0;
      memcpy(&packetSize, fileData.constData() + offset + 8, sizeof(uint64_t));

      // 包大小超出缓冲区，跳过
      if (offset + packetSize > dataSize) {
        offset += 8;
        continue;
      }

      // 校验包 MD5（32字节到包尾的 MD5 需与 16-32 字节一致）
      QByteArray packetData = fileData.mid(offset + 32, packetSize - 32);
      QByteArray md5Hash = QCryptographicHash::hash(packetData, QCryptographicHash::Md5);
      QByteArray packetHash = fileData.mid(offset + 16, 16);
      if (md5Hash == packetHash) {
        // 提取 SetID（32-48 字节）
        if (setId.isEmpty()) {
          setId = fileData.mid(offset + 32, 16);
          LOG_D("PAR2 SetID[%s]", setId.toHex().constData());
        } else if (setId != fileData.mid(offset + 32, 16)) {
          // SetID 不一致则跳过该包
          offset += packetSize;
          continue;
        }

        // 解析包类型（48-64 字节）
        QByteArray packetType = fileData.mid(offset + 48, 16);
        if (packetType == MAIN_PACK) {
          // Main 包：提取源文件数量（72字节 uint32_t）
          uint32_t fileCount = 0;
          memcpy(&fileCount, fileData.constData() + offset + 72, sizeof(uint32_t));
          sourceFileCount = fileCount;
          LOG_D("src files count[%d]", sourceFileCount);
          if (sourceFileCount == 0) {
            break;
          }
          if (oldFileNames.count() >= sourceFileCount) {
            break;
          }
        } else if (packetType == BUILD_IN_SRC_FILE) {
          // FileDesc 包：提取内嵌文件名（120字节开始，去除末尾空字节）
          int nameEnd = offset + packetSize;
          while (nameEnd > offset + 120 && fileData.at(nameEnd - 1) == 0) {
            nameEnd--;
          }
          QByteArray nameBytes = fileData.mid(offset + 120, nameEnd - offset - 120);
          QString fileName = QString::fromUtf8(nameBytes);

          // 去重后加入列表
          if (oldFileNames.contains(fileName)) {
            offset += packetSize;
            continue;
          }
          oldFileNames.append(fileName);
          LOG_D("resolve built-in file[%s]", fileName.toUtf8().constData());

          // 达到声明数量则停止解析, 获取oldFileNames元素优先于sourceFileCount
          if (oldFileNames.count() == sourceFileCount) {
            break;
          }
        }
        offset += packetSize;
      } else {
        LOG_D("Pack MD5 Check failed Deviate %d", offset);
        offset += 8;
      }
    } else {
      offset++;
    }
    // 偏移超过 1MB 时，读取下一块数据（与原 Python 逻辑一致）
    if (offset >= 1024 * 1024) {
      fileData = fileData.mid(offset) + firstPar2File.read(1024 * 1024);
      dataSize = fileData.size();
      offset = 0;
    }
  }
  return {std::move(setId), std::move(oldFileNames), sourceFileCount};;
}

QString GetSyncPar2FileAbsPath(const QString& par2FileAbsPath) {
  return par2FileAbsPath + SYNC_PAR2_POSTFIX;
}

int ModifyPar2FilesFileDesc(const QStringList& par2FileList, const QStringList& newFileNames, const Par2Info& par2Info) {
  int renamePacketCount{0};

  for (int par2FileIndex = 0; par2FileIndex < par2FileList.size(); ++par2FileIndex) {
    const QString& srcPath = par2FileList[par2FileIndex];
    const QString dstPath{GetSyncPar2FileAbsPath(srcPath)};

    // 打开源文件和目标文件
    QFile srcFile{srcPath};
    if (!srcFile.open(QIODevice::ReadOnly)) {
      LOG_D("Open srcFile failed skip[%s]", qPrintable(srcPath));
      return false;
    }
    QFile dstFile{dstPath};
    if (!dstFile.open(QIODevice::WriteOnly)) {
      LOG_D("Open target file failed skip[%s]", qPrintable(dstPath));
      return false;
    }

    // 分块读取源文件
    QByteArray srcData = srcFile.read(2 * 1024 * 1024);
    int srcDataSize = srcData.size();
    int srcOffset = 0;

    while (srcOffset + 64 < srcDataSize) {
      if (srcData.mid(srcOffset, 8) == PKT_PACK) {
        // 解析包大小
        uint64_t packetSize = 0;
        memcpy(&packetSize, srcData.constData() + srcOffset + 8, sizeof(uint64_t));

        if (srcOffset + packetSize > srcDataSize) {
          // 包大小超出缓冲区，写入前8字节并跳过
          dstFile.write(srcData.mid(srcOffset, 8));
          srcOffset += 8;
          continue;
        }

        // 校验包 MD5
        QByteArray packetData = srcData.mid(srcOffset + 32, packetSize - 32);
        QByteArray md5Hash = QCryptographicHash::hash(packetData, QCryptographicHash::Md5);
        QByteArray packetHash = srcData.mid(srcOffset + 16, 16);
        if (md5Hash != packetHash) {
          dstFile.write(srcData.mid(srcOffset, 8));
          srcOffset += 8;
          continue;
        }

        // 校验 SetID
        QByteArray currentSetId = srcData.mid(srcOffset + 32, 16);
        if (currentSetId != par2Info.setId) {
          // SetID 不一致，直接写入原包
          dstFile.write(srcData.mid(srcOffset, packetSize));
          srcOffset += packetSize;
          continue;
        }

        // 处理 FileDesc 包
        QByteArray packetType = srcData.mid(srcOffset + 48, 16);
        int nameIndex = -1;
        if (packetType == BUILD_IN_SRC_FILE) {
          // 提取原文件名并查找索引
          int nameEnd = srcOffset + packetSize;
          while (nameEnd > srcOffset + 120 && srcData.at(nameEnd - 1) == 0) {
            nameEnd--;
          }
          QByteArray oldNameBytes = srcData.mid(srcOffset + 120, nameEnd - srcOffset - 120);
          QString oldName = QString::fromUtf8(oldNameBytes);
          nameIndex = par2Info.oldFileNames.indexOf(oldName);
        }

        if (nameIndex >= 0) {
          // ========== 修改 FileDesc 包 ==========
          QString newName = newFileNames.at(nameIndex);
          QByteArray newNameBytes = newName.toUtf8();
          int newNameLen = newNameBytes.size();

          // 文件名按 4 字节对齐（填充空字节）
          while (newNameLen % 4 != 0) {
            newNameBytes.append(char{0});
            newNameLen++;
          }

          // 构建新包
          QByteArray newPacket = srcData.mid(srcOffset, 120); // 前120字节复用
          newPacket.append(newNameBytes);                     // 写入新文件名
          uint64_t newPacketSize = newPacket.size();
          newPacket.replace(8, 8, QByteArray((const char*) &newPacketSize, 8)); // 更新包大小

          // 重新计算 MD5 并更新
          QByteArray newPacketData = newPacket.mid(32);
          QByteArray newMd5Hash = QCryptographicHash::hash(newPacketData, QCryptographicHash::Md5);
          newPacket.replace(16, 16, newMd5Hash);

          // 写入新包
          dstFile.write(newPacket);
          renamePacketCount++;
          srcOffset += packetSize;
        } else {
          // 非 FileDesc 包，直接写入原数据
          dstFile.write(srcData.mid(srcOffset, packetSize));
          srcOffset += packetSize;
        }
      } else {
        // 非 PAR2 包头部，写入单个字节
        dstFile.write(srcData.mid(srcOffset, 1));
        srcOffset++;
      }

      // 偏移超过 1MB 时读取下一块
      if (srcOffset >= 1024 * 1024) {
        srcData = srcData.mid(srcOffset) + srcFile.read(1024 * 1024);
        srcDataSize = srcData.size();
        srcOffset = 0;
      }
    }

    // 写入剩余数据
    if (srcOffset < srcDataSize) {
      dstFile.write(srcData.mid(srcOffset));
    }
  }

  return renamePacketCount;
}

bool SyncBuiltInSrcFileListInPar2(const QString& folderPath, const QString& par2FileName, const QStringList& volPar2Files) {
  bool bCheckedOk{false};
  QStringList par2FileList;
  std::tie(bCheckedOk, par2FileList) = CheckInputParms(folderPath, par2FileName, volPar2Files);
  if (!bCheckedOk) {
    return false;
  }

  const Par2Info par2Info = ParsePar2File(par2FileList.front());
  if (!par2Info.isTrustable()) {
    return false;
  }

  // 生成新的内嵌文件名（替换基名）
  const QStringList newFileNames{ChopPostfixVolAndPar2(par2FileName)};

  const int renamePacketCount{ModifyPar2FilesFileDesc(par2FileList, newFileNames, par2Info)};
  if (renamePacketCount == 0) {
    LOG_D("no change made to %d PAR2 files(s)", par2FileList.count());
    return false;
  }

  LOG_D("Success modify %d pack, including %d PAR2 file(s)", renamePacketCount, par2FileList.count());
  return true;
}

}
