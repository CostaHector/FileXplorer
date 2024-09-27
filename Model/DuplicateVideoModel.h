#ifndef DUPLICATEVIDEOMODEL_H
#define DUPLICATEVIDEOMODEL_H

#include <QAbstractTableModel>
#include <QDateTime>
#include <QMap>
#include <QSet>
#include <QString>
#include "Tools/AIMediaDuplicate.h"

enum class DIFFER_BY_TYPE {
  DURATION = 0,
  SIZE = 1,
  BOTTOM,
};

typedef QList<QList<DUP_INFO>> CLASSIFIED_SORT_LIST_2D[(int)DIFFER_BY_TYPE::BOTTOM];

class DuplicateDetailsModel : public QAbstractTableModel {
 public:
  DuplicateDetailsModel(QObject* parent = nullptr) : QAbstractTableModel{parent} {}

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  auto rowCount(const QModelIndex& parent = {}) const -> int override;
  auto columnCount(const QModelIndex& parent = {}) const -> int override { return VIDS_DETAIL_HEADER.size(); }

  auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override;

  QString fileNameEverything(const QModelIndex& index) const;
  QString filePath(const QModelIndex& index) const;

  void SyncFrom(CLASSIFIED_SORT_LIST_2D* _classifiedSort, DIFFER_BY_TYPE* _currentDiffer) {
    m_currentDiffer = _currentDiffer;
    p_classifiedSort = _classifiedSort;
  }

  void onChangeDetailIndex(int newRow);
  void whenDifferTypeAboutToChanged();
  void whenItemsRecycledUpdateForgroundColor();

 private:
  int m_leftRow = -1;  // -1: nothing selected in left

  DIFFER_BY_TYPE* m_currentDiffer{nullptr};
  CLASSIFIED_SORT_LIST_2D* p_classifiedSort{nullptr};

  static const QStringList VIDS_DETAIL_HEADER;
};

class VidInfoModel : public QAbstractTableModel {
 public:
  VidInfoModel(QObject* parent = nullptr);

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override { return m_classifiedSort[(int)m_currentDiffer].size(); }
  auto columnCount(const QModelIndex& parent = QModelIndex()) const -> int override { return DUPLICATE_LIST_HEADER.size(); }

  auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override {
    if (role == Qt::TextAlignmentRole) {
      if (orientation == Qt::Vertical) {
        return Qt::AlignRight;
      }
    }
    if (role == Qt::DisplayRole) {
      if (orientation == Qt::Orientation::Horizontal) {
        if (0 <= section and section < DUPLICATE_LIST_HEADER.size())
          return DUPLICATE_LIST_HEADER[section];
      }
      return section + 1;
    }
    return QAbstractTableModel::headerData(section, orientation, role);
  }

  void ChangeTableGroups(const QStringList& path);

  void TryUpdateRowCountAndDisplay();

  void UpdateMemberList();
  QList<QList<DUP_INFO>> getDurationsLst() const;
  QList<QList<DUP_INFO>> getSizeLst() const;

  void setDifferType(const DIFFER_BY_TYPE& newDifferType);

  void setDeviationDuration(int newDuration);

  void setDeviationSize(int newSize);

  static inline int getFuzzyDur(int dur) { return m_deviationDur == 0 ? dur : (dur + m_deviationDur / 2) / m_deviationDur * m_deviationDur; }
  static inline qint64 getFuzzySz(qint64 sz) { return m_deviationSz == 0 ? sz : (sz + m_deviationSz / 2) / m_deviationSz * m_deviationSz; }

  int getReadVidsCount() const { return m_vidsInfo.size(); }
  QString getDifferTypeStr() const {
    switch (m_currentDiffer) {
      case DIFFER_BY_TYPE::SIZE:
        return "Size";
      case DIFFER_BY_TYPE::DURATION:
        return "Duration";
      default:
        return "Unknown";
    }
  }

  DIFFER_BY_TYPE m_currentDiffer = DIFFER_BY_TYPE::SIZE;
  CLASSIFIED_SORT_LIST_2D m_classifiedSort;

 private:
  void resetBeforeAfterRow() { m_beforeRowN = m_afterRow = -1; }
  void setDataChangedFlag() { memset(m_dataChangedFlag, true, (int)DIFFER_BY_TYPE::BOTTOM); }

  int m_beforeRowN = -1, m_afterRow = -1;
  bool m_dataChangedFlag[(int)DIFFER_BY_TYPE::BOTTOM] = {0};

  QList<DUP_INFO> m_vidsInfo;

  static int m_deviationDur;    // ms, 995~1004 => 1000, (v+dev/2)//dev*dev
  static qint64 m_deviationSz;  // bytes

  constexpr static int NAME_LEVEL_COUNT = 1;  // home/to/vid.mov => vid.mov when cnt = 1, to/vid.mov when cnt = 2
  static const QStringList DUPLICATE_LIST_HEADER;
};

#endif  // DUPLICATEVIDEOMODEL_H
