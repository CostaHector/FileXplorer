#include "HarMgr.h"
#include "PublicMacro.h"
#include "StyleSheet.h"
#include "SizeTool.h"
#include "Configuration.h"

HarMgr::HarMgr()
  : QWidget{nullptr} {
  setAttribute(Qt::WA_DeleteOnClose, true);

  m_searchLineEdit = new (std::nothrow) QLineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(m_searchLineEdit);
  m_searchLineEdit->setClearButtonEnabled(true);
  m_searchLineEdit->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  m_searchAction = m_searchLineEdit->addAction(QIcon(":img/FILE_SYSTEM_FILTER"), QLineEdit::LeadingPosition);

  m_harView = new HarTableView{this};
  m_imgPreview = new ByteArrayImageViewer{"HarImagePreview", this};
  m_imgPreview->setMaximumWidth(800);

  m_splitterWid = new QSplitter{this};
  m_splitterWid->addWidget(m_harView);
  m_splitterWid->addWidget(m_imgPreview);

  m_lo = new QVBoxLayout{this};
  m_lo->addWidget(m_searchLineEdit);
  m_lo->addWidget(m_splitterWid);

  setWindowTitle(m_harView->GetWinTitleStr(""));
  setWindowIcon(QIcon{":img/HAR_VIEW"});
  ReadSetting();

  subscribe();
}

void HarMgr::showEvent(QShowEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  QWidget::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void HarMgr::closeEvent(QCloseEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  Configuration().setValue("Geometry/HAR_MGR", saveGeometry());
  QWidget::closeEvent(event);
}

int HarMgr::OpenHar(const QString& harAbsPath) {
  return m_harView->operator()(harAbsPath);
}

void HarMgr::subscribe() {
  connect(m_searchLineEdit, &QLineEdit::returnPressed, this, &HarMgr::onStartFilter);
  connect(m_searchAction, &QAction::triggered, this, &HarMgr::onStartFilter);
  connect(m_harView, &QTableView::windowTitleChanged, this, &HarMgr::setWindowTitle);
  connect(m_harView, &HarTableView::pixmapByteArrayChanged, m_imgPreview, &ByteArrayImageViewer::setPixmapByByteArrayData);
}

void HarMgr::onStartFilter() {
  m_harView->setFilter(m_searchLineEdit->text());
}

void HarMgr::ReadSetting() {
  if (Configuration().contains("Geometry/HAR_MGR")) {
    restoreGeometry(Configuration().value("Geometry/HAR_MGR").toByteArray());
  } else {
    setGeometry(SizeTool::DEFAULT_GEOMETRY);
  }
}
