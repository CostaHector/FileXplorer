#include "PropertiesWindowActions.h"
#include <QToolBar>


PropertiesWindowActions::PropertiesWindowActions(QObject* parent) : QObject{parent} {
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

QToolBar* PropertiesWindowActions::getPropertiesToolBar(QWidget* parent) {
  QToolBar* displayItems{new QToolBar{parent}};
  displayItems->addAction(SHOW_FILES_SIZE);
  displayItems->addSeparator();
  displayItems->addAction(SHOW_VIDS_DURATION);
  displayItems->addSeparator();
  displayItems->addAction(SHOW_FILES_MD5);
  displayItems->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextOnly);
  return displayItems;
}

PropertiesWindowActions& g_propertiesWindowAct(){
  static PropertiesWindowActions ins;
  return ins;
}
