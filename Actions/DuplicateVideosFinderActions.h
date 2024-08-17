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

  QAction* DIFFER_BY_DURATION{new QAction{QIcon{":/themes/VIDEO_DURATION"}, "Duration Deviation(ms)", this}};
  QAction* DIFFER_BY_SIZE{new QAction{QIcon{":/themes/FILE_SIZE"}, "Size Deviation(B)", this}};
  QActionGroup* DIFFER_BY{new QActionGroup{this}};

  QAction* RECYCLE_ONE_FILE{new QAction{QIcon{":/themes/MOVE_TO_TRASH_BIN"}, "Recycle", this}};

  QAction* CANCEL_ANALYSE{new QAction{"Cancel Analyse", this}};
  QAction* ANALYSE_THESE_TABLES{new QAction{QIcon(":/themes/ANALYSE_AI_MEDIA_TABLES"), "Append Analyse", this}};
  QAction* SCAN_A_PATH{new QAction{QIcon(":/themes/LOAD_A_PATH"), "Scan a path", this}};
  QAction* AUDIT_AI_MEDIA_TABLE{new QAction{QIcon{":/themes/AUDIT_AI_MEDIA_DUP"}, "Audit Table", this}};
  QAction* DROP_TABLE{new QAction{QIcon{":/themes/DROP_TABLE"}, "Drop Table", this}};
  QAction* DROP_THEN_REBUILD_THIS_TABLE{new QAction{QIcon{""}, "Drop then rebuild", this}};
};

DuplicateVideosFinderActions& g_dupVidFinderAg();
#endif  // DUPLICATEVIDEOSFINDERACTIONS_H
