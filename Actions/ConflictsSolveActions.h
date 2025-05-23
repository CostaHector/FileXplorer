#ifndef CONFLICTSSOLVEACTIONS_H
#define CONFLICTSSOLVEACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QIcon>
#include <QMenu>
#include <QObject>
#include <QToolBar>

class ConflictsSolveActions : public QObject {
  Q_OBJECT
 public:
  explicit ConflictsSolveActions(QObject* parent = nullptr);

  auto GetBatchChangeSelectionToolBar(QWidget* parent) -> QToolBar* {
    auto* conflictsControlBar = new QToolBar{"conflict table bar", parent};
    conflictsControlBar->addAction(SIZE_ACT);
    conflictsControlBar->addAction(DATE_ACT);
    conflictsControlBar->addSeparator();
    conflictsControlBar->addAction(REVERT_ACT);
    conflictsControlBar->addSeparator();
    conflictsControlBar->addAction(ALWAYS_KEEP_LEFT_SIDE);
    conflictsControlBar->addAction(ALWAYS_KEEP_RIGHT_SIDE);
    conflictsControlBar->addSeparator();
    conflictsControlBar->addAction(HIDE_NO_CONFLICT_ITEM);
    conflictsControlBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    return conflictsControlBar;
  }

  auto GetMenu(QWidget* parent) -> QMenu* {
    auto* menu = new QMenu{"conflict table menu", parent};
    menu->addAction(RECYCLE_SELECTION);
    menu->addSeparator();
    menu->addAction(KEEP_SELECTION);
    menu->addSeparator();
    menu->addAction(LEFT_FOLDER);
    menu->addAction(RIGHT_FOLDER);
    return menu;
  }

  QAction* SIZE_ACT{new (std::nothrow) QAction(QIcon(":img/FILESIZE_SCALE"), tr("Size larger"), this)};
  QAction* DATE_ACT{new (std::nothrow) QAction(QIcon(":img/TIME_AXIS"), tr("Date newer"), this)};
  QAction* REVERT_ACT{new (std::nothrow) QAction(QIcon(":img/MIRROR_REVERT"), tr("Revert Selection"), this)};

  QAction* ALWAYS_KEEP_LEFT_SIDE{new (std::nothrow) QAction(QIcon(""), tr("Keep left side"), this)};
  QAction* ALWAYS_KEEP_RIGHT_SIDE{new (std::nothrow) QAction(QIcon(""), tr("Keep right side"), this)};

  QAction* HIDE_NO_CONFLICT_ITEM{new (std::nothrow) QAction(QIcon(""), tr("Hide no conflict"), this)};

  QAction* RECYCLE_SELECTION{new (std::nothrow) QAction(QIcon(":img/RECYCLE"), tr("recycle"), this)};
  QAction* KEEP_SELECTION{new (std::nothrow) QAction(QIcon(""), tr("keep"), this)};

  QAction* LEFT_FOLDER{new (std::nothrow) QAction(QIcon(":img/FOLDER"), "left folder path", this)};
  QAction* RIGHT_FOLDER{new (std::nothrow) QAction(QIcon(":img/FOLDER"), "right folder path", this)};
};

ConflictsSolveActions& g_conflictSolveAct();

#endif  // CONFLICTSSOLVEACTIONS_H
