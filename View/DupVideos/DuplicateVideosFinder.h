#ifndef DUPLICATEVIDEOSFINDER_H
#define DUPLICATEVIDEOSFINDER_H

#include "DuplicateVideosMetaInfoTable.h"
#include "LeftVideoGroupsTable.h"
#include "RightVideoDuplicatesDetails.h"
#include "DuplicateVideosHelper.h"
#include <QMainWindow>
#include <QSplitter>
#include <QToolBar>
#include <QLineEdit>

class DuplicateVideosFinder : public QMainWindow {
  Q_OBJECT
 public:
  explicit DuplicateVideosFinder(QWidget* parent = nullptr);

  void onAnalyzeDuplicatesInVideosList(const DupVidMetaInfoList& needAnalyzeVidLst);
  void onDifferTypeChanged(QAction* newDifferAct);
  void onChangeSizeDeviation();
  void onChangeDurationDeviation();

  void UpdateWindowsTitleGroupInfo(const QString& dupVidGroupInfoTitle);
  void UpdateWindowsTitleMetaInfo(const QString& dupVidTblMetaInfoTitle);
  void UpdateWindowsTitle();
  void updateWindowsSize();

  void showEvent(QShowEvent* event) override;
  void closeEvent(QCloseEvent* event) override;

  void keyPressEvent(QKeyEvent* e) override;

  bool operator()(const QString& /**/) { return true; }

 private:
  void invalidateLeftGrpTblData();
  void subscribe();

  QLineEdit* tableNameFilterLE = nullptr;
  QLineEdit* durationDevLE = nullptr;
  QLineEdit* sizeDevLE = nullptr;
  QToolBar* m_tb{nullptr};
  DuplicateVideosMetaInfoTable* m_aiTables{nullptr};

  LeftVideoGroupsTable* m_leftGrpsTbl{nullptr};
  RightVideoDuplicatesDetails* m_rightDetailsTbl{nullptr};

  QSplitter* m_detail_left_right{nullptr};
  QSplitter* m_tbl_detail_ver{nullptr};

  QString mTitleGrpInto;
  QString mTitleMetaInfo;
};

#endif  // DUPLICATEVIDEOSFINDER_H
