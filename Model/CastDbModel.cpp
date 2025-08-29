#include "CastDbModel.h"
#include "CastDBActions.h"
#include "MemoryKey.h"
#include "PublicVariable.h"
#include "TableFields.h"
#include "PublicMacro.h"
#include <QPainter>
#include <QPixmap>
#include <QSqlError>
#include <QSqlRecord>

QPixmap GetRatePixmap(int r, bool hasBorder = false) {
  if (r < 0 || r > CastDbModel::MAX_RATE) {
    qDebug("rate[%d] out bound", r);
    return {};
  }
  static constexpr int WIDTH = 100, HEIGHT = (int)(WIDTH * 0.618);
  QPixmap mp{WIDTH, HEIGHT};
  int orangeWidth = WIDTH * r / CastDbModel::MAX_RATE;
  static constexpr QColor OPAGUE{0, 0, 0, 0};
  static constexpr QColor STD_ORANGE{255, 165, 0, 255};
  mp.fill(OPAGUE); // opague
  QPainter painter{&mp};
  painter.setPen(STD_ORANGE); // standard orange
  painter.setBrush(STD_ORANGE);
  painter.drawRect(0, 0, orangeWidth, HEIGHT);
  if (hasBorder) {
    painter.setPen(QColor{0, 0, 0, 255}); // standard black
    painter.setBrush(OPAGUE);
    painter.drawRect(0, 0, WIDTH - 1, HEIGHT - 1);
  }
  painter.end();
  return mp;
}

constexpr int CastDbModel::MAX_RATE;

CastDbModel::CastDbModel(QObject *parent, QSqlDatabase db) : //
  QSqlTableModel{parent, db},
  m_imageHostPath{Configuration().value(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name,  //
                                        MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.v)
                      .toString()}
{
  if (!QFileInfo{m_imageHostPath}.isDir()) {
    qWarning("ImageHostPath[%s] is not a directory or not exist. cast view function abnormal", qPrintable(m_imageHostPath));
  }
  mSubmitAllAction = g_castAct().SUBMIT;
  CHECK_NULLPTR_RETURN_VOID(mSubmitAllAction)

  if (!db.isValid()) {
    qWarning("db invalid[%s]", qPrintable(db.lastError().text()));
    return;
  }
  if (!db.isOpen() && db.open()) {
    qWarning("db opend failed [%s]", qPrintable(db.lastError().text()));
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
  static const QPixmap PERFORMER_SCORE_BOARD[MAX_RATE + 1] = {GetRatePixmap(0), GetRatePixmap(1), GetRatePixmap(2), GetRatePixmap(3), GetRatePixmap(4), GetRatePixmap(5),
                                                              GetRatePixmap(6), GetRatePixmap(7), GetRatePixmap(8), GetRatePixmap(9), GetRatePixmap(10)};
  if (role == Qt::DecorationRole && index.column() == PERFORMER_DB_HEADER_KEY::Rate) {
    const int sc = QSqlTableModel::data(index, Qt::DisplayRole).toInt();
    return PERFORMER_SCORE_BOARD[(sc > MAX_RATE) ? MAX_RATE : (sc < 0 ? 0 : sc)];
  }

  return QSqlTableModel::data(index, role);
}

bool CastDbModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (index.column() == PERFORMER_DB_HEADER_KEY::AKA) {
    qDebug("Old Value: %s, new Value: %s", qPrintable(data(index, Qt::DisplayRole).toString()), qPrintable(value.toString()));
  }
  return QSqlTableModel::setData(index, value, role);
}

QString CastDbModel::fileName(const QModelIndex& curIndex) const {
  const QModelIndex& nameIndex = curIndex.siblingAtColumn(PERFORMER_DB_HEADER_KEY::Name);
  return data(nameIndex, Qt::ItemDataRole::DisplayRole).toString();
}

QString CastDbModel::filePath(const QModelIndex& curIndex) const {
  const QModelIndex& oriIndex = curIndex.siblingAtColumn(PERFORMER_DB_HEADER_KEY::Ori);
  return m_imageHostPath + '/' + data(oriIndex, Qt::ItemDataRole::DisplayRole).toString() + '/' + fileName(curIndex);
}

QString CastDbModel::portaitPath(const QModelIndex& curIndex) const {
  const auto& rec = record(curIndex.row());
  const QString& imgs = rec.value(PERFORMER_DB_HEADER_KEY::Imgs).toString();
  if (imgs.isEmpty()) {
    return "";
  }
  return m_imageHostPath +
         '/' + rec.value(PERFORMER_DB_HEADER_KEY::Ori).toString() +
         '/' + rec.value(PERFORMER_DB_HEADER_KEY::Name).toString() +
         '/' + imgs.left(imgs.indexOf('\n'));
}

bool CastDbModel::submitAll() {
  QSqlDatabase db = database();
  if (!db.transaction()) {
    qWarning("Begin transaction failed: %s", qPrintable(db.lastError().text()));
    return false;
  }
  if (!QSqlTableModel::submitAll()) {
    qWarning("SubmitAll failed[%s], rollback now", qPrintable(lastError().text()));
    if (!db.rollback()) {
      qWarning("Rollback also failed: %s", qPrintable(db.lastError().text()));
    }
    return false;
  }
  if (!db.commit()) {
    qWarning("Commit failed[%s]", qPrintable(db.lastError().text()));
    return false;
  }
  onUpdateSubmitAllAction();
  return true;
}

void CastDbModel::onUpdateSubmitAllAction() {
  mSubmitAllAction->setEnabled(isDirty());
}
