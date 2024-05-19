#ifndef DUPLICATEVIDEOSFINDERACTIONS_H
#define DUPLICATEVIDEOSFINDERACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QToolBar>

class QLineEdit;

class DuplicateVideosFinderActions : public QObject {
 public:
  DuplicateVideosFinderActions(QObject* parent = nullptr) : QObject{parent} {
    DIFFER_BY_DURATION->setToolTip("Value in [a-dev/2, a+dev/2) will be classified to a\nUnit: ms");
    DIFFER_BY_SIZE->setToolTip("Value in [a-dev/2, a+dev/2) will be classified to a\nUnit: Byte");

    DIFFER_BY->addAction(DIFFER_BY_DURATION);
    DIFFER_BY->addAction(DIFFER_BY_SIZE);
    DIFFER_BY->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

    RECYCLE_ONE_FILE->setShortcut(QKeySequence(Qt::KeyboardModifier::NoModifier | Qt::Key_Delete));

    for (auto* act : DIFFER_BY->actions()) {
      act->setCheckable(true);
    }
    DIFFER_BY_SIZE->setChecked(true);
  }

  QToolBar* getToolBar(QWidget* parent);

  QLineEdit* durationDevLE = nullptr;
  QLineEdit* sizeDevLE = nullptr;

  QAction* APPEND_A_PATH{new QAction{QIcon(":/themes/LOAD_A_PATH"), "Append a path", this}};
  QAction* DIFFER_BY_DURATION{new QAction{QIcon{":/themes/VIDEO_DURATION"}, "Duration Deviation(ms)", this}};
  QAction* DIFFER_BY_SIZE{new QAction{QIcon{":/themes/FILE_SIZE"}, "Size Deviation(B)", this}};
  QActionGroup* DIFFER_BY{new QActionGroup{this}};

  QAction* RECYCLE_ONE_FILE{new QAction{QIcon{":/themes/MOVE_TO_TRASH_BIN"}, "Recycle", this}};
};

DuplicateVideosFinderActions& g_dupVidFinderAg();
#endif  // DUPLICATEVIDEOSFINDERACTIONS_H
