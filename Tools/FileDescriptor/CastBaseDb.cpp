#include "CastBaseDb.h"
#include "JsonHelper.h"
#include "NameTool.h"
#include "PerformerJsonFileHelper.h"
#include "PublicTool.h"
#include "PublicVariable.h"
#include "PublicMacro.h"
#include "StringTool.h"
#include "TableFields.h"

#include <QSqlQuery>
#include <QSqlField>
#include <QSqlError>
#include <QDirIterator>

using namespace PERFORMER_DB_HEADER_KEY;

const QString CastBaseDb::CREATE_PERF_TABLE_TEMPLATE  //
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
           .arg(ENUM_2_STR(Name))
           .arg(ENUM_2_STR(Rate))
           .arg(PerformerJsonFileHelper::DEFAULT_RATE)
           .arg(ENUM_2_STR(AKA))
           .arg(ENUM_2_STR(Tags))
           .arg(ENUM_2_STR(Ori))
           .arg(PerformerJsonFileHelper::DEFAULT_ORIENTATION)
           .arg(ENUM_2_STR(Vids))
           .arg(ENUM_2_STR(Imgs))
           .arg(ENUM_2_STR(Detail))};

const QString INSERT_FULL_FIELDS_TEMPLATE  //
    {"REPLACE INTO `%1` "                  //
     + QString(R"(
(`%1`, `%2`, `%3`, `%4`, `%5`, `%6`, `%7`, `%8`)
VALUES(:1, :2, :3, :4, :5, :6, :7, :8);)")
           .arg(ENUM_2_STR(Name))
           .arg(ENUM_2_STR(Rate))
           .arg(ENUM_2_STR(AKA))
           .arg(ENUM_2_STR(Tags))
           .arg(ENUM_2_STR(Ori))
           .arg(ENUM_2_STR(Vids))
           .arg(ENUM_2_STR(Imgs))
           .arg(ENUM_2_STR(Detail))};

enum INSERT_FULL_FIELDS_TEMPLATE_FIELD {
  INSERT_FULL_FIELDS_TEMPLATE_FIELD_Name = 0,
  INSERT_FULL_FIELDS_TEMPLATE_FIELD_Rate,
  INSERT_FULL_FIELDS_TEMPLATE_FIELD_AKA,
  INSERT_FULL_FIELDS_TEMPLATE_FIELD_Tags,
  INSERT_FULL_FIELDS_TEMPLATE_FIELD_Orientation,
  INSERT_FULL_FIELDS_TEMPLATE_FIELD_Vids,
  INSERT_FULL_FIELDS_TEMPLATE_FIELD_Imgs,
  INSERT_FULL_FIELDS_TEMPLATE_FIELD_Detail,
};

/* When Name is the only primary key and no other fields.
 * INSERT INTO `TABLE` (`%1`, `%2`, `%3`) VALUES (:%1, :%2, :%3) ON CONFLICT(%1) DO UPDATE SET `%2`=:%2, `%3`=:%3;
 * is same as follows. Otherwise field not specified will be null or default(replace into: delete and insert) */
const QString INSERT_NAME_ORI_IMGS_TEMPLATE  //
    {"INSERT INTO `%1`" + QString{R"((`%1`, `%2`, `%3`) VALUES (?1, ?2, ?3) ON CONFLICT(`%1`) DO UPDATE SET `%2`=?4, `%3`=?5;)"}
                              .arg(ENUM_2_STR(Name), ENUM_2_STR(Ori), ENUM_2_STR(Imgs))};

enum INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD { //  DO UPDATE SET `%2`=:%3, `%3`=:%4; must!
  INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD_Name = 0,
  INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD_Orientation,
  INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD_Imgs,
  INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD_Orientation_VALUE,
  INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD_Imgs_VALUE,
  };

// 数值参数占位符即便要填入的值一样也不可重用, 例如:1, :2, :2, 有两个:2会出错, 需要改占位符:1, :2, :3
/* INSERT INTO `%1` (`%1`,`%2`) VALUES (:1, :2) ON CONFLICT(`%1`) DO UPDATE SET `%2`=:3; */
// 命名占位符:Name, :AKA则可重复
/* INSERT INTO `TABLE` (`%1`,`%2`) VALUES (:%1, :%2) ON CONFLICT(`%1`) DO UPDATE SET `%2`=:%2;).arg("Name", "AKA") */
const QString INSERT_PERF_AND_AKA_TEMPLATE  //
    {"INSERT INTO `%1` "                    //
     + QString{R"(
(`%1`,`%2`) VALUES
(:1, :2) ON CONFLICT(`%1`) DO UPDATE SET `%2`=:3;)"}
           .arg(ENUM_2_STR(Name), ENUM_2_STR(AKA))};

enum INSERT_PERF_AND_AKA_TEMPLATE_FIELD {
  INSERT_PERF_AND_AKA_TEMPLATE_FIELD_Name = 0,
  INSERT_PERF_AND_AKA_TEMPLATE_FIELD_AKA,
  INSERT_PERF_AND_AKA_TEMPLATE_FIELD_AKA_VALUE,
};

int CastBaseDb::ReadFromImageHost(const QString& imgsHostPath) {
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
  QDirIterator it{imgsHostPath, TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
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

  for (auto mpIt = name2Ori.cbegin(); mpIt != name2Ori.cend(); ++mpIt) {
    const QString& perf = mpIt.key();
    const QString& imgs = name2Imgs[perf].join(PERFS_VIDS_IMGS_SPLIT_CHAR);  // img seperated by \n
    qry.bindValue(INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD_Name, perf);
    qry.bindValue(INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD_Orientation, mpIt.value());
    qry.bindValue(INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD_Imgs, imgs);
    qry.bindValue(INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD_Orientation_VALUE, mpIt.value());
    qry.bindValue(INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD_Imgs_VALUE, imgs);

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

int CastBaseDb::LoadFromPsonFile(const QString& imgsHostPath) {
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
  QDirIterator it{imgsHostPath, {"*.pson"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  using namespace PERFORMER_DB_HEADER_KEY;
  while (it.hasNext()) {
    it.next();
    const QVariantHash& pson = JsonHelper::MovieJsonLoader(it.filePath());
    qry.bindValue(INSERT_FULL_FIELDS_TEMPLATE_FIELD_Name,        pson[ENUM_2_STR(Name)].toString());
    qry.bindValue(INSERT_FULL_FIELDS_TEMPLATE_FIELD_Rate,        pson[ENUM_2_STR(Rate)].toInt());
    qry.bindValue(INSERT_FULL_FIELDS_TEMPLATE_FIELD_AKA,         pson[ENUM_2_STR(AKA)].toString());
    qry.bindValue(INSERT_FULL_FIELDS_TEMPLATE_FIELD_Tags,        pson[ENUM_2_STR(Tags)].toString());
    qry.bindValue(INSERT_FULL_FIELDS_TEMPLATE_FIELD_Orientation, pson[ENUM_2_STR(Ori)].toString());
    qry.bindValue(INSERT_FULL_FIELDS_TEMPLATE_FIELD_Vids,        pson[ENUM_2_STR(Vids)].toString());
    qry.bindValue(INSERT_FULL_FIELDS_TEMPLATE_FIELD_Imgs,        pson[ENUM_2_STR(Imgs)].toString());
    qry.bindValue(INSERT_FULL_FIELDS_TEMPLATE_FIELD_Detail,      pson[ENUM_2_STR(Detail)].toString());

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

QMap<QString, QString> CastBaseDb::GetFreqName2AkaNames(const QStringList& perfsList) {
  QMap<QString, QString> perfs;
  static NameTool nt;
  for (const QString& line : perfsList) {
    if (line.isEmpty()) {
      continue;
    }
    QStringList aka = nt(line);
    if (aka.isEmpty()) {
      continue;
    }
    QString mostFreqUsedName = aka.front();
    aka.pop_front();
    perfs.insert(mostFreqUsedName, aka.join(","));
  }
  return perfs;
}

QMap<QString, QString> CastBaseDb::GetFreqName2AkaNames(const QString& perfsText) {
  const QStringList& perfsList{perfsText.split(StringTool::PERFS_VIDS_IMGS_SPLIT_CHAR, Qt::SkipEmptyParts)};
  return GetFreqName2AkaNames(perfsList);
}

int CastBaseDb::AppendCastFromMultiLineInput(const QString& perfsText) {
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
    qWarning("start the %dth transaction failed: %s", 1, qPrintable(db.lastError().text()));
    return FD_TRANSACTION_FAILED;
  }

  QSqlQuery qry{db};
  if (!qry.prepare(INSERT_PERF_AND_AKA_TEMPLATE.arg(DB_TABLE::PERFORMERS))) {
    qWarning("prepare command[%s] failed: %s",  //
             qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    return FD_PREPARE_FAILED;
  }
  int succeedCnt = 0;
  // update aka by new value if name conflict
  for (auto it = perfs.cbegin(); it != perfs.cend(); ++it) {
    qry.bindValue(INSERT_PERF_AND_AKA_TEMPLATE_FIELD_Name, it.key());
    qry.bindValue(INSERT_PERF_AND_AKA_TEMPLATE_FIELD_AKA, it.value());
    qry.bindValue(INSERT_PERF_AND_AKA_TEMPLATE_FIELD_AKA_VALUE, it.value());
    if (!qry.exec()) {
      qWarning("Insert[%s] failed: %s",  //
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

bool CastBaseDb::UpdateRecordImgsField(QSqlRecord& sqlRecord, const QString& imageHostPath) {
  QDir castDir{GetCastPath(sqlRecord, imageHostPath)};
  castDir.setFilter(QDir::Filter::Files);
  castDir.setSorting(QDir::SortFlag::Name);
  castDir.setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
  if (!castDir.exists()) {
    return false;
  }
  sqlRecord.setValue(INSERT_FULL_FIELDS_TEMPLATE_FIELD_Imgs, castDir.entryList().join(StringTool::PERFS_VIDS_IMGS_SPLIT_CHAR));
  return true;
}

QString CastBaseDb::GetCastPath(const QSqlRecord& sqlRecord, const QString& imageHostPath) {
  return imageHostPath + '/'//
         + sqlRecord.field(INSERT_FULL_FIELDS_TEMPLATE_FIELD_Orientation).value().toString() + '/'//
         + sqlRecord.field(INSERT_FULL_FIELDS_TEMPLATE_FIELD_Name).value().toString();//
}

QString CastBaseDb::GetCastFilePath(const QSqlRecord& sqlRecord, const QString& imageHostPath) {
  return GetCastPath(sqlRecord, imageHostPath) + '/'//
         + sqlRecord.field(INSERT_FULL_FIELDS_TEMPLATE_FIELD_Name).value().toString()//
         + ".pson";
}

bool CastBaseDb::IsNewOriFolderPathValid(const QString& destPath, const QString& imageHost, QString& newOri) {
  if (!QFileInfo{destPath}.isDir()) {
    qDebug("Abort Migrate, path[%s] not a directory", qPrintable(destPath));
    return false;
  }
  if (!destPath.startsWith(imageHost + '/')) {
    qWarning("Abort Migrate, Path[%s] not under imageHost[%s]", qPrintable(destPath), qPrintable(imageHost));
    return false;
  }
  QString newOriFolder = destPath.mid(imageHost.size() + 1);
  if (newOriFolder.isEmpty() || newOriFolder.contains('/')) {
    qWarning("Abort Migrate, Ori Folder Name[%s] invalid", qPrintable(newOriFolder));
    return false;
  }
  newOri.swap(newOriFolder);
  return true;
}

int CastBaseDb::MigrateToNewOriFolder(QSqlRecord &sqlRecord, QDir& imageHostDir, const QString &newOri) {
  const QString oldOri {sqlRecord.field(PERFORMER_DB_HEADER_KEY::Ori).value().toString()};
  if (newOri == oldOri) {
    return FD_ERROR_CODE::FD_SKIP;
  }
  // Migrate old folder from oldOri to newOri
  const QString castName {sqlRecord.field(PERFORMER_DB_HEADER_KEY::Name).value().toString()};
  if (imageHostDir.exists(oldOri + '/' + castName)) { // folder oldOri not exist
    if (!imageHostDir.rename(oldOri + '/' + castName, newOri + '/' + castName)) {
      qWarning("Migrate folder failed, castName[%s] from oldOri[%s] to newOri[%s]", qPrintable(castName), qPrintable(oldOri), qPrintable(newOri));
      return FD_ERROR_CODE::FD_RENAME_FAILED;
    }
  }
  sqlRecord.setValue(PERFORMER_DB_HEADER_KEY::Ori, newOri);
  return FD_ERROR_CODE::FD_OK;
}

