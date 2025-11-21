#ifndef DUPLICATEVIDEOSFINDERACTIONS_H
#define DUPLICATEVIDEOSFINDERACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QToolBar>
#include <QLineEdit>

class DuplicateVideosFinderActions : public QObject {
 public:
  DuplicateVideosFinderActions(QObject* parent = nullptr);

  QToolBar* GetAiMediaToolBar(QLineEdit* tableNameFilterLE, QLineEdit* sizeDevLE, QLineEdit* durationDevLE, QWidget* parent);

  QAction* DIFFER_BY_DURATION{nullptr};
  QAction* DIFFER_BY_SIZE{nullptr};
  QActionGroup* DIFFER_BY{nullptr};

  QAction* OPEN_DATABASE{nullptr};
  QAction* RECYCLE_SELECTIONS{nullptr};

  QAction* CLEAR_ANALYSIS_LIST{nullptr};
  QAction* ANALYSE_THESE_TABLES{nullptr};
  QAction* SCAN_A_PATH{nullptr};
  QAction* AUDIT_THESE_TABLES{nullptr};
  QAction* DROP_THESE_TABLES{nullptr};
  QAction* FORCE_RELOAD_TABLES{nullptr};
};

DuplicateVideosFinderActions& g_dupVidFinderAg();
#endif  // DUPLICATEVIDEOSFINDERACTIONS_H
