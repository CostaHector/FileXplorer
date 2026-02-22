#include "HarFiles.h"
#include "PublicVariable.h"
#include "FileTool.h"
#include "Logger.h"
#include <QFile>
#include <QDir>
#include <QIODevice>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>

const QMap<QString, QString> HarFiles::SUPPORTED_MIMETYPES = {{"image/webp", ".webp"}, {"image/jpeg", ".jpeg"},  // *.jpg files have two possible extensions
                                                              {"image/jpeg", ".jpg"},                            //   (but .jpeg is official and thus preferred)
                                                              {"image/png", ".png"},   {"image/svg+xml", ".svg"}, {"image/avif", ".avif"},
                                                              {"image/bmp", ".bmp"},   {"image/gif", ".gif"},     {"image/vnd.microsoft.icon", ".ico"},
                                                              {"image/tiff", ".tif"},   // *.tiff files have two possible extensions
                                                              {"image/tiff", ".tiff"},  //   (but .tiff is what I know and prefer)
                                                              {"video/mp2t", ".ts"}, //
                                                              {"video/mp4", ".mp4"}, //
                                                              };

HarFiles::HarFiles() {}

QString GetPathStem(const QString& url) {
  const int slashIndex = url.lastIndexOf('/');

  const QString noRootPath = url.mid(slashIndex + 1);  // if slashIndex = -1, also ok
  const int dotIndex = noRootPath.lastIndexOf('.');
  // "rootpath/aaaaaaa.jpeg"
  if (dotIndex + 5 < noRootPath.size()) {
    return noRootPath;
  }
  return noRootPath.left(dotIndex);
}

void HarFiles::init() {
  mHarFilePath.clear();
  mHarItems.clear();
}

void HarFiles::swap(HarFiles& rhs)
{
  mHarFilePath.swap(rhs.mHarFilePath);
  mHarItems.swap(rhs.mHarItems);
}

const HAR_FILE_ITEM& HarFiles::operator[](int i) const {
  if (i < 0 || i >= size()) {
    static HAR_FILE_ITEM INVALID_HAR_FILE_ITEM;
    return INVALID_HAR_FILE_ITEM;
  }
  return mHarItems[i];
}

bool HarFiles::IsHarFile(const QFileInfo& fi) {
  return TYPE_FILTER::HAR_TYPE_SET.contains("*." + fi.suffix());
}

bool HarFiles::operator()(const QString& harAbsPath) {
  LOG_D("parse har file[%s] start...", qPrintable(harAbsPath));
  init();
  mHarFilePath = harAbsPath;
  const QString& jsonStr = FileTool::TextReader(harAbsPath);
  if (jsonStr.isEmpty()) {
    return false;
  }

  QJsonParseError jsonErr;
  QJsonDocument json_doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &jsonErr);
  if (jsonErr.error != QJsonParseError::NoError) {
    LOG_W("Error parse json string %d char(s): %s", jsonStr.size(), qPrintable(jsonErr.errorString()));
    return {};
  }
  const QJsonObject& rootObj = json_doc.object();
  const QVariantHash& harJson = rootObj.toVariantHash();
  if (!harJson.contains("log")) {
    LOG_W("key[%s] not exist in json dict harJson", "log");
    return false;
  }
  const QVariantHash& logValueVarHash = harJson["log"].toHash();
  // harJson["log"]["entries"]
  if (!logValueVarHash.contains("entries")) {
    LOG_W("key[%s] not exist in json dict logValueVarHash", "entries");
    return false;
  }
  const QList<QVariant>& entries = logValueVarHash["entries"].toList();
  LOG_D("entries.size()=%d", entries.size());
  for (const QVariant& entry : entries) {
    const auto& entryHash = entry.toHash();
    if (!entryHash.contains("response")) {
      LOG_W("key[%s] not exist in entryHash", "response");
      return false;
    }
    const auto& rspHash = entryHash["response"].toHash();
    if (!rspHash.contains("content")) {
      LOG_W("key[response/%s] not exist in rspHash", "content");
      return false;
    }
    const auto& contentHash = rspHash["content"].toHash();
    if (!contentHash.contains("mimeType")) {
      LOG_W("key[response/content/%s] not exist in contentHash", "mimeType");
      return false;
    }
    // entry["response"]["content"]["mimeType"]
    const QString mimetype = contentHash["mimeType"].toString();
    if (!SUPPORTED_MIMETYPES.contains(mimetype)) {
      LOG_D("mimetype:%s not in supported list", qPrintable(mimetype));
      continue;
    }
    if (!entryHash.contains("request")) {
      LOG_W("key[%s] not exist in entryHash", "request");
      return false;
    }
    const auto& reqHash = entryHash["request"].toHash();
    if (!reqHash.contains("url")) {
      LOG_W("key[request/%s] not exist in reqHash", "url");
      return false;
    }
    const QString& url = reqHash["url"].toString();
    const QString& fileBasename = GetPathStem(url);
    const QString& extension = SUPPORTED_MIMETYPES.contains(mimetype) ? SUPPORTED_MIMETYPES[mimetype] : "";
    const QString dstFileName = fileBasename + extension;
    LOG_D("fileStem[%25s%-6s], url: %64s", qPrintable(fileBasename), qPrintable(extension), qPrintable(url));
    if (!contentHash.contains("text")) {
      LOG_W("key[response/content/%s] not exist in reqHash", "text");
      continue;
    }
    const QString responseText = contentHash["text"].toString();
    QString encodingType = "literal";
    if (contentHash.contains("encoding")) {
      encodingType = contentHash["encoding"].toString();
    }
    QByteArray fileContent;
    if (encodingType == "literal") {
      fileContent = responseText.toUtf8();
    } else {  // base64
      fileContent = QByteArray::fromBase64(responseText.toUtf8(), QByteArray::Base64Encoding);
    }
    mHarItems.append(HAR_FILE_ITEM{dstFileName, fileContent, extension, url});
    // entry["response"]["content"].get("text")
    // entry["response"]["content"].get("encoding", "literal")
  }
  LOG_D("har file parsed ok and %d file were found.", mHarItems.size());
  return true;
}

int HarFiles::SaveToLocal(QString dstRootpath, const QList<int>& selectedRows) {
  if (mHarItems.isEmpty() || selectedRows.isEmpty()) {
    LOG_D("mHarItems is empty or nothing selected no need to save");
    return 0;
  }
  if (dstRootpath.isEmpty()) {
    dstRootpath = QFileInfo(mHarFilePath).absolutePath();
  }
  if (!QFileInfo(dstRootpath).isDir()) {
    LOG_W("destination path[%s] not exists", qPrintable(dstRootpath));
    return -1;
  }
  int fileWriteCnt = 0;
  const QDir dstDir(dstRootpath);
  for (const int& rowIndex : selectedRows) {
    const HAR_FILE_ITEM& item = mHarItems[rowIndex];
    const QString& dstAbsFilePath = dstDir.absoluteFilePath(item.name);
    QFile dstFi{dstAbsFilePath};
    if (!dstFi.open(QIODevice::WriteOnly)) {
      LOG_W("open file[%s] for write failed", qPrintable(dstAbsFilePath));
      continue;
    }
    dstFi.write(item.content);
    dstFi.close();
    ++fileWriteCnt;
    LOG_D("%3d. file[%s] write succeed", fileWriteCnt + 1, qPrintable(dstAbsFilePath));
  }
  LOG_W("%d file(s) write into path[%s] succeed", fileWriteCnt, qPrintable(dstRootpath));
  return fileWriteCnt;
}
