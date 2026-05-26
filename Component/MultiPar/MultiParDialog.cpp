#include "MultiParDialog.h"
#include "PublicMacro.h"
#include "MultiPar2Actions.h"
#include <QLineEdit>
#include <QAction>
#include <QVBoxLayout>
#include <QPushButton>

MultiParDialog::MultiParDialog(ParVerifyInfomationList&& resultList, QWidget* parent)
  : DialogWithSearchLine{parent} {
  m_multiParTableView = new MultiParView{std::move(resultList), "MultiParTableView", this};
  CHECK_NULLPTR_RETURN_VOID(m_multiParTableView);

  m_verifyCliOutput = new (std::nothrow) QPlainTextEdit{this};
  CHECK_NULLPTR_RETURN_VOID(m_verifyCliOutput);
  m_verifyCliOutput->setProperty("UseCodeFontFamily", true);
  m_verifyCliOutput->setHidden(true);

  m_dlgBtnBox = new (std::nothrow) QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Help, Qt::Orientation::Horizontal, this};
  CHECK_NULLPTR_RETURN_VOID(m_dlgBtnBox);

  auto* lo = new (std::nothrow) QVBoxLayout{this};
  CHECK_NULLPTR_RETURN_VOID(lo)
  lo->addWidget(GetSearchLineEdit());
  lo->addWidget(m_multiParTableView);
  lo->addWidget(m_verifyCliOutput);
  lo->addWidget(m_dlgBtnBox);

  setWindowFlags(Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
  setWindowTitle(MultiPar2Actions::GetInst()._VERIFY_IF_NEED_RECOVERY->text());
  setWindowIcon(MultiPar2Actions::GetInst()._VERIFY_IF_NEED_RECOVERY->icon());

  subscribe();
}

void MultiParDialog::subscribe() {
  connect(m_dlgBtnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

  if (QPushButton* pHelpBtn = m_dlgBtnBox->button(QDialogButtonBox::Help)) {
    pHelpBtn->setText(tr("See CLI output"));
    pHelpBtn->setCheckable(true);
    pHelpBtn->setChecked(false);
    connect(pHelpBtn, &QPushButton::toggled, m_verifyCliOutput, &QPlainTextEdit::setVisible);
  }

  connect(m_multiParTableView, &MultiParView::showCliOutputReq, m_verifyCliOutput, &QPlainTextEdit::setPlainText);
}

void MultiParDialog::onStartFilter(const QString& searchText) {
  m_multiParTableView->setFilter(searchText);
}
