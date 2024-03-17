#ifndef JSONMODEL_H
#define JSONMODEL_H
#include <QAbstractListModel>
#include <QSet>

struct JsonProperties {
  explicit JsonProperties(const QString& path);
  static int getPerfsCount(const QString& pth);

  QString jsonPath;
  int perfsCount;
};

class JsonModel : public QAbstractListModel {
 public:
  explicit JsonModel(QObject* parent = nullptr);

  QModelIndex setRootPath(const QString& path);

  auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override { return m_jsons.size(); }
  auto columnCount(const QModelIndex& parent = QModelIndex()) const -> int override { return 1; }

  auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override;

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

  Qt::ItemFlags flags(const QModelIndex& index) const override {
    if (index.column() == 2) {
      return Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
    }
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
  }

  QString filePath(const QModelIndex& index) const {
    if (not index.isValid()) {
      return "";
    }
    return m_jsons[index.row()].jsonPath;  // Todo. May some int bool here
  }

  bool isPerfComplete(int row) const { return m_jsons[row].perfsCount >= m_completeJsonPerfCount; }

  void clear();

  void SetCompletePerfCount(int newCount);
  void updatePerfCount(int row);
  void setPerfCount(int row, int newCount);

 private:
  int m_completeJsonPerfCount;

  QString m_rootPath;
  QList<JsonProperties> m_jsons;
  QSet<QString> m_uniqueSet;
};

#endif  // JSONMODEL_H
