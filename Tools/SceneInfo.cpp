#include "SceneInfo.h"
#include "PathTool.h"
#include <QFileInfo>
#include <QFile>
#include <QDirIterator>
#include <QSaveFile>
#include <QVariantHash>

constexpr quint32 SceneInfo::MAGIC_NUMBER;  // "LMSC" = "Local Media Scene Cache"
constexpr quint16 SceneInfo::CURRENT_VERSION;
constexpr quint16 SceneInfo::MIN_SUPPORTED_VERSION;

SceneInfo SceneInfo::fromJsonVariantHash(const QVariantHash& varHash) {
  return {
      "",                                           //
      varHash.value("Name", "").toString(),         //
      varHash.value("ImgName", "").toStringList(),  //
      varHash.value("VidName", "").toString(),      //
      varHash.value("Size", 0).toULongLong(),       //
      varHash.value("Rate", 0).toInt(),             //
      varHash.value("Uploaded", "").toString(),     //
  };
}

QString SceneInfo::GetAbsolutePath(const QString& rootPath) const {
  return rootPath + rel2scn;
}

QString SceneInfo::GetFirstImageAbsPath(const QString& rootPath) const {
  return PathTool::GetAbsFilePathFromRootRelName(rootPath, rel2scn, (imgs.isEmpty() ? "" : imgs.front()));
}

QStringList SceneInfo::GetImagesAbsPathList(const QString& rootPath) const {
  QStringList imgsAbsPathList;
  imgsAbsPathList.reserve(imgs.size());
  for (const QString& imgName : imgs) {
    imgsAbsPathList.append(PathTool::GetAbsFilePathFromRootRelName(rootPath, rel2scn, imgName));
  }
  return imgsAbsPathList;
}

QString SceneInfo::GetVideoAbsPath(const QString& rootPath) const {
  return PathTool::GetAbsFilePathFromRootRelName(rootPath, rel2scn, (vidName.isEmpty() ? name : vidName));
}

QString SceneInfo::GetJsonAbsPath(const QString& rootPath) const {
  return PathTool::GetAbsFilePathFromRootRelName(rootPath, rel2scn, name + ".json");
}

SceneInfo::CompareFunc SceneInfo::getCompareFunc(SceneSortOrderHelper::SortDimE dim) {
  using namespace SceneSortOrderHelper;
  switch (dim) {
    case SortDimE::MOVIE_PATH:
      return &SceneInfo::operator<;
    case SortDimE::MOVIE_SIZE:
      return &SceneInfo::lessThanVidSize;
    case SortDimE::RATE:
      return &SceneInfo::lessThanRate;
    case SortDimE::UPLOADED_TIME:
      return &SceneInfo::lessThanUploaded;
    default:
      LOG_D("Sort Dimension[%s] not support", c_str(dim));
      return &SceneInfo::lessThanName;
  }
}

bool SceneInfo::operator<(const SceneInfo& other) const {
  return rel2scn != other.rel2scn ? rel2scn < other.rel2scn : name < other.name;
}

bool SceneInfo::operator==(const SceneInfo& rhs) const {
  return rel2scn == rhs.rel2scn && name == rhs.name && imgs == rhs.imgs && vidName == rhs.vidName && vidSize == rhs.vidSize && rate == rhs.rate &&
         uploaded == rhs.uploaded;
}

bool SceneInfo::lessThanName(const SceneInfo& other) const {
  return name < other.name;
}

bool SceneInfo::lessThanVidSize(const SceneInfo& other) const {
  return vidSize < other.vidSize;
}

bool SceneInfo::lessThanRate(const SceneInfo& other) const {
  return rate < other.rate;
}

bool SceneInfo::lessThanUploaded(const SceneInfo& other) const {
  return uploaded < other.uploaded;
}

bool SceneInfo::GetNameFromStream(QDataStream& stream) {
  stream >> rel2scn >> name;
  return stream.status() == QDataStream::Ok;
}
bool SceneInfo::DeviateStreamFromNameToRateAndOverrideRate(QDataStream& stream, int newRate) {
  stream >> imgs >> vidName >> vidSize;
  if (stream.status() != QDataStream::Ok) {
    LOG_W("Deviate pointer from name to rate failed");
    return false;
  }
  stream << newRate;
  return stream.status() == QDataStream::Ok;
}

namespace SceneHelper {
SceneInfoList GetScnsLstFromPath(const QString& path) {
  if (!QFileInfo(path).isDir()) {
    LOG_D("path[%s] is not a directory", qPrintable(path));
    return {};
  }
  const int PATH_N = path.size();

  SceneInfoList scnTotals;
  int scnFilesCnt = 0;
  QDirIterator jsonIt(path, {"*.scn"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (jsonIt.hasNext()) {
    const QString& scnFullPath{jsonIt.next()};
    const QString& rel2JsonFile = PathTool::GetRelPathFromRootRelName(PATH_N, scnFullPath);
    scnTotals += ParseAScnFile(scnFullPath, rel2JsonFile);
    ++scnFilesCnt;
  }
  std::sort(scnTotals.begin(), scnTotals.end());
  LOG_D("total %d scenes get from %d *.scn file(s)", scnTotals.size(), scnFilesCnt);
  return scnTotals;
}

SceneInfoList ParseAScnFile(const QString& scnFileFullPath, const QString& rel) {
  QFile scnFi{scnFileFullPath};
  if (!scnFi.exists()) {
    LOG_D("scn file[%s] not exist", qPrintable(scnFileFullPath));
    return {};
  }
  if (!scnFi.open(QIODevice::ReadOnly)) {
    LOG_C("Open scn file[%s] to read failed", qPrintable(scnFi.fileName()));
    return {};
  }

  QDataStream stream(&scnFi);
  stream.setVersion(QDataStream::Qt_5_15);

  // 检查文件头
  auto magicNumberInFileHeader = SceneInfo::MAGIC_NUMBER;
  stream >> magicNumberInFileHeader;
  if (magicNumberInFileHeader != SceneInfo::MAGIC_NUMBER) {
    LOG_W("Invalid file format or corrupted file[%s]", qPrintable(scnFileFullPath));
    return {};
  }

  auto fileVersion = SceneInfo::MIN_SUPPORTED_VERSION;
  stream >> fileVersion;
  if (fileVersion < SceneInfo::MIN_SUPPORTED_VERSION) {
    LOG_W("Unsupported file version[%d] at least[%d] needed in file[%s]", fileVersion, SceneInfo::MIN_SUPPORTED_VERSION, qPrintable(scnFileFullPath));
    return {};
  }

  int scenesCount{0};
  stream >> scenesCount;

  if (scenesCount == 0) {
    return {};
  }

  SceneInfoList scenesList;
  scenesList.reserve(scenesCount);

  for (int i = 0; i < scenesCount && !stream.atEnd(); ++i) {
    SceneInfo scene;
    stream >> scene;
    scene.rel2scn = rel;

    if (stream.status() != QDataStream::Ok) {
      LOG_W("Error reading scene data at index %d from file[%s], status:%d", i, qPrintable(scnFileFullPath), stream.status());
      break;
    }

    scenesList.append(scene);
  }

  scnFi.close();
  LOG_D("Read %d scenes out from file[%s] succeed", scenesList.size(), qPrintable(scnFileFullPath));
  return scenesList;
}

bool SaveScenesListToBinaryFile(const QString& scnAbsFilePath, const SceneInfoList& scenes) {
  QSaveFile saveFi(scnAbsFilePath);  // QSaveFile提供原子写入
  if (!saveFi.open(QIODevice::WriteOnly)) {
    LOG_W("Open file[%s] for writing failed: %s", qPrintable(scnAbsFilePath), qPrintable(saveFi.errorString()));
    return false;
  }
  QDataStream iStream{&saveFi};
  iStream.setVersion(QDataStream::Qt_5_15);  // 设置版本保证兼容性
  // 写入文件头：魔数和版本
  iStream << SceneInfo::MAGIC_NUMBER;     // "LMSC" magic
  iStream << SceneInfo::CURRENT_VERSION;  //
  // 写入记录数量
  iStream << (SceneInfo::ELEMENT_COUNT_TYPE)scenes.size();
  for (const SceneInfo& scene : scenes) {
    iStream << scene;
  }

  if (iStream.status() != QDataStream::Status::Ok) {
    LOG_W("DataStream Status[%d] invalid", iStream.status());
    return false;
  }

  if (!saveFi.commit()) {
    LOG_W("Commit file[%s] failed: %s", qPrintable(scnAbsFilePath), qPrintable(saveFi.errorString()));
    return false;
  }
  return true;
}

bool UpdateNameWithNewRate(const QString& scnFilePath, const QString& specifiedName, int newRate) {
  QFile file{scnFilePath};
  if (!file.open(QIODevice::ReadWrite)) {
    LOG_D("cannot open file for ReadWrite. %s", qPrintable(file.errorString()));
    return false;
  }

  QDataStream stream(&file);
  stream.setVersion(QDataStream::Qt_5_15);

  // 读取文件头
  auto magic = SceneInfo::MAGIC_NUMBER;
  auto version = SceneInfo::CURRENT_VERSION;
  SceneInfo::ELEMENT_COUNT_TYPE recordCount;

  stream >> magic >> version >> recordCount;

  if (magic != SceneInfo::MAGIC_NUMBER) {
    file.close();
    LOG_W("magic[%d] not match", magic);
    return false;
  }

  if (version < SceneInfo::MIN_SUPPORTED_VERSION) {
    file.close();
    LOG_W("version[%d] too low", version);
    return false;
  }

  SceneInfo temp;
  for (int i = 0; i < recordCount; ++i) {
    const qint64 recordStart = file.pos();
    if (!temp.GetNameFromStream(stream)) {
      LOG_W("Read name of element:%d failed", i);
      break;
    }

    if (temp.name != specifiedName) {  // 不匹配，跳过整个记录
      file.seek(recordStart);          // 回到记录开始位置
      stream >> temp;                  // 读取并丢弃整个记录
      continue;
    }

    // 找到匹配记录，现在需要跳过imgs, vidName, vidSize字段到达rate字段, 写入新值
    bool reWrite = temp.DeviateStreamFromNameToRateAndOverrideRate(stream, newRate);
    file.close();
    if (!reWrite) {
      LOG_W("Write new rate[%d] to element:%d failed", newRate, i);
      return false;
    }
    return true;
  }

  file.close();
  LOG_W("Scene Name '%s' not found", qPrintable(specifiedName));
  return false;
}

}  // namespace SceneHelper
