#include "SqlTableTestPreconditionTool.h"
#include "JsonHelper.h"
#include "CastPsonFileHelper.h"
#include "Logger.h"
#include <QDebug>

namespace SqlTableTestPreconditionTool {
bool CheckIndexesDisplayRoleIgnoreOrder(const QSqlTableModel& model,  //
                                        const QModelIndexList& indexes,
                                        QStringList expectsList,
                                        Qt::ItemDataRole role) {
  QStringList actualsList;
  for (const QModelIndex& ind : indexes) {
    actualsList.push_back(model.data(ind, role).toString());
  }
  std::sort(actualsList.begin(), actualsList.end());
  std::sort(expectsList.begin(), expectsList.end());
  int n1 = actualsList.size(), n2 = expectsList.size();
  if (actualsList == expectsList) {
    return true;
  }
  if (n1 != n2) {
    LOG_W("list length[%d, %d] not equal", n1, n2);
    return false;
  }

  qDebug() << "expects:" << expectsList;
  qDebug() << "actuals:" << actualsList;
  return false;
}

bool CheckIndexesDisplayRoleIgnoreOrder(const QSqlTableModel& model,  //
                                        const QModelIndexList& indexes,
                                        QList<int> expectsList,
                                        Qt::ItemDataRole role) {
  QList<int> actualsList;
  for (const QModelIndex& ind : indexes) {
    actualsList.push_back(model.data(ind, role).toInt());
  }
  std::sort(actualsList.begin(), actualsList.end());
  std::sort(expectsList.begin(), expectsList.end());
  int n1 = actualsList.size(), n2 = expectsList.size();
  if (actualsList == expectsList) {
    return true;
  }
  if (n1 != n2) {
    LOG_W("list length[%d, %d] not equal", n1, n2);
    return false;
  }

  qDebug() << "expects:" << expectsList;
  qDebug() << "actuals:" << actualsList;
  return false;
}

QModelIndexList GetIndexessAtOneRow(const QSqlTableModel& model, int rowBegin, int rowEnd, int column) {
  if (column < 0 || column >= model.columnCount()) {
    LOG_E("column[%d] out of range [0, %d)", column, model.columnCount());
    return {};
  }
  if (rowBegin > rowEnd || (rowBegin < 0) || (rowEnd > model.rowCount())) {
    LOG_E("row range invalid: must [beg:%d <= end:%d] an in [0, %d)", rowBegin, rowEnd, 0, model.rowCount());
    return {};
  }

  QModelIndexList indexesNames;
  indexesNames.reserve(rowEnd - rowBegin);
  for (int r = rowBegin; r < rowEnd; ++r) {
    indexesNames.push_back(model.index(r, column));
  }
  return indexesNames;
}

bool CreateFileStructure(TDir& tDir) {
  // imageHostPath/ori/name/name.pson
  // tDir.path()/.../.../...
  if (!tDir.IsValid()) {
    LOG_W("tDir invalid");
    return false;
  }
  const QList<FsNodeEntry> structureNodes{
      {"SuperHero/Chris Evans/Chris Evans 1.jpg", false, "Chris Evans 1"},
      {"SuperHero/Chris Evans/Chris Evans 2.jpg", false, "Chris Evans 2"},
      {"SuperHero/Chris Hemsworth/Chris Hemsworth.jpg", false, "Chris Hemsworth"},
      {"SuperHero/Chris Pine/Chris Pine.jpg", false, "Chris Pine"},
      {"X-MEN/Michael Fassbender/Michael Fassbender.jpg", false, "Michael Fassbender"},
      {"Football/Cristiano Ronaldo/Cristiano Ronaldo.jpg", false, "Cristiano Ronaldo"},
  };
  return tDir.createEntries(structureNodes) == 6;  // 6 files in total
}

bool CreateFileStructurePsonFiles(TDir& tDir) {
  if (!tDir.IsValid()) {
    LOG_W("tDir invalid");
    return false;
  }

  QByteArray chrisEvansPsonContents =
      CastPsonFileHelper::CastValues2PsonStr("Chris Evans", 10, "Captain,Steve", "hero,movie star", "SuperHero", -1, -1, "", "Chris Evans in captain america.mp4",
                                             "Chris Evans portait.jpg", "Chris Evans, American movie star");
  QByteArray chrisHemesworthPsonContents =
      CastPsonFileHelper::CastValues2PsonStr("Chris Hemsworth", 9, "Thor", "hero,movie star", "SuperHero", -1, -1, "", "Chris Hemsworth in captain america.mp4",
                                             "Chris Hemsworth portait.jpg", "Chris Hemsworth, Australia movie star");
  QByteArray jamesCaviezelPsonContents = CastPsonFileHelper::CastValues2PsonStr("James Caviezel", 9, "General Zod", "hero,movie star", "Action", -1, -1, "",
                                                                                "James Caviezel in Man of Steel - General Zod.mp4",
                                                                                "James Caviezel portait.jpg", "James Caviezel, American movie star");

  const QList<FsNodeEntry> psonNodes{
      {"SuperHero/Chris Evans/Chris Evans.pson", false, chrisEvansPsonContents},
      {"SuperHero/Chris Hemsworth/Chris Hemsworth.pson", false, chrisHemesworthPsonContents},
      {"Action/James Caviezel/James Caviezel.pson", false, jamesCaviezelPsonContents},
  };
  return tDir.createEntries(psonNodes) == 3;
}

}  // namespace SqlTableTestPreconditionTool
