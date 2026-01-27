#include "CastBaseDb.h"
#include "JsonHelper.h"
#include "NameTool.h"
#include "CastPsonFileHelper.h"
#include "PublicTool.h"
#include "PublicVariable.h"
#include "PublicMacro.h"
#include "StringTool.h"
#include "TableFields.h"
#include "JsonRenameRegex.h"

#include <QSqlQuery>
#include <QSqlField>
#include <QSqlError>
#include <QDirIterator>

using namespace PERFORMER_DB_HEADER_KEY;

const QString CastBaseDb::CREATE_PERF_TABLE_TEMPLATE  //
    {"CREATE TABLE IF NOT EXISTS `%1` ("                //
#define PSON_KEY_ITEM(enu, enumVal, defaultValue, sqlRecordToValueFunc, tblFieldDefinition) + tblFieldDefinition.arg(ENUM_2_STR(enu)).arg(defaultValue)
      PSON_MODEL_FIELD_MAPPING
#undef PSON_KEY_ITEM
     + QString(R"( PRIMARY KEY (%1));)").arg(ENUM_2_STR(Name))
    };

const QString INSERT_FULL_FIELDS_TEMPLATE  //
    {"REPLACE INTO `%1` "                  //
     + QString(R"(
(`%1`, `%2`, `%3`, `%4`, `%5`, `%6`, `%7`, `%8`, `%9`, `%10`, `%11`)
VALUES(:1, :2, :3, :4, :5, :6, :7, :8, :9, :10, :11);)")
           .arg(ENUM_2_STR(Name))
           .arg(ENUM_2_STR(Rate))
           .arg(ENUM_2_STR(ALIAS))
           .arg(ENUM_2_STR(Tags))
           .arg(ENUM_2_STR(Ori))
           .arg(ENUM_2_STR(Height))
           .arg(ENUM_2_STR(Size))
           .arg(ENUM_2_STR(Birth))
           .arg(ENUM_2_STR(Vids))
           .arg(ENUM_2_STR(Imgs))
           .arg(ENUM_2_STR(Detail))
    };

enum INSERT_FULL_FIELDS_TEMPLATE_FIELD {
  INSERT_FULL_FIELDS_TEMPLATE_FIELD_Name = 0,
  INSERT_FULL_FIELDS_TEMPLATE_FIELD_Rate,
  INSERT_FULL_FIELDS_TEMPLATE_FIELD_ALIAS,
  INSERT_FULL_FIELDS_TEMPLATE_FIELD_Tags,
  INSERT_FULL_FIELDS_TEMPLATE_FIELD_Ori,
  INSERT_FULL_FIELDS_TEMPLATE_FIELD_Height,
  INSERT_FULL_FIELDS_TEMPLATE_FIELD_Size,
  INSERT_FULL_FIELDS_TEMPLATE_FIELD_Birth,
  INSERT_FULL_FIELDS_TEMPLATE_FIELD_Vids,
  INSERT_FULL_FIELDS_TEMPLATE_FIELD_Imgs,
  INSERT_FULL_FIELDS_TEMPLATE_FIELD_Detail,
};

/* When Name is the only primary key and no other fields.
 * INSERT INTO `TABLE` (`%1`, `%2`, `%3`) VALUES (:%1, :%2, :%3) ON CONFLICT(%1) DO UPDATE SET `%2`=:%2, `%3`=:%3;
 * is same as follows. Otherwise field not specified will be null or default(replace into: delete and insert) */
const QString INSERT_NAME_ORI_IMGS_TEMPLATE  //
    {"INSERT INTO `%1`" +
     QString{R"((`%1`, `%2`, `%3`) VALUES (?1, ?2, ?3) ON CONFLICT(`%1`) DO UPDATE SET `%2`=?4, `%3`=?5;)"}.arg(ENUM_2_STR(Name),
                                                                                                                ENUM_2_STR(Ori),
                                                                                                                ENUM_2_STR(Imgs))};

enum INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD {  //  DO UPDATE SET `%2`=:%3, `%3`=:%4; must!
  INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD_Name = 0,
  INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD_Ori,
  INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD_Imgs,
  INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD_Ori_VALUE,
  INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD_Imgs_VALUE,
};

// 数值参数占位符即便要填入的值一样也不可重用, 例如:1, :2, :2, 有两个:2会出错, 需要改占位符:1, :2, :3
/* INSERT INTO `%1` (`%1`,`%2`) VALUES (:1, :2) ON CONFLICT(`%1`) DO UPDATE SET `%2`=:3; */
// 命名占位符:Name, :ALIAS 则可重复
/* INSERT INTO `TABLE` (`%1`,`%2`) VALUES (:%1, :%2) ON CONFLICT(`%1`) DO UPDATE SET `%2`=:%2;).arg("Name", "ALIAS") */
const QString INSERT_PERF_AND_ALIAS_TEMPLATE  //
    {"INSERT INTO `%1` "                    //
     + QString{R"(
(`%1`,`%2`) VALUES
(:1, :2) ON CONFLICT(`%1`) DO UPDATE SET `%2`=:3;)"}
           .arg(ENUM_2_STR(Name), ENUM_2_STR(ALIAS))};

enum INSERT_PERF_AND_ALIAS_TEMPLATE_FIELD {
  INSERT_PERF_AND_ALIAS_TEMPLATE_FIELD_Name = 0,
  INSERT_PERF_AND_ALIAS_TEMPLATE_FIELD_ALIAS,
  INSERT_PERF_AND_ALIAS_TEMPLATE_FIELD_ALIAS_VALUE,
};

int CastBaseDb::ReadFromImageHost(const QString& imgsHostOriPath) {
  if (!QFileInfo(imgsHostOriPath).isDir()) {
    LOG_W("Directory[%s] not exist", qPrintable(imgsHostOriPath));
    return FD_NOT_DIR;
  }

  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }
  SafeTransaction safeTrans{db};
  if (!safeTrans) {
    return FD_TRANSACTION_FAILED;
  }

  QSqlQuery qry{db};
  if (!qry.prepare(INSERT_NAME_ORI_IMGS_TEMPLATE.arg(DB_TABLE::PERFORMERS))) {
    LOG_W("prepare command[%s] failed: %s",  //
          qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    return FD_PREPARE_FAILED;
  }

  const TCast2OriImgs& cast2OriImgs = FromFileSystemStructure(imgsHostOriPath);
  int succeedCnt = 0;
  for (auto mpIt = cast2OriImgs.cbegin(); mpIt != cast2OriImgs.cend(); ++mpIt) {
    const QString& perf = mpIt.key();
    const QString& ori = mpIt.value().first;
    const QString& imgsStr = mpIt.value().second.join(StringTool::PERFS_VIDS_IMGS_SPLIT_CHAR);  // img seperated by \n
    qry.bindValue(INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD_Name, perf);
    qry.bindValue(INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD_Ori, ori);
    qry.bindValue(INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD_Imgs, imgsStr);
    qry.bindValue(INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD_Ori_VALUE, ori);
    qry.bindValue(INSERT_NAME_ORI_IMGS_TEMPLATE_FIELD_Imgs_VALUE, imgsStr);
    if (!qry.exec()) {
      LOG_W("replace[%s] failed: %s",  //
            qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
      return FD_EXEC_FAILED;
    }
    ++succeedCnt;
  }

  if (!safeTrans.commit()) {
    return FD_COMMIT_FAILED;
  }
  qry.finish();
  LOG_D("Read %d perfs out from path[%s] succeed", succeedCnt, qPrintable(imgsHostOriPath));
  return succeedCnt;
}

int CastBaseDb::LoadFromPsonFile(const QString& imgsHostOriPath) {
  if (!QFileInfo(imgsHostOriPath).isDir()) {
    LOG_W("Directory[%s] not exist", qPrintable(imgsHostOriPath));
    return FD_NOT_DIR;
  }

  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }
  SafeTransaction safeTrans{db};
  if (!safeTrans) {
    return FD_TRANSACTION_FAILED;
  }

  QSqlQuery qry{db};
  if (!qry.prepare(INSERT_FULL_FIELDS_TEMPLATE.arg(DB_TABLE::PERFORMERS))) {
    LOG_W("prepare command[%s] failed: %s",  //
          qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    return FD_PREPARE_FAILED;
  }

  int succeedCnt = 0;
  QDirIterator it{imgsHostOriPath, {"*.pson"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  using namespace PERFORMER_DB_HEADER_KEY;
  while (it.hasNext()) {
    const QString psonPath = it.next();
    const QVariantHash& pson = JsonHelper::MovieJsonLoader(psonPath);
    if (pson.isEmpty()) {
      LOG_D("psonPath[%s] is empty", qPrintable(psonPath));
      continue;
    }

#define PSON_KEY_ITEM(enu, enumVal, defaultValue, sqlRecordToValueFunc, tblFieldDefinition) qry.bindValue(INSERT_FULL_FIELDS_TEMPLATE_FIELD_##enu, pson[ENUM_2_STR(enu)]);
    PSON_MODEL_FIELD_MAPPING
#undef PSON_KEY_ITEM

    if (!qry.exec()) {
      LOG_W("replace[%s] failed: %s",  //
            qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
      return FD_EXEC_FAILED;
    }
    ++succeedCnt;
  }

  if (!safeTrans.commit()) {
    return FD_COMMIT_FAILED;
  }
  qry.finish();
  LOG_D("Read %d perfs out from path[%s] succeed", succeedCnt, qPrintable(imgsHostOriPath));
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
    LOG_D("Read no perfs out from text");
    return FD_SKIP;
  }
  LOG_D("Read %d perf(s) out from text", perfs.size());

  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }
  if (!db.transaction()) {
    LOG_W("start the %dth transaction failed: %s", 1, qPrintable(db.lastError().text()));
    return FD_TRANSACTION_FAILED;
  }

  QSqlQuery qry{db};
  if (!qry.prepare(INSERT_PERF_AND_ALIAS_TEMPLATE.arg(DB_TABLE::PERFORMERS))) {
    LOG_W("prepare command[%s] failed: %s",  //
          qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    return FD_PREPARE_FAILED;
  }
  int succeedCnt = 0;
  // update aka by new value if name conflict
  for (auto it = perfs.cbegin(); it != perfs.cend(); ++it) {
    qry.bindValue(INSERT_PERF_AND_ALIAS_TEMPLATE_FIELD_Name, it.key());
    qry.bindValue(INSERT_PERF_AND_ALIAS_TEMPLATE_FIELD_ALIAS, it.value());
    qry.bindValue(INSERT_PERF_AND_ALIAS_TEMPLATE_FIELD_ALIAS_VALUE, it.value());
    if (!qry.exec()) {
      LOG_W("Insert[%s] failed: %s",  //
            qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
      db.rollback();
      return FD_EXEC_FAILED;
    }
    ++succeedCnt;
  }
  if (!db.commit()) {
    LOG_W("commit failed: %s", qPrintable(db.lastError().text()));
    db.rollback();
    return FD_COMMIT_FAILED;
  }
  qry.finish();
  LOG_D("Read %d perfs out from text user input[%s] succeed", succeedCnt, qPrintable(perfsText));
  return succeedCnt;
}

bool CastBaseDb::UpdateRecordImgsField(QSqlRecord& sqlRecord, const QString& imageHostPath) {
  QDir castDir{GetCastPath(sqlRecord, imageHostPath), "", QDir::SortFlag::Name, QDir::Filter::Files};
  castDir.setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
  if (!castDir.exists()) {
    LOG_D("cast path[%s] not exist", qPrintable(castDir.absolutePath()));
    return false;  // no need update
  }
  const QString newImages = castDir.entryList().join(StringTool::PERFS_VIDS_IMGS_SPLIT_CHAR);
  if (sqlRecord.value(PERFORMER_DB_HEADER_KEY::Imgs).toString() == newImages) {
    return false;  // no need update
  }
  sqlRecord.setValue(PERFORMER_DB_HEADER_KEY::Imgs, newImages);
  return true;
}

QString CastBaseDb::GetCastPath(const QSqlRecord& sqlRecord, const QString& imageHostPath) {
  return imageHostPath + '/'                                                                        //
         + sqlRecord.field(INSERT_FULL_FIELDS_TEMPLATE_FIELD_Ori).value().toString() + '/'  //
         + sqlRecord.field(INSERT_FULL_FIELDS_TEMPLATE_FIELD_Name).value().toString();              //
}

QString CastBaseDb::GetCastFilePath(const QSqlRecord& sqlRecord, const QString& imageHostPath) {
  return GetCastPath(sqlRecord, imageHostPath) + '/'                                   //
         + sqlRecord.field(INSERT_FULL_FIELDS_TEMPLATE_FIELD_Name).value().toString()  //
         + ".pson";
}

auto CastBaseDb::FromFileSystemStructure(const QString& imgsHostOriPath) -> TCast2OriImgs {
  TCast2OriImgs cast2OriImgs;
  const int IMG_RELPATH_START_INDEX = imgsHostOriPath.size() + 1;  // "C:/home/to/imageHost/"   "ori/cast/cast.jpg"
  QDirIterator it{imgsHostOriPath, TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  while (it.hasNext()) {
    it.next();
    const QString imgRelativePath = it.filePath().mid(IMG_RELPATH_START_INDEX);
    const QStringList& imgPathParts = imgRelativePath.split('/');  // ori/cast/img
    const int pathSectionSize = imgPathParts.size();
    if (pathSectionSize != 3) {
      continue;
    }
    const QString& ori{imgPathParts[0]};
    const QString& castName{imgPathParts[1]};
    const QString& imgName{imgPathParts[2]};

    auto it = cast2OriImgs.find(castName);
    if (it == cast2OriImgs.end()) {
      cast2OriImgs[castName] = std::make_pair(ori, QStringList{imgName});
    } else {
      it.value().second.push_back(imgName);
    }
  }
  using namespace StringTool;
  for (auto it = cast2OriImgs.begin(); it != cast2OriImgs.end(); ++it) {
    ImgsSortNameFirst(it.value().second);
  }
  return cast2OriImgs;
}

int CastBaseDb::WhenCastNameRenamed(const QString& imgsHostOriPath, const QString& oldName, const QString& newName) {
  if (oldName == newName) {
    return 0;
  }
  if (NameTool::IsFileNameInvalid(newName)) {
    LOG_W("New name[%s] invalid", qPrintable(newName));
    return -1;
  }

  if (newName.contains(JSON_RENAME_REGEX::INVALID_CHARS_IN_FILENAME)) {
    LOG_W("New name[%s] invalid should not contain invalid chars", qPrintable(newName));
    return -1;
  }

  QDir oriDir{imgsHostOriPath};
  if (!oriDir.exists()) {
    LOG_D("Host Image Path[%s] does not exist.", qPrintable(imgsHostOriPath));
    return -1;
  }
  if (oriDir.exists(newName)) {
    LOG_D("New Name[%s] already been occupied.", qPrintable(newName));
    return -1;
  }
  int renameCount = 0;

  // 1. rename files under ori. imgsHostOriPath/oldName/oldNameXX -> imgsHostOriPath/oldName/newNameXX
  QDir oldir{imgsHostOriPath + "/" + oldName, "", QDir::SortFlag::Name, QDir::Filter::Files};  // oldName + "*", QDir::SortFlag::Name
  oldir.setNameFilters({oldName + "*"});
  for (const QString& oldFileName : oldir.entryList()) {
    QString newFileName = oldFileName;
    newFileName.replace(oldName, newName);
    if (newFileName == oldFileName) {
      continue;
    }
    if (!oldir.rename(oldFileName, newFileName)) {
      LOG_W("Rename file from %s->%s failed.", qPrintable(oldFileName), qPrintable(newFileName));
      return -1;
    }
    ++renameCount;
  }

  // 2. rename ori itself,  imgsHostOriPath/oldName -> imgsHostOriPath/newName
  if (!oriDir.rename(oldName, newName)) {
    LOG_W("Renamed directory from %s->%s failed.", qPrintable(oldName), qPrintable(newName));
    return -1;
  }
  ++renameCount;

  return renameCount;
}
