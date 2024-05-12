#ifndef CATEGORIZER_H
#define CATEGORIZER_H

#include <QDir>
#include <QMap>
#include <QString>
#include "FileOperation/FileOperation.h"
#include "PublicVariable.h"
#include "UndoRedo.h"

class Categorizer {
 public:
  Categorizer() = default;

  static auto ImaRName(const QString& p) -> QString {
    auto imgDotIndex = p.lastIndexOf('.');
    const QString& nm = p.left(imgDotIndex);  // name without ".suffix"
    const auto spaceInd = nm.lastIndexOf(' ');
    if (spaceInd == -1) {
      return nm;
    }

    const QString& num = nm.mid(spaceInd + 1);
    bool isNumber = false;
    const int numInt = num.toInt(&isNumber);

    if (not isNumber) {  // AA - 1.mp4, 1984 is a specified year here;
      return nm;         // delete sequence;
    }
    // 10 hour = 36000s
    // 0 second = 00000s
    if (1800 <= numInt and numInt <= 2300) {  // AA - 1984.mp4, 1984 is a specified year here;
      return nm;                              // keep years;
    }
    QString rnm = nm.left(spaceInd).trimmed();  // AA - 1.mp4
    if (rnm.back() != '-') {                    // AA -
      return rnm;
    }
    rnm.chop(1);
    return rnm.trimmed();
    ;
  }

  static auto VidRName(const QString& p) -> QString {
    auto vidDotIndex = p.lastIndexOf('.');
    if (vidDotIndex == -1) {
      return p;
    }
    return p.left(vidDotIndex);
  }
  // Attention: "xx n.image" -> "xx" not "xx n";
  //            "xx.image" -> "xx";
  static auto Classify(const QString& rootDir) -> QMap<QString, QStringList> {
    // Folder: Keep the Same;
    // Vid: eliminate format;
    // Images: eliminate format and "- sequence"(if exists);
    // Json: eliminate format;
    // d2p["xx 1"] = {xx 1, xx 1.mp4, xx 1 1.jpg, xx 1.json};
    // d2p["xx"] = {xx, xx.mp4, xx.jpg, xx 1.jpg, xx 2.png, xx.json};
    QMap<QString, QStringList> dst2Contents;
    const QList<QFileInfo>& nameList = QDir(rootDir).entryInfoList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot);

    for (const QFileInfo& fi : nameList) {
      const QString& nm = fi.fileName();
      QString dst;
      if (fi.isDir()) {  // folder;
        dst = fi.fileName();
      } else if (TYPE_FILTER::IMAGE_TYPE_SET.contains("*." + fi.suffix())) {  // ima;
        dst = ImaRName(nm);
      } else {  // vid or json;
        dst = VidRName(nm);
      }
      if (not dst2Contents.contains(dst)) {
        dst2Contents.insert(dst, {nm});
        continue;
      }
      dst2Contents[dst].append(nm);
    }
    return dst2Contents;
  }

  auto operator()(const QString& rootDir) const -> bool {
    if (QDir(rootDir).isRoot() or not QFile::exists(rootDir)) {
      qDebug("[Folder is root or not exists error] %s", qPrintable(rootDir));
      return false;
    }
    FileOperatorType::BATCH_COMMAND_LIST_TYPE cmds;
    const auto& dic = Classify(rootDir);
    QMapIterator<QString, QStringList> i(dic);
    while (i.hasNext()) {
      i.next();
      const QString& folderName = i.key();
      const QStringList& fileItems = i.value();

      if (fileItems.size() < 2)
        continue;  // [Ignored Only One Item] [{folderName}];
      const QString& underPath = rootDir + '/' + folderName;
      if (not QFile::exists(underPath)) {
        cmds.append({"mkpath", rootDir, folderName});
      }
      QDir underDir(underPath);
      for (const QString& fileName : fileItems) {
        if (QFileInfo(rootDir, fileName).isDir()) {
          continue;  // skip move dir
        }
        if (underDir.exists(fileName)) {
          qDebug("%s Already Exist in %s", qPrintable(fileName), qPrintable(underPath));
          continue;
        }
        cmds.append({"rename", rootDir, fileName, underPath, fileName});
      }
    }
    const auto isAllSuccess = g_undoRedo.Do(cmds);
    return isAllSuccess;
  }
};

#endif  // CATEGORIZER_H
