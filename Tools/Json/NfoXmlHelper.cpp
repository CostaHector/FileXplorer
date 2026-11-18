#include "NfoXmlHelper.h"
#include "JsonRenameRegex.h"
#include "FileTool.h"
#include "JsonHelper.h"
#include "NameTool.h"
#include "Logger.h"
#include <QDomElement>
#include <QVariantHash>

namespace NfoXmlHelper {
QString ProcessDetailMap(const QStringList& castLst, const QString& castDetail, QMap<QString, QString>& detailMap);
std::pair<bool, QVariantHash> MovieNfoXml2Json(const QString& nfoFileContentsRaw);
std::pair<QString, QString> ParseActorNode(const QDomElement& actorNode);
void CollectDetail(const QDomElement &elem, QMap<QString, QString> &map);
}

namespace NfoXmlHelper {
int GenerateJsonsFromNfoXMLs(const QStringList& nfoXmlFileFullPaths) {
  int succeedCnt = 0;
  for (const QString& pth: nfoXmlFileFullPaths) {
    bool isOk = GenerateJsonFromNfoXML(pth);
    succeedCnt += isOk;
  }
  return succeedCnt;
}

bool GenerateJsonFromNfoXML(const QString& nfoXmlFileFullPath) {
  const int startExtIndex = nfoXmlFileFullPath.lastIndexOf(".nfo", -1, Qt::CaseSensitivity::CaseInsensitive);
  if (startExtIndex + sizeof(".nfo") - 1 != nfoXmlFileFullPath.size()) {
    return false;
  }
  bool bReadOk{false};
  const QString nfoContentsRaw = FileTool::StringTextReader(nfoXmlFileFullPath, &bReadOk);
  if (!bReadOk) {
    return false;
  }
  const std::pair<bool, QVariantHash> parseResultPair = MovieNfoXml2Json(nfoContentsRaw);
  if (!parseResultPair.first) {
    return false;
  }
  if (parseResultPair.second.isEmpty()) {
    return false;
  }

  const QString jsonFullPath{nfoXmlFileFullPath.left(startExtIndex) + ".json"};
  return JsonHelper::DumpJsonDict(parseResultPair.second, jsonFullPath);
}

std::pair<bool, QVariantHash> MovieNfoXml2Json(const QString& nfoFileContentsRaw) {
  QDomDocument doc;
  QString errorMsg;
  int errorLine = 0, errorColumn = 0;

  // 直接解析字符串，不再使用 QFile
  if (!doc.setContent(nfoFileContentsRaw, &errorMsg, &errorLine, &errorColumn)) {
    LOG_W("Parse failed. message[%s] failed at position line %d:%d", qPrintable(errorMsg), errorLine, errorColumn);
    return {false, {}};
  }

  QDomElement root = doc.documentElement();
  if (root.tagName() != "movie") {
    LOG_W("Root node[%s] is not movie but", qPrintable(root.tagName()));
  }

  QString name;
  QString studio;
  QString resolution;
  QStringList tags;
  QStringList castLst; QString castDetail;
  QMap<QString, QString> detailMap;

  QDomElement child = root.firstChildElement();
  while (!child.isNull()) {
    const QString tag = child.tagName();

    if (tag == "title") {
      name = child.text().trimmed();
    } else if (tag == "genre") {
      tags << child.text().trimmed();
    } else if (tag == "studio") {
      studio = child.text().trimmed();
    } else if (tag == "actor") {
      QString actorName, actorDetail;
      std::tie(actorName, actorDetail) = ParseActorNode(child);
      castLst << actorName;
      if (!castDetail.isEmpty()) {
        castDetail += "\n";
      }
      castDetail += actorDetail;
    } else if (tag == "fileinfo") {
      QDomElement streamdetails = child.firstChildElement("streamdetails");
      if (!streamdetails.isNull()) {
        QDomElement video = streamdetails.firstChildElement("video");
        if (!video.isNull()) {
          const QString width  = video.firstChildElement("width").text().trimmed();
          const QString height = video.firstChildElement("height").text().trimmed();
          if (!width.isEmpty() && !height.isEmpty()) {
            resolution = width + "x" + height;
          }

          QDomElement vc = video.firstChildElement();
          while (!vc.isNull()) {
            if (vc.tagName() != "width" && vc.tagName() != "height") {
              CollectDetail(vc, detailMap);
            }
            vc = vc.nextSiblingElement();
          }
        }

        QDomElement audio = streamdetails.firstChildElement("audio");
        if (!audio.isNull()) {
          CollectDetail(audio, detailMap);
        }

        QDomElement other = streamdetails.firstChildElement();
        while (!other.isNull()) {
          if (other.tagName() != "video" && other.tagName() != "audio") {
            CollectDetail(other, detailMap);
          }
          other = other.nextSiblingElement();
        }
      }

      QDomElement other = child.firstChildElement();
      while (!other.isNull()) {
        if (other.tagName() != "streamdetails") {
          CollectDetail(other, detailMap);
        }
        other = other.nextSiblingElement();
      }
    } else {
      CollectDetail(child, detailMap);
    }

    child = child.nextSiblingElement();
  }

  QVariantHash obj;
  obj["Name"]       = name;
  obj["Resolution"] = resolution;
  obj["Studio"]     = studio;
  obj["Tags"]       = tags;
  obj["Cast"]       = castLst;
  obj["Detail"]     = ProcessDetailMap(castLst, castDetail, detailMap);
  return {true, obj};
}

void CollectDetail(const QDomElement &elem, QMap<QString, QString> &map) {
  QDomElement child = elem.firstChildElement();
  if (child.isNull()) {
    const QString tag  = elem.tagName();
    const QString text = elem.text().trimmed();

    if (map.contains(tag)) {
      if (!text.isEmpty()) {
        if (map[tag].isEmpty())
          map[tag] = text;
        else
          map[tag] += ", " + text;
      }
    } else {
      map[tag] = text;
    }
    return;
  }

  while (!child.isNull()) {
    CollectDetail(child, map);
    child = child.nextSiblingElement();
  }
}

// <"ActorName", "ActorName, MainCharactor, portrait.jpg, profile">
std::pair<QString, QString> ParseActorNode(const QDomElement& actorNode) {
  QString actorName;
  QDomElement nameElem = actorNode.firstChildElement("name");
  if (!nameElem.isNull()) {
    actorName = nameElem.text().trimmed();
  }

  // 把当前 actor 下所有子节点按 DOM 顺序用逗号拼接（空节点也占位）
  QStringList parts;
  QDomElement ac = actorNode.firstChildElement();
  while (!ac.isNull()) {
    parts << ac.text().trimmed();
    ac = ac.nextSiblingElement();
  }

  return {actorName, parts.join(NameTool::COMMA_SPACE)};
}

QString ProcessDetailMap(const QStringList& castLst, const QString& castDetail, QMap<QString, QString>& detailMap) {
  // 按照顺序 cast, role, plot/outline, country, durationinseconds, premiered（首映时间） 拼接
  const QStringList keyInOrder {"country", "durationinseconds", "premiered"};
  QString detailMsg;
  if (!castLst.isEmpty()) {
    /*
     * Casts: name1, name 2
     * Roles:
     * ActorName, MainCharactor, portrait.jpg, profile
     * ActorName, MainCharactor, portrait.jpg, profile
     *
     * plot or outline here
     *
     * other key: value pair
     */
    detailMsg += "Casts";
    detailMsg += NameTool::COLON_SPACE;

    detailMsg += castLst.join(NameTool::COMMA_SPACE);
    detailMsg += "\nRoles:\n";
    detailMsg += castDetail;
    detailMsg += "\n\n";
  }

  {
    QString plotRawStr, plotAlphaDigitStr;

    // "plot", "outline"
    auto plotIt = detailMap.find("plot");
    if (plotIt != detailMap.cend()) {
      plotAlphaDigitStr = plotRawStr = plotIt.value();
      plotAlphaDigitStr.replace(JSON_RENAME_REGEX::INVALID_GOOGLE_SEARCH_LETTER, "");
      detailMap.erase(plotIt);
    }

    auto outlineIt = detailMap.find("outline");
    QString outlineRawStr, outlineAlphaDigitStr;
    if (outlineIt != detailMap.cend()) {
      outlineAlphaDigitStr = outlineRawStr = outlineIt.value();
      outlineAlphaDigitStr.replace(JSON_RENAME_REGEX::INVALID_GOOGLE_SEARCH_LETTER, "");
      detailMap.erase(outlineIt);
    }
    // 只比较字母和数字是否相等
    const QString plotOrOutlineMsg{ plotAlphaDigitStr == outlineAlphaDigitStr ? plotRawStr : plotRawStr + '\n' + outlineRawStr };
    detailMsg += plotOrOutlineMsg;
    detailMsg += "\n\n";
  }

  for (const QString& key: keyInOrder) {
    auto keyIt = detailMap.find(key);
    if (keyIt == detailMap.cend()) {
      continue;
    }
    detailMsg += key + NameTool::COLON_SPACE + keyIt.value();
    detailMsg += "\n";
    detailMap.erase(keyIt);
  }

  for (auto it = detailMap.constBegin(); it != detailMap.constEnd(); ++it) {
    detailMsg += it.key() + NameTool::COLON_SPACE + it.value();
    detailMsg += "\n";
  }
  return detailMsg;
}

}