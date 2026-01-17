#include "PropertiesWindowActions.h"
#include "PublicMacro.h"
#include <QToolBar>

PropertiesWindowActions::PropertiesWindowActions(QObject* parent)  //
    : QObject{parent}                                              //
{
  SHOW_FILES_SIZE = new (std::nothrow) QAction{QIcon(":img/FILE_SIZE"), tr("Files Size"), this};
  CHECK_NULLPTR_RETURN_VOID(SHOW_FILES_SIZE)
  SHOW_FILES_SIZE->setCheckable(true);
  SHOW_FILES_SIZE->setChecked(true);
  SHOW_FILES_SIZE->setToolTip("Display both total files size and each file");

  SHOW_VIDS_DURATION = new (std::nothrow) QAction{QIcon(":img/VIDEO_DURATION"), tr("Videos Duration"), this};
  CHECK_NULLPTR_RETURN_VOID(SHOW_VIDS_DURATION)
  SHOW_VIDS_DURATION->setCheckable(true);
  SHOW_VIDS_DURATION->setChecked(false);
  SHOW_VIDS_DURATION->setToolTip("Display videos duration in million-second");

  SHOW_FILES_MD5 = new (std::nothrow) QAction{QIcon(":img/MD5_FILE_IDENTIFIER_PATH"), tr("Files Identifier"), this};
  CHECK_NULLPTR_RETURN_VOID(SHOW_FILES_MD5)
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
  displayItems->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return displayItems;
}

PropertiesWindowActions& g_propertiesWindowAct() {
  static PropertiesWindowActions ins;
  return ins;
}
