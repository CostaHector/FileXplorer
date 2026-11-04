#ifndef DUPLICATEVIDEOSFINDER_H
#define DUPLICATEVIDEOSFINDER_H

#include "DuplicateVideosMetaInfoTable.h"
#include "DuplicateVideoGroupsTable.h"
#include "DuplicateVideoDetailsTable.h"
#include "DuplicateVideosHelper.h"

#include <QSplitter>
#include <QToolBar>
#include <QLineEdit>

class QComboBox;
class VideoPlayerSplitter;

class DuplicateVideosFinder : public QSplitter {
  Q_OBJECT
 public:
  explicit DuplicateVideosFinder(QWidget* parent = nullptr);
  ~DuplicateVideosFinder();
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

  void saveSplitterStateOnClose();

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
  QAction* m_showVideoPlayer{nullptr};

  DuplicateVideosMetaInfoTable* m_leftTables{nullptr};
  DuplicateVideoGroupsTable* m_RightGrpsTbl{nullptr};
  DuplicateVideoDetailsTable* m_bottomDetailsTbl{nullptr};

  QSplitter* m_tables_group_LR_splitter{nullptr};

  VideoPlayerSplitter* m_videoPlayers{nullptr};

  QString mTitleGrpInto;
  QString mTitleMetaInfo;

  bool m_geometryRestoredOnce{false};
};

#endif  // DUPLICATEVIDEOSFINDER_H
