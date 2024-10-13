#ifndef PROPERTIESWINDOWACTIONS_H
#define PROPERTIESWINDOWACTIONS_H

#include <QAction>
#include <QObject>
#include <QToolBar>
#include <QWidget>

class PropertiesWindowActions : QObject {
 public:
  explicit PropertiesWindowActions(QObject* parent = nullptr) : QObject{parent} {
    SHOW_FILES_SIZE->setCheckable(true);
    SHOW_FILES_SIZE->setChecked(true);
    SHOW_FILES_SIZE->setToolTip("Display both total files size and each file");

    SHOW_VIDS_DURATION->setCheckable(true);
    SHOW_VIDS_DURATION->setChecked(false);
    SHOW_VIDS_DURATION->setToolTip("Display videos duration in million-second");

    SHOW_FILES_MD5->setCheckable(true);
    SHOW_FILES_MD5->setChecked(false);
    SHOW_FILES_MD5->setToolTip("Display file identifier for each file");
  }

  QToolBar* getPropertiesToolBar(QWidget* parent) {
    QToolBar* displayItems{new QToolBar{parent}};
    displayItems->addActions({SHOW_FILES_SIZE, SHOW_VIDS_DURATION, SHOW_FILES_MD5});
    displayItems->addSeparator();
    displayItems->addAction(FORCE_RECALCULATE);
    displayItems->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    return displayItems;
  }

  QAction* SHOW_FILES_SIZE{new QAction("Files Size", this)};
  QAction* SHOW_VIDS_DURATION{new QAction("Videos Duration", this)};
  QAction* SHOW_FILES_MD5{new QAction(QIcon(":img/MD5_FILE_IDENTIFIER_PATH"), "FilesIdentifier", this)};
  QAction* FORCE_RECALCULATE{new QAction{"Force", this}};
};

PropertiesWindowActions& g_propertiesWindowAct();

#endif  // PROPERTIESWINDOWACTIONS_H
