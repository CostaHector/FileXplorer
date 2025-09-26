#include "CastDbModel.h"
#include "CastBaseDb.h"
#include "CastDBActions.h"
#include "MemoryKey.h"
#include "PublicVariable.h"
#include "PublicTool.h"
#include "TableFields.h"
#include "PublicMacro.h"
#include <QPainter>
#include <QPixmap>
#include <QSqlError>
#include <QSqlRecord>

constexpr int CastDbModel::MAX_RATE;

CastDbModel::CastDbModel(QObject* parent, QSqlDatabase db)
    :  //
      QSqlTableModel{parent, db},
      m_imageHostPath{Configuration()
                          .value(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name,  //
                                 MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.v)
                          .toString()} {
  if (!QFileInfo{m_imageHostPath}.isDir()) {
    LOG_W("ImageHostPath[%s] is not a directory or not exist. cast view function abnormal", qPrintable(m_imageHostPath));
  }
  mSubmitAllAction = g_castAct().SUBMIT;
  CHECK_NULLPTR_RETURN_VOID(mSubmitAllAction)

  if (!db.isValid()) {
    LOG_W("db invalid[%s]", qPrintable(db.lastError().text()));
    return;
  }
  if (!db.isOpen() && db.open()) {
    LOG_W("db opend failed [%s]", qPrintable(db.lastError().text()));
    return;
  }
  if (db.tables().contains(DB_TABLE::PERFORMERS)) {
    setTable(DB_TABLE::PERFORMERS);
    submitAll();
  }
  setEditStrategy(QSqlTableModel::EditStrategy::OnManualSubmit);

  onUpdateSubmitAllAction();
  connect(this, &QSqlTableModel::dataChanged, this, &CastDbModel::onUpdateSubmitAllAction);
}

QVariant CastDbModel::data(const QModelIndex& index, int role) const {
  using namespace FileTool;
  static const QPixmap PERFORMER_SCORE_BOARD[MAX_RATE + 1]  //
      = {
          GetRatePixmap(0, CastDbModel::MAX_RATE),   //
          GetRatePixmap(1, CastDbModel::MAX_RATE),   //
          GetRatePixmap(2, CastDbModel::MAX_RATE),   //
          GetRatePixmap(3, CastDbModel::MAX_RATE),   //
          GetRatePixmap(4, CastDbModel::MAX_RATE),   //
          GetRatePixmap(5, CastDbModel::MAX_RATE),   //
          GetRatePixmap(6, CastDbModel::MAX_RATE),   //
          GetRatePixmap(7, CastDbModel::MAX_RATE),   //
          GetRatePixmap(8, CastDbModel::MAX_RATE),   //
          GetRatePixmap(9, CastDbModel::MAX_RATE),   //
          GetRatePixmap(10, CastDbModel::MAX_RATE),  //
      };                                             //
  if (role == Qt::DecorationRole && index.column() == PERFORMER_DB_HEADER_KEY::Rate) {
    const int sc = QSqlTableModel::data(index, Qt::DisplayRole).toInt();
    return PERFORMER_SCORE_BOARD[(sc > MAX_RATE) ? MAX_RATE : (sc < 0 ? 0 : sc)];
  }

  return QSqlTableModel::data(index, role);
}

bool CastDbModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (index.column() == PERFORMER_DB_HEADER_KEY::Name) {
    const QString oldName{data(index, Qt::DisplayRole).toString()};
    const QString newName{value.toString()};
    if (oldName == newName) {
      return false;
    }
    // rename folder and files
    const QString imgOriPath{oriPath(index)};
    if (CastBaseDb::WhenCastNameRenamed(imgOriPath, oldName, newName) < 0) {
      LOG_C("Rename failed, not write into db");
      return false;
    }
  }
  return QSqlTableModel::setData(index, value, role);
}

QString CastDbModel::fileName(const QModelIndex& curIndex) const {
  const QModelIndex& nameIndex = curIndex.siblingAtColumn(PERFORMER_DB_HEADER_KEY::Name);
  return data(nameIndex, Qt::ItemDataRole::DisplayRole).toString();
}

QString CastDbModel::filePath(const QModelIndex& curIndex) const {
  return oriPath(curIndex) + '/' + fileName(curIndex);
}

QString CastDbModel::oriPath(const QModelIndex& curIndex) const {
  const QModelIndex& oriIndex = curIndex.siblingAtColumn(PERFORMER_DB_HEADER_KEY::Ori);
  return m_imageHostPath + '/' + data(oriIndex, Qt::ItemDataRole::DisplayRole).toString();
}

QString CastDbModel::portaitPath(const QModelIndex& curIndex) const {
  const auto& rec = record(curIndex.row());
  const QString& imgs = rec.value(PERFORMER_DB_HEADER_KEY::Imgs).toString();
  if (imgs.isEmpty()) {
    return "";
  }
  return m_imageHostPath + '/' + rec.value(PERFORMER_DB_HEADER_KEY::Ori).toString() + '/' + rec.value(PERFORMER_DB_HEADER_KEY::Name).toString() +
         '/' + imgs.left(imgs.indexOf('\n'));
}

bool CastDbModel::submitAll() {
  QSqlDatabase db = database();
  if (!db.transaction()) {
    LOG_W("Begin transaction failed: %s", qPrintable(db.lastError().text()));
    return false;
  }
  if (!QSqlTableModel::submitAll()) {
    LOG_W("SubmitAll failed[%s], rollback now", qPrintable(lastError().text()));
    if (!db.rollback()) {
      LOG_W("Rollback also failed: %s", qPrintable(db.lastError().text()));
    }
    return false;
  }
  if (!db.commit()) {
    LOG_W("Commit failed[%s]", qPrintable(db.lastError().text()));
    return false;
  }
  onUpdateSubmitAllAction();
  return true;
}

void CastDbModel::onUpdateSubmitAllAction() {
  mSubmitAllAction->setEnabled(isDirty());
}
