#include "PreviewTypeToolBar.h"
#include "MemoryKey.h"
#include "StyleSheet.h"

PreviewTypeToolBar::PreviewTypeToolBar(const QString &title, QWidget *parent)://
  QToolBar{title, parent}
{
  STACKS_PRE = new (std::nothrow) QAction{QIcon(":img/FOLDER_PREVIEW_LISTS"), "stacks", this};
  STACKS_PRE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key::Key_P));
  STACKS_PRE->setToolTip(QString("<b>%1 (%2)</b><br/> IMAGE/VIDEO/OTHER items by type view and single item text-browser view.")  //
                             .arg(STACKS_PRE->text(), STACKS_PRE->shortcut().toString()));
  STACKS_PRE->setCheckable(true);

  BROWSER_PRE = new (std::nothrow) QAction{QIcon(":img/FOLDER_PREVIEW_BROWSER"), "browser", this};
  BROWSER_PRE->setToolTip(QString("<b>%1 (%2)</b><br/> QTextBrowser(images count dynamic expansion).")  //
                              .arg(BROWSER_PRE->text(), BROWSER_PRE->shortcut().toString()));
  BROWSER_PRE->setCheckable(true);

  SLIDERS_PRE = new (std::nothrow) QAction{QIcon(":img/FOLDER_PREVIEW_LABELS"), "sliders", this};
  SLIDERS_PRE->setToolTip(QString("<b>%1 (%2)</b><br/> QLabels(Slidershow and Periodic timer).")  //
                              .arg(SLIDERS_PRE->text(), SLIDERS_PRE->shortcut().toString()));
  SLIDERS_PRE->setCheckable(true);

  using namespace PreviewTypeTool;
  mPreviewTypeIntAction.init({{STACKS_PRE, PREVIEW_TYPE_E::STACKS},//
                              {BROWSER_PRE, PREVIEW_TYPE_E::BROWSER},//
                              {SLIDERS_PRE, PREVIEW_TYPE_E::SLIDERS}},//
                             PREVIEW_TYPE_E::NONE, QActionGroup::ExclusionPolicy::ExclusiveOptional);//
  int curPreviewType = Configuration().value(MemoryKey::FOLDER_PREVIEW_TYPE.name, (int)mPreviewTypeIntAction.defVal()).toInt();
  mPreviewTypeIntAction.setCheckedIfActionExist(curPreviewType);
  mCurrentPreviewType = mPreviewTypeIntAction.intVal2Enum(curPreviewType);

  addActions(mPreviewTypeIntAction.getActionEnumAscendingList());
  setOrientation(Qt::Orientation::Vertical);
  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  setStyleSheet("QToolBar { max-width: 256px; }");
  setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  subscribe();
}

void PreviewTypeToolBar::subscribe() {
  connect(mPreviewTypeIntAction.getActionGroup(), &QActionGroup::triggered, this, [this](QAction* pPreview){
    mCurrentPreviewType = mPreviewTypeIntAction.act2Enum(pPreview);
    emit previewTypeChanged(mCurrentPreviewType);
  });
}
