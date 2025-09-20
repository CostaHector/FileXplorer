#ifndef DUPLICATEVIDEOSFINDERACTIONS_H
#define DUPLICATEVIDEOSFINDERACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QToolBar>

class QLineEdit;

class DuplicateVideosFinderActions : public QObject {
 public:
  DuplicateVideosFinderActions(QObject* parent = nullptr);

  QToolBar* GetAiMediaToolBar(QWidget* parent);
  QMenu* GetMenu(QWidget* parent);

  QLineEdit* tblKWFilter = nullptr;
  QLineEdit* durationDevLE = nullptr;
  QLineEdit* sizeDevLE = nullptr;

  QAction* DIFFER_BY_DURATION{new (std::nothrow) QAction{QIcon{":img/VIDEO_DURATION"}, "Duration Deviation(ms)", this}};
  QAction* DIFFER_BY_SIZE{new (std::nothrow) QAction{QIcon{":img/FILE_SIZE"}, "Size Deviation(B)", this}};
  QActionGroup* DIFFER_BY{new (std::nothrow) QActionGroup{this}};

  QAction* OPEN_DATABASE{new (std::nothrow) QAction{QIcon(":img/SQLITE_APP"), "&Open Database", this}};
  QAction* RECYCLE_ONE_FILE{new (std::nothrow) QAction{QIcon{":img/MOVE_TO_TRASH_BIN"}, "Recycle", this}};

  QAction* CANCEL_ANALYSE{new (std::nothrow) QAction{"Cancel Analyse", this}};
  QAction* ANALYSE_THESE_TABLES{new (std::nothrow) QAction{QIcon(":img/ANALYSE_AI_MEDIA_TABLES"), "Append Analyse", this}};
  QAction* SCAN_A_PATH{new (std::nothrow) QAction{QIcon(":img/LOAD_A_PATH"), "Scan a path", this}};
  QAction* AUDIT_AI_MEDIA_TABLE{new (std::nothrow) QAction{QIcon{":img/AUDIT_AI_MEDIA_DUP"}, "Audit Table", this}};
  QAction* DROP_TABLE{new (std::nothrow) QAction{QIcon{":img/DROP_TABLE"}, "Drop Table", this}};
  QAction* DROP_THEN_REBUILD_THIS_TABLE{new (std::nothrow) QAction{QIcon{""}, "Drop then rebuild", this}};
};

DuplicateVideosFinderActions& g_dupVidFinderAg();
#endif  // DUPLICATEVIDEOSFINDERACTIONS_H
