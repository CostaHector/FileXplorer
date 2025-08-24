#include "PerfBaseDb.h"
#include "JsonHelper.h"
#include "NameTool.h"
#include "PerformerJsonFileHelper.h"
#include "PublicTool.h"
#include "PublicVariable.h"
#include "StringTool.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDirIterator>

const QString PerfBaseDb::CREATE_PERF_TABLE_TEMPLATE  //
    {"CREATE TABLE IF NOT EXISTS `%1`"                //
     + QString(R"((
`%1` TEXT NOT NULL,
`%2` INT DEFAULT %3,
`%4` TEXT DEFAULT "",
`%5` TEXT DEFAULT "",
`%6` TEXT DEFAULT "%7",
`%8` TEXT DEFAULT "",
`%9` TEXT DEFAULT "",
`%10` TEXT DEFAULT "",
 PRIMARY KEY (%1)
);)")
           .arg(PERFORMER_DB_HEADER_KEY::Name)
           .arg(PERFORMER_DB_HEADER_KEY::Rate)
           .arg(PERFORMER_DB_HEADER_KEY::DEFAULT_RATE)
           .arg(PERFORMER_DB_HEADER_KEY::AKA)
           .arg(PERFORMER_DB_HEADER_KEY::Tags)
           .arg(PERFORMER_DB_HEADER_KEY::Orientation)
           .arg(PERFORMER_DB_HEADER_KEY::DEFAULT_ORIENTATION)
           .arg(PERFORMER_DB_HEADER_KEY::Vids)
           .arg(PERFORMER_DB_HEADER_KEY::Imgs)
           .arg(PERFORMER_DB_HEADER_KEY::Detail)};

const QString INSERT_FULL_FIELDS_TEMPLATE  //
    {"REPLACE INTO `%1` "                  //
     + QString(R"(
(`%1`, `%2`, `%3`, `%4`, `%5`, `%6`, `%7`, `%8`)
VALUES(:%1, :%2, :%3, :%4, :%5, :%6, :%7, :%8);)")
           .arg(PERFORMER_DB_HEADER_KEY::Name)
           .arg(PERFORMER_DB_HEADER_KEY::Rate)
           .arg(PERFORMER_DB_HEADER_KEY::AKA)
           .arg(PERFORMER_DB_HEADER_KEY::Tags)
           .arg(PERFORMER_DB_HEADER_KEY::Orientation)
           .arg(PERFORMER_DB_HEADER_KEY::Vids)
           .arg(PERFORMER_DB_HEADER_KEY::Imgs)
           .arg(PERFORMER_DB_HEADER_KEY::Detail)};

const QString INSERT_NAME_ORI_IMGS_TEMPLATE  //
    {"INSERT INTO %1"                        //
     + QString{R"(
(`%1`, `%2`, `%3`)
VALUES (:%1, :%2, :%3)
ON CONFLICT(%1)
DO UPDATE SET `%2`=:%2, `%3`=:%3;)"}
           .arg(PERFORMER_DB_HEADER_KEY::Name, PERFORMER_DB_HEADER_KEY::Orientation, PERFORMER_DB_HEADER_KEY::Imgs)};

const QString INSERT_PERF_AND_AKA_TEMPLATE  //
    {"INSERT INTO `%1` "                    //
     + QString{R"(
(`%1`,`%2`) VALUES
(:%1, :%2) ON CONFLICT(`%1`) DO UPDATE SET `%2`=:%2;)"}
           .arg(PERFORMER_DB_HEADER_KEY::Name, PERFORMER_DB_HEADER_KEY::AKA)};

const QString INSERT_ONLY_PERFS_NAME_TEMPLAE  //
    {"INSERT INTO `%1` "                      //
     + QString(R"(
(`%2`) VALUES(:%2);)")
           .arg(PERFORMER_DB_HEADER_KEY::Name)};

int PerfBaseDb::InsertPerformers(const QStringList& perfList) {
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    qWarning("Open failed");
    return FD_DB_OPEN_FAILED;
  }
  QSqlQuery qry{db};
  if (!qry.prepare(INSERT_ONLY_PERFS_NAME_TEMPLAE.arg(DB_TABLE::PERFORMERS))) {
    qWarning("prepare cmd[%s] failed: %s",  //
             qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    return FD_PREPARE_FAILED;
  }
  int succeedCnt = 0;
  static const QString pHKey{":" + PERFORMER_DB_HEADER_KEY::Name};
  foreach (const QString& perf, perfList) {
    if (perf.isEmpty()) {
      continue;
    }
    qry.bindValue(pHKey, perf);
    if (!qry.exec()) {
      db.rollback();
      qWarning("exec cmd[%s] failed: %s",  //
               qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
      return FD_EXEC_FAILED;
    }
    ++succeedCnt;
  }
  qry.finish();
  qDebug("%d/%d performer(s) add succeed by specified string:%s",  //
         succeedCnt, perfList.size(), qPrintable(perfList.join('|')));
  return succeedCnt;
}

int PerfBaseDb::ReadFromImageHost(const QString& imgsHostPath) {
  if (!QFileInfo(imgsHostPath).isDir()) {
    qWarning("Directory[%s] not exist", qPrintable(imgsHostPath));
    return FD_NOT_DIR;
  }

  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }
  if (!db.transaction()) {
    qWarning("start the %dth transaction failed: %s",  //
             1, qPrintable(db.lastError().text()));
    return FD_TRANSACTION_FAILED;
  }

  QSqlQuery qry{db};
  if (!qry.prepare(INSERT_NAME_ORI_IMGS_TEMPLATE.arg(DB_TABLE::PERFORMERS))) {
    qWarning("prepare command[%s] failed: %s",  //
             qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    return FD_PREPARE_FAILED;
  }

  int succeedCnt = 0;
  QMap<QString, QString> name2Ori;
  QMap<QString, QStringList> name2Imgs;
  QDirIterator it(imgsHostPath, TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QStringList& imgPathParts = it.filePath().split('/');
    const int pathSectionSize = imgPathParts.size();
    if (pathSectionSize < 3) {
      qWarning("Path[%s] section count[%d] invalid", qPrintable(it.filePath()), pathSectionSize);
      continue;
    }
    const QString& imgName = imgPathParts[pathSectionSize - 1];
    const QString& perfName = imgPathParts[pathSectionSize - 2];
    const QString& ori = imgPathParts[pathSectionSize - 3];
    auto it = name2Imgs.find(perfName);
    if (it != name2Imgs.end()) {
      it.value().append(imgName);
    } else {
      name2Imgs.insert(perfName, {imgName});
      name2Ori[perfName] = ori;
    }
  }
  using namespace StringTool;
  for (auto it = name2Imgs.begin(); it != name2Imgs.end(); ++it) {
    ImgsSortNameLengthFirst(it.value());
  }

  static const QString PERF_PH{':' + PERFORMER_DB_HEADER_KEY::Name};
  static const QString ORI_PH{':' + PERFORMER_DB_HEADER_KEY::Orientation};
  static const QString IMGS_PH{':' + PERFORMER_DB_HEADER_KEY::Imgs};
  for (auto mpIt = name2Ori.cbegin(); mpIt != name2Ori.cend(); ++mpIt) {
    const QString& perf = mpIt.key();
    const QString& imgs = name2Imgs[perf].join(PERFS_VIDS_IMGS_SPLIT_CHAR);  // img seperated by \n
    qry.bindValue(PERF_PH, perf);
    qry.bindValue(ORI_PH, mpIt.value());
    qry.bindValue(IMGS_PH, imgs);
    if (!qry.exec()) {
      qWarning("replace[%s] failed: %s",  //
               qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
      db.rollback();
      return FD_EXEC_FAILED;
    }
    ++succeedCnt;
  }

  if (!db.commit()) {
    qWarning("commit failed: %s", qPrintable(db.lastError().text()));
    db.rollback();
    return FD_COMMIT_FAILED;
  }
  qry.finish();
  qDebug("Read %d perfs out from path[%s] succeed", succeedCnt, qPrintable(imgsHostPath));
  return succeedCnt;
}

int PerfBaseDb::LoadFromPJsonFile(const QString& imgsHostPath) {
  if (!QFileInfo(imgsHostPath).isDir()) {
    qWarning("Directory[%s] not exist", qPrintable(imgsHostPath));
    return FD_NOT_DIR;
  }

  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }
  if (!db.transaction()) {
    qWarning("start the %dth transaction failed: %s",  //
             1, qPrintable(db.lastError().text()));
    return FD_TRANSACTION_FAILED;
  }

  QSqlQuery qry{db};
  if (!qry.prepare(INSERT_FULL_FIELDS_TEMPLATE.arg(DB_TABLE::PERFORMERS))) {
    qWarning("prepare command[%s] failed: %s",  //
             qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    return FD_PREPARE_FAILED;
  }

  int succeedCnt = 0;
  QDirIterator it{imgsHostPath, {"*.pjson"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  while (it.hasNext()) {
    it.next();
    const QVariantHash& pJson = JsonHelper::MovieJsonLoader(it.filePath());
    qry.bindValue(':' + PERFORMER_DB_HEADER_KEY::Name, pJson[PERFORMER_DB_HEADER_KEY::Name].toString());
    qry.bindValue(':' + PERFORMER_DB_HEADER_KEY::Rate, pJson[PERFORMER_DB_HEADER_KEY::Rate].toInt());
    qry.bindValue(':' + PERFORMER_DB_HEADER_KEY::AKA, pJson[PERFORMER_DB_HEADER_KEY::AKA].toString());
    qry.bindValue(':' + PERFORMER_DB_HEADER_KEY::Tags, pJson[PERFORMER_DB_HEADER_KEY::Tags].toString());
    qry.bindValue(':' + PERFORMER_DB_HEADER_KEY::Orientation, pJson[PERFORMER_DB_HEADER_KEY::Orientation].toString());
    qry.bindValue(':' + PERFORMER_DB_HEADER_KEY::Vids, pJson[PERFORMER_DB_HEADER_KEY::Vids].toString());
    qry.bindValue(':' + PERFORMER_DB_HEADER_KEY::Imgs, pJson[PERFORMER_DB_HEADER_KEY::Imgs].toString());
    qry.bindValue(':' + PERFORMER_DB_HEADER_KEY::Detail, pJson[PERFORMER_DB_HEADER_KEY::Detail].toString());

    if (!qry.exec()) {
      qWarning("replace[%s] failed: %s",  //
               qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
      db.rollback();
      return FD_EXEC_FAILED;
    }
    ++succeedCnt;
  }

  if (!db.commit()) {
    qWarning("commit failed: %s", qPrintable(db.lastError().text()));
    db.rollback();
    return FD_COMMIT_FAILED;
  }
  qry.finish();
  qDebug("Read %d perfs out from path[%s] succeed", succeedCnt, qPrintable(imgsHostPath));
  return succeedCnt;
}

QMap<QString, QString> PerfBaseDb::GetFreqName2AkaNames(const QString& perfsText) {
  QMap<QString, QString> perfs;
  for (const QString& line : perfsText.split(StringTool::PERFS_VIDS_IMGS_SPLIT_CHAR)) {
    if (line.isEmpty()) {
      continue;
    }
    QStringList aka = NameTool()(line);
    if (aka.isEmpty()) {
      continue;
    }
    QString mostFreqUsedName = aka.front();
    aka.pop_front();
    perfs.insert(mostFreqUsedName, aka.join(","));
  }
  return perfs;
}

int PerfBaseDb::ReadFromUserInputSentence(const QString& perfsText) {
  const auto& perfs = GetFreqName2AkaNames(perfsText);
  if (perfs.isEmpty()) {
    qDebug("Read no perfs out from text");
    return FD_SKIP;
  }
  qDebug("Read %d perf(s) out from text", perfs.size());

  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }
  if (!db.transaction()) {
    qWarning("start the %dth transaction failed: %s",  //
             1, qPrintable(db.lastError().text()));
    return FD_TRANSACTION_FAILED;
  }

  QSqlQuery qry(db);
  if (!qry.prepare(INSERT_PERF_AND_AKA_TEMPLATE.arg(DB_TABLE::PERFORMERS))) {
    qWarning("prepare command[%s] failed: %s",  //
             qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    return FD_PREPARE_FAILED;
  }
  int succeedCnt = 0;
  static const QString NAME_PH{':' + PERFORMER_DB_HEADER_KEY::Name};
  static const QString AKA_PH{':' + PERFORMER_DB_HEADER_KEY::AKA};
  // update aka by new value if name conflict
  for (auto it = perfs.cbegin(); it != perfs.cend(); ++it) {
    qry.bindValue(NAME_PH, it.key());
    qry.bindValue(AKA_PH, it.value());
    if (!qry.exec()) {
      qWarning("replace[%s] failed: %s",  //
               qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
      db.rollback();
      return FD_EXEC_FAILED;
    }
    ++succeedCnt;
  }
  if (!db.commit()) {
    qWarning("commit failed: %s", qPrintable(db.lastError().text()));
    db.rollback();
    return FD_COMMIT_FAILED;
  }
  qry.finish();
  qDebug("Read %d perfs out from text user input[%s] succeed", succeedCnt, qPrintable(perfsText));
  return succeedCnt;
}
