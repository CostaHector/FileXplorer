#ifndef PROPERTIESWINDOWACTIONS_H
#define PROPERTIESWINDOWACTIONS_H

#include <QAction>
#include <QObject>
#include <QWidget>

class QToolBar;

class PropertiesWindowActions : QObject {
 public:
  explicit PropertiesWindowActions(QObject* parent = nullptr);
  QToolBar* getPropertiesToolBar(QWidget* parent);

  QAction* SHOW_FILES_SIZE{new QAction("Files Size", this)};
  QAction* SHOW_VIDS_DURATION{new QAction(QIcon(":img/VIDEO_DURATION"), "Videos Duration", this)};
  QAction* SHOW_FILES_MD5{new QAction(QIcon(":img/MD5_FILE_IDENTIFIER_PATH"), "Files Identifier", this)};
};

PropertiesWindowActions& g_propertiesWindowAct();

#endif  // PROPERTIESWINDOWACTIONS_H
