#ifndef JSONMODEL_H
#define JSONMODEL_H
#include "Model/DifferRootFileSystemModel.h"

#include <QDebug>
#include <QSet>

struct JsonProperties {
  explicit JsonProperties(const QString& path);
  static int getPerfsCount(const QString& pth);

  QString jsonPath;
  int perfsCount;
};

class JsonModel : public DifferRootFileSystemModel {
 public:
  explicit JsonModel(QObject* parent = nullptr);

  int appendAPath(const QString& path) override;
  int appendRows(const QStringList& lst) override;

  QString filePath(const QModelIndex& index) const override {
    if (not index.isValid()) {
      qWarning() << "Try to access invalid index" << index;
      return "";
    }
    return m_jsons[index.row()].jsonPath;
  }
  QString filePath(const int row) const override {
    if (not(0 <= row and row < rowCount())) {
      qWarning("Try to access row[%d] not in [0, %d)", row, rowCount());
      return "";
    }
    return m_jsons[row].jsonPath;
  }

  void clear() override;

  auto rowCount(const QModelIndex& /*parent*/ = {}) const -> int override { return m_jsons.size(); }
  auto columnCount(const QModelIndex& /*parent*/ = {}) const -> int override { return 1; }

  auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override {
    if (role == Qt::TextAlignmentRole) {
      if (orientation == Qt::Vertical) {
        return Qt::AlignRight;
      }
    }
    if (role == Qt::DisplayRole) {
      if (orientation == Qt::Orientation::Vertical) {
        return section + 1;
      }
    }
    return QAbstractListModel::headerData(section, orientation, role);
  }

  bool isPerfComplete(int row) const { return m_jsons[row].perfsCount >= m_completeJsonPerfCount; }

  void SetCompletePerfCount(int newCount);
  void updatePerfCount(int row, int newCnt);

 private:
  int m_completeJsonPerfCount;

  QString m_rootPath;
  QList<JsonProperties> m_jsons;
  QSet<QString> m_uniqueSet;
};

#endif  // JSONMODEL_H
