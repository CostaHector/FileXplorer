#include "PreviewDockWidget.h"
#include "StyleSheet.h"
#include "ImageTool.h"
#include "MemoryKey.h"
#include "Configuration.h"
#include "ViewActions.h"

PreviewDockWidget::PreviewDockWidget(const QString& title, QWidget* parent, Qt::WindowFlags flags) : QDockWidget{title, parent, flags} {
  CATEGORY_PRE = new (std::nothrow) QAction{QIcon(":img/FOLDER_PREVIEW_CATEGORY"), tr("Category Preview"), this};
  CATEGORY_PRE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key::Key_P));
  CATEGORY_PRE->setToolTip(QString("<b>%1 (%2)</b><br/> Through 3 QListViews and Items that group file by type(IMAGE/VIDEO/OTHER)")  //
                               .arg(CATEGORY_PRE->text(), CATEGORY_PRE->shortcut().toString()));
  CATEGORY_PRE->setCheckable(true);

  PROGRESSIVE_LOAD_PRE = new (std::nothrow) QAction{QIcon(":img/FOLDER_PREVIEW_PROGRESSIVE_LOAD"), tr("Progressive Load Preview"), this};
  PROGRESSIVE_LOAD_PRE->setToolTip(QString("<b>%1 (%2)</b><br/> Through QTextBrowser(images count dynamic expansion).")  //
                                       .arg(PROGRESSIVE_LOAD_PRE->text(), PROGRESSIVE_LOAD_PRE->shortcut().toString()));
  PROGRESSIVE_LOAD_PRE->setCheckable(true);

  CAROUSEL_PRE = new (std::nothrow) QAction{QIcon(":img/FOLDER_PREVIEW_CAROUSEL"), tr("Carousel Preview"), this};
  CAROUSEL_PRE->setToolTip(QString("<b>%1 (%2)</b><br/> Through QLabels(Slidershow and a periodic timer).")  //
                               .arg(CAROUSEL_PRE->text(), CAROUSEL_PRE->shortcut().toString()));
  CAROUSEL_PRE->setCheckable(true);

  QAction* SHOW_OR_HIDE_PREVIEW = g_viewActions()._PREVIEW_PANEL;

  using namespace PreviewTypeTool;
  mPreviewTypeIntAction.init({{CATEGORY_PRE, PREVIEW_TYPE_E::CATEGORY},                         //
                              {PROGRESSIVE_LOAD_PRE, PREVIEW_TYPE_E::PROGRESSIVE_LOAD},         //
                              {CAROUSEL_PRE, PREVIEW_TYPE_E::CAROUSEL}},                        //
                             DEFULT_PREVIEW_TYPE_E, QActionGroup::ExclusionPolicy::Exclusive);  //
  int curPreviewType = Configuration().value(CompoVisKey::FOLDER_PREVIEW_TYPE.name, CompoVisKey::FOLDER_PREVIEW_TYPE.name).toInt();
  mPreviewTypeIntAction.setCheckedIfActionExist(curPreviewType);

  m_floatingPanel = new QAction{ImageTool::GetBuiltInIcon(QStyle::SP_TitleBarNormalButton), tr("Float"), this};
  m_floatingPanel->setCheckable(true);

  m_minimizePanel = new QAction{ImageTool::GetBuiltInIcon(QStyle::SP_TitleBarMinButton), tr("Minimize"), this};

  QAction* originalCloseAction = toggleViewAction();
  originalCloseAction->setCheckable(false);
  originalCloseAction->setText("close");
  originalCloseAction->setIcon(ImageTool::GetBuiltInIcon(QStyle::SP_DockWidgetCloseButton));

  m_titleBar = new ToolBarWidget{QBoxLayout::Direction::LeftToRight, this};
  m_titleBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
  m_windowsTitleLabel = m_titleBar->addString("Preview");
  m_titleBar->addStretch(1);
  m_titleBar->addSeparator();
  m_titleBar->addAction(SHOW_OR_HIDE_PREVIEW);
  m_titleBar->addSeparator();
  m_titleBar->addAction(CATEGORY_PRE);
  m_titleBar->addAction(PROGRESSIVE_LOAD_PRE);
  m_titleBar->addAction(CAROUSEL_PRE);
  m_titleBar->addSeparator();
  m_titleBar->addAction(m_floatingPanel);
  m_titleBar->addAction(m_minimizePanel);
  m_titleBar->addAction(originalCloseAction);

  setTitleBarWidget(m_titleBar);
  subscribe();
}

PreviewDockWidget::~PreviewDockWidget() {
  Configuration().setValue(CompoVisKey::FOLDER_PREVIEW_TYPE.name, (int)GetCurrentPreviewType());
}

void PreviewDockWidget::showEvent(QShowEvent* event) {
  QDockWidget::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

PreviewTypeTool::PREVIEW_TYPE_E PreviewDockWidget::GetCurrentPreviewType() const {
  return mPreviewTypeIntAction.curVal();
}

void PreviewDockWidget::onWindowsTitleChanged(const QString& newWindowsTitle) {
  CHECK_NULLPTR_RETURN_VOID(m_windowsTitleLabel);
  m_windowsTitleLabel->setText(newWindowsTitle);
}

void PreviewDockWidget::setVisible(bool visible) {
  if (QWidget* pWid = widget()) {
    pWid->setVisible(visible);
  }
  QDockWidget::setVisible(visible);
}

void PreviewDockWidget::onPreviewTypeActionToggled(QAction* pPreview) {
  CHECK_NULLPTR_RETURN_VOID(pPreview);
  PreviewTypeTool::PREVIEW_TYPE_E newPreviewType{mPreviewTypeIntAction.act2Enum(pPreview)};
  emit previewTypeChanged(newPreviewType);
}

void PreviewDockWidget::subscribe() {
  connect(mPreviewTypeIntAction.getActionGroup(), &QActionGroup::triggered, this, &PreviewDockWidget::onPreviewTypeActionToggled);
  connect(m_floatingPanel, &QAction::toggled, this, &QDockWidget::setFloating);
  connect(m_minimizePanel, &QAction::triggered, this, &QWidget::showMinimized);
}
