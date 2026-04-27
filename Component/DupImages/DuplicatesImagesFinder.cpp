#include "DuplicatesImagesFinder.h"
#include "SpacerWidget.h"
#include "MenuToolButton.h"

#include "FileOpActs.h"
#include "RedundantImageFinderActions.h"
#include "SizeTool.h"
#include "Configuration.h"
#include "StyleSheet.h"
#include "UndoRedo.h"
#include "PublicMacro.h"

constexpr char GEOMETRY_KEY[]{"RedunImgFinderKey/GEOMETRY"};

DuplicatesImagesFinder::DuplicatesImagesFinder(QWidget* parent) //
  : QDialog{parent}                                             //
{
  m_toolBar = new (std::nothrow) QToolBar{"Redundent toolbar", this};
  CHECK_NULLPTR_RETURN_VOID(m_toolBar);
  g_redunImgFinderAg().AddRelatedAction(m_toolBar);
  m_toolBar->addWidget(SpacerWidget::GetSpacerWidget(this, Qt::Orientation::Horizontal));
  m_toolBar->addActions(FileOpActs::GetInst().UNDO_REDO_RIBBONS->actions());
  m_toolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);

  m_table = new (std::nothrow) DuplicateImagesTable{this};
  CHECK_NULLPTR_RETURN_VOID(m_table);

  m_lo = new QVBoxLayout{this};
  m_lo->addWidget(m_toolBar);
  m_lo->addWidget(m_table);

  subscribe();

  ReadSetting();
  setWindowIcon(QIcon{":img/DUPLICATE_IMAGES_FINDER"});
}

bool DuplicatesImagesFinder::operator()(const QString& folderPath) {
  return m_table->operator()(folderPath);
}

void DuplicatesImagesFinder::ReadSetting() {
  if (Configuration().contains(GEOMETRY_KEY)) {
    restoreGeometry(Configuration().value(GEOMETRY_KEY).toByteArray());
  } else {
    setGeometry(SizeTool::DEFAULT_GEOMETRY);
  }
}

void DuplicatesImagesFinder::showEvent(QShowEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  QDialog::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void DuplicatesImagesFinder::closeEvent(QCloseEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  FileOpActs::GetInst()._DUPLICATE_IMAGES_FINDER->setChecked(false);
  Configuration().setValue(GEOMETRY_KEY, saveGeometry());
  QDialog::closeEvent(event);
}

void DuplicatesImagesFinder::subscribe() {
  connect(m_table, &QWidget::windowTitleChanged, this, &QWidget::setWindowTitle);
}
