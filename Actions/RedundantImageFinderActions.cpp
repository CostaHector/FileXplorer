#include "RedundantImageFinderActions.h"

#include "PublicMacro.h"
#include "MemoryKey.h"

RedundantImageFinderActions::RedundantImageFinderActions(QObject* parent)  //
    : QObject{parent} {
  FIND_DUPLICATE_IMGS_BY_LIBRARY = new (std::nothrow) QAction{"By benchmark library", this};
  CHECK_NULLPTR_RETURN_VOID(FIND_DUPLICATE_IMGS_BY_LIBRARY);
  FIND_DUPLICATE_IMGS_BY_LIBRARY->setCheckable(true);
  FIND_DUPLICATE_IMGS_BY_LIBRARY->setChecked(true);
  FIND_DUPLICATE_IMGS_BY_LIBRARY->setToolTip(
      "Active: Search duplicates via benchmark library."
      "Inactive: Compare MD5 checksums in current directory.");

  RECYLE_NOW = new (std::nothrow) QAction{QIcon(":img/MOVE_TO_TRASH_BIN"), "Recycle", this};
  CHECK_NULLPTR_RETURN_VOID(RECYLE_NOW);
  RECYLE_NOW->setShortcut(QKeySequence(Qt::KeyboardModifier::NoModifier | Qt::Key::Key_Delete));
  RECYLE_NOW->setToolTip(QString("<b>%1 (%2)</b><br/> Move the selected item(s) to the Recyle Bin.")  //
                             .arg(RECYLE_NOW->text(), RECYLE_NOW->shortcut().toString()));

  ALSO_EMPTY_IMAGE = new (std::nothrow) QAction{QIcon(":img/FILE"), "Also empty image", this};
  CHECK_NULLPTR_RETURN_VOID(ALSO_EMPTY_IMAGE);
  ALSO_EMPTY_IMAGE->setCheckable(true);
  ALSO_EMPTY_IMAGE->setChecked(Configuration().value(RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.name, RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.v).toBool());
  ALSO_EMPTY_IMAGE->setToolTip(QString("<b>%1 (%2)</b><br/> Blank images (with a file size of 0Byte) will also be considered redundant images.")  //
                                   .arg(ALSO_EMPTY_IMAGE->text(), ALSO_EMPTY_IMAGE->shortcut().toString()));

  OPEN_REDUNDANT_IMAGES_FOLDER = new (std::nothrow) QAction{QIcon(":img/FOLDER_OPEN"), "Open Redundant library", this};
  CHECK_NULLPTR_RETURN_VOID(OPEN_REDUNDANT_IMAGES_FOLDER);
  OPEN_REDUNDANT_IMAGES_FOLDER->setToolTip(QString("<b>%1 (%2)</b><br/> Open the path where the redundant image library(benchmark) is located.")  //
                                               .arg(OPEN_REDUNDANT_IMAGES_FOLDER->text(), OPEN_REDUNDANT_IMAGES_FOLDER->shortcut().toString()));
}

QToolBar* RedundantImageFinderActions::GetRedunImgTB(QWidget* parent) {
  auto* m_toolBar = new (std::nothrow) QToolBar{"RedundantImageFinderToolbar", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(m_toolBar);
  m_toolBar->addAction(RECYLE_NOW);
  m_toolBar->addSeparator();
  m_toolBar->addAction(FIND_DUPLICATE_IMGS_BY_LIBRARY);
  m_toolBar->addSeparator();
  m_toolBar->addAction(ALSO_EMPTY_IMAGE);
  m_toolBar->addSeparator();
  m_toolBar->addAction(OPEN_REDUNDANT_IMAGES_FOLDER);
  m_toolBar->addSeparator();
  m_toolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  return m_toolBar;
}

RedundantImageFinderActions& g_redunImgFinderAg() {
  static RedundantImageFinderActions ins;
  return ins;
}
