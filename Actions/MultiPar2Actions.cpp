#include "MultiPar2Actions.h"
#include "PublicMacro.h"
#include "StyleSheet.h"
#include "ImageTool.h"
#include <QToolBar>

MultiPar2Actions& MultiPar2Actions::GetInst() {
  static MultiPar2Actions inst;
  return inst;
}

MultiPar2Actions::MultiPar2Actions(QObject* parent)
  : QObject{parent} {
  _CREATE_PAR2_FILES = new QAction{QIcon{":img/CREATE_MULTI_PAR"}, tr("Create Par2"), this};
  _VERIFY_IF_NEED_RECOVERY = new QAction{QIcon{":img/VERIFY_INTEGRITY"}, tr("Verify Integrity"), this};
}

QWidget* MultiPar2Actions::GetToolBar(QWidget* parent) {
  CHECK_NULLPTR_RETURN_NULLPTR(parent);
  QToolBar* tb = new QToolBar{"MultiPar Toolbar", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(tb);
  tb->setOrientation(Qt::Orientation::Vertical);
  tb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  tb->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));

  tb->addAction(_CREATE_PAR2_FILES);
  tb->addAction(_VERIFY_IF_NEED_RECOVERY);
  SetLayoutAlightment(tb->layout(), Qt::AlignLeft);
  return tb;
}
