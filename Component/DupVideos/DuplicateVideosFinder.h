#ifndef DUPLICATEVIDEOSFINDER_H
#define DUPLICATEVIDEOSFINDER_H

#include "DuplicateVideosMetaInfoTable.h"
#include "DuplicateVideoGroupsTable.h"
#include "DuplicateVideoDetailsTable.h"
#include "DuplicateVideosHelper.h"
#include <QMainWindow>
#include <QSplitter>
#include <QToolBar>
#include <QLineEdit>

class QComboBox;

class DuplicateVideosFinder : public QMainWindow {
  Q_OBJECT
 public:
  explicit DuplicateVideosFinder(QWidget* parent = nullptr);
  void onAnalyzeDuplicatesInVideosList(const DupVidMetaInfoList& needAnalyzeVidLst);
  void onChangeSizeDeviation();
  void onChangeDurationDeviation();

protected:
  void showEvent(QShowEvent* event) override;
  void closeEvent(QCloseEvent* event) override;
  void keyPressEvent(QKeyEvent* e) override;

 private:
  void onDifferTypeChanged(const int grpByIndex);
  void invalidateLeftGrpTblData();
  void subscribe();

  void UpdateWindowsTitleGroupInfo(const QString& dupVidGroupInfoTitle);
  void UpdateWindowsTitleMetaInfo(const QString& dupVidTblMetaInfoTitle);
  void UpdateWindowsTitle();
  void ReadSettings();

  QToolBar* m_tb{nullptr};
  QComboBox* m_GroupByCB{nullptr};
  QLineEdit* durationDevLE{nullptr};
  QLineEdit* sizeDevLE{nullptr};
  QLineEdit* tableNameFilterLE{nullptr};
  QAction* m_RecycleSelections{nullptr};
  QAction* m_RenameSelectionsReplace{nullptr};

  DuplicateVideosMetaInfoTable* m_leftTables{nullptr};
  DuplicateVideoGroupsTable* m_RightGrpsTbl{nullptr};
  DuplicateVideoDetailsTable* m_bottomDetailsTbl{nullptr};

  QSplitter* m_tables_group_LR_splitter{nullptr};
  QSplitter* m_tables_detail_UD_splitter{nullptr};

  QString mTitleGrpInto;
  QString mTitleMetaInfo;

  bool m_geometryRestoredOnce{false};
};

#endif  // DUPLICATEVIDEOSFINDER_H
