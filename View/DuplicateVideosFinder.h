#ifndef DUPLICATEVIDEOSFINDER_H
#define DUPLICATEVIDEOSFINDER_H

#include "View/CustomTableView.h"
#include "View/AiMediaDupTableView.h"

#include "Model/DuplicateVideoModel.h"

#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QSplitter>
#include <QToolBar>

class DuplicateVideosFinder;
class QSortFilterProxyModel;

class LeftDuplicateList : public CustomTableView {
 public:
  friend class DuplicateVideosFinder;
  LeftDuplicateList(QWidget* parent = nullptr);

 private:
  void subscribe();
  VidInfoModel* m_dupListModel{nullptr};
  QSortFilterProxyModel* m_sortProxy{nullptr};
};
// -------------------------------------------------------------------------------------------------
class RightDuplicateDetails : public CustomTableView {
 public:
  friend class DuplicateVideosFinder;
  RightDuplicateDetails(QWidget* parent = nullptr);
  void on_effectiveNameCopiedForEverything(const QModelIndex& ind) const;
  void on_cellDoubleClicked(const QModelIndex& ind) const;
  void setSharedMember(CLASSIFIED_SORT_LIST_2D* pClassifiedSort, DIFFER_BY_TYPE* pCurDifferType);
  void onRecycleSelection();

 private:
  void subscribe();
  DuplicateDetailsModel* m_detailsModel{nullptr};
  QSortFilterProxyModel* m_sortProxy{nullptr};
};
// -------------------------------------------------------------------------------------------------
class DuplicateVideosFinder : public QMainWindow {
 public:
  DuplicateVideosFinder(QWidget* parent = nullptr);
  bool TablesGroupChangedTo(const QStringList& tbls);
  
  void onAnalyseAiMediaTableChanged();
  void onCancelAnalyse();
  void onDifferTypeChanged(QAction* newDifferAct);

  void onChangeSizeDeviation();
  void onChangeDurationDeviation();
  void on_selectionChanged();

  void UpdateAiMediaTableNames();

  void UpdateWindowsTitle();

  void updateWindowsSize();

  void closeEvent(QCloseEvent* event) override;

  void keyPressEvent(QKeyEvent* e) override;
 private:
  void subscribe();

  QToolBar* m_tb{nullptr};
  LeftDuplicateList* m_dupList{nullptr};
  RightDuplicateDetails* m_details{nullptr};

  QToolBar* m_aiTablesTB{nullptr};
  AiMediaDupTableView* m_aiTables{nullptr};

  QSplitter* m_mainWidget{nullptr};

  static const QString DUPLICATE_FINDER_TEMPLATE;
};

#endif  // DUPLICATEVIDEOSFINDER_H
