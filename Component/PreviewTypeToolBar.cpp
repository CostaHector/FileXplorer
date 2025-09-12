#include "PreviewTypeToolBar.h"
#include "MemoryKey.h"
#include "StyleSheet.h"

PreviewTypeToolBar::PreviewTypeToolBar(const QString &title, QWidget *parent)://
  QToolBar{title, parent}
{
  CATEGORY_PRE = new (std::nothrow) QAction{QIcon(":img/FOLDER_PREVIEW_CATEGORY"), "Category Preview", this};
  CATEGORY_PRE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key::Key_P));
  CATEGORY_PRE->setToolTip(QString("<b>%1 (%2)</b><br/> Through 3 QListViews and Items that group file by type(IMAGE/VIDEO/OTHER)")  //
                               .arg(CATEGORY_PRE->text(), CATEGORY_PRE->shortcut().toString()));
  CATEGORY_PRE->setCheckable(true);

  PROGRESSIVE_LOAD_PRE = new (std::nothrow) QAction{QIcon(":img/FOLDER_PREVIEW_PROGRESSIVE_LOAD"), "Progressive Load Preview", this};
  PROGRESSIVE_LOAD_PRE->setToolTip(QString("<b>%1 (%2)</b><br/> Through QTextBrowser(images count dynamic expansion).")  //
                                       .arg(PROGRESSIVE_LOAD_PRE->text(), PROGRESSIVE_LOAD_PRE->shortcut().toString()));
  PROGRESSIVE_LOAD_PRE->setCheckable(true);

  CAROUSEL_PRE = new (std::nothrow) QAction{QIcon(":img/FOLDER_PREVIEW_CAROUSEL"), "Carousel Preview", this};
  CAROUSEL_PRE->setToolTip(QString("<b>%1 (%2)</b><br/> Through QLabels(Slidershow and a periodic timer).")  //
                               .arg(CAROUSEL_PRE->text(), CAROUSEL_PRE->shortcut().toString()));
  CAROUSEL_PRE->setCheckable(true);

  using namespace PreviewTypeTool;
  mPreviewTypeIntAction.init({{CATEGORY_PRE, PREVIEW_TYPE_E::CATEGORY},//
                              {PROGRESSIVE_LOAD_PRE, PREVIEW_TYPE_E::PROGRESSIVE_LOAD},//
                              {CAROUSEL_PRE, PREVIEW_TYPE_E::CAROUSEL}},//
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
