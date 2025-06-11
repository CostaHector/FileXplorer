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

  QAction* SHOW_FILES_SIZE{nullptr};
  QAction* SHOW_VIDS_DURATION{nullptr};
  QAction* SHOW_FILES_MD5{nullptr};
};

PropertiesWindowActions& g_propertiesWindowAct();

#endif  // PROPERTIESWINDOWACTIONS_H
