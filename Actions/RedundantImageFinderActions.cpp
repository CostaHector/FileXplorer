#include "RedundantImageFinderActions.h"

#include "public/PublicMacro.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"

RedundantImageFinderActions::RedundantImageFinderActions(QObject* parent)  //
    : QObject{parent} {
  RECYLE_NOW = new (std::nothrow) QAction{QIcon(":img/MOVE_TO_TRASH_BIN"), "Recycle", this};
  CHECK_NULLPTR_RETURN_VOID(RECYLE_NOW);
  RECYLE_NOW->setShortcut(QKeySequence(Qt::KeyboardModifier::NoModifier | Qt::Key::Key_Delete));
  RECYLE_NOW->setToolTip(QString("<b>%1 (%2)</b><br/> Move the selected item(s) to the Recyle Bin.")  //
                             .arg(RECYLE_NOW->text(), RECYLE_NOW->shortcut().toString()));

  RECYCLE_EMPTY_IMAGE = new (std::nothrow) QAction{QIcon(":img/FILE"), "Empty image", this};
  CHECK_NULLPTR_RETURN_VOID(RECYCLE_EMPTY_IMAGE);
  RECYCLE_EMPTY_IMAGE->setCheckable(true);
  RECYCLE_EMPTY_IMAGE->setChecked(PreferenceSettings().value(RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.name, RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.v).toBool());
  RECYCLE_EMPTY_IMAGE->setToolTip(QString("<b>%1 (%2)</b><br/> Blank images (with a file size of 0Byte) will also be considered redundant images.")  //
                                      .arg(RECYCLE_EMPTY_IMAGE->text(), RECYCLE_EMPTY_IMAGE->shortcut().toString()));

  OPEN_REDUNDANT_IMAGES_FOLDER = new (std::nothrow) QAction{QIcon(":img/FOLDER_OPEN"), "Open Redundant library", this};
  CHECK_NULLPTR_RETURN_VOID(OPEN_REDUNDANT_IMAGES_FOLDER);
  OPEN_REDUNDANT_IMAGES_FOLDER->setToolTip(QString("<b>%1 (%2)</b><br/> Open the path where the redundant image library is located.")  //
                                               .arg(OPEN_REDUNDANT_IMAGES_FOLDER->text(), OPEN_REDUNDANT_IMAGES_FOLDER->shortcut().toString()));
}

QToolBar* RedundantImageFinderActions::GetRedunImgTB(QWidget* parent) {
  auto* m_toolBar = new (std::nothrow) QToolBar{"RedundantImageFinderToolbar", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(m_toolBar);
  m_toolBar->addAction(RECYLE_NOW);
  m_toolBar->addSeparator();
  m_toolBar->addAction(RECYCLE_EMPTY_IMAGE);
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
