#include "AccountDetailView.h"
#include "PublicVariable.h"
#include "PublicMacro.h"
#include "StyleSheet.h"

AccountDetailView::AccountDetailView(const QString &title, QWidget *parent) //
  : QDockWidget{parent} {
  setObjectName(title);
  mFormWid = new (std::nothrow) QWidget{this};
  CHECK_NULLPTR_RETURN_VOID(mFormWid);
  etType = new (std::nothrow) QLineEdit{mFormWid};
  CHECK_NULLPTR_RETURN_VOID(etType);
  etName = new (std::nothrow) QLineEdit{mFormWid};
  CHECK_NULLPTR_RETURN_VOID(etName);
  etAccount = new (std::nothrow) QLineEdit{mFormWid};
  CHECK_NULLPTR_RETURN_VOID(etAccount);
  etPwd = new (std::nothrow) QLineEdit{mFormWid};
  CHECK_NULLPTR_RETURN_VOID(etPwd);
  etOthers = new (std::nothrow) QTextEdit{mFormWid};
  CHECK_NULLPTR_RETURN_VOID(etOthers);

  mBtnRecover = new (std::nothrow) QPushButton{tr("Recover"), nullptr};
  CHECK_NULLPTR_RETURN_VOID(mBtnRecover);
  mBtnApply = new (std::nothrow) QPushButton{QIcon(":img/SAVED"), tr("Apply"), nullptr};
  CHECK_NULLPTR_RETURN_VOID(mBtnApply);
  mBtnApply->setShortcut(QKeySequence(Qt::Key_F10));
  mBtnApply->setToolTip(QString("<b>%1 (%2)</b><br/> Apply changes right now.") //
                            .arg(mBtnApply->text(), mBtnApply->shortcut().toString()));
  StyleSheet::UpdateApplyPushButton(mBtnApply);

  mForm = new (std::nothrow) QFormLayout;
  CHECK_NULLPTR_RETURN_VOID(mForm);
  mForm->addRow(tr("Type"), etType);
  mForm->addRow(tr("Name"), etName);
  mForm->addRow(tr("Account"), etAccount);
  mForm->addRow(tr("Pwd"), etPwd);
  mForm->addRow(tr("Others"), etOthers);
  mForm->addRow(mBtnRecover, mBtnApply);

  mFormWid->setLayout(mForm);
  setWidget(mFormWid);
  Subscribe();
  editNotHappen();
  setWindowTitle(tr("Detail View"));
}

void AccountDetailView::Subscribe() {
  connect(mBtnRecover, &QPushButton::clicked, this, &AccountDetailView::onRecoverModify);
  connect(mBtnApply, &QPushButton::clicked, this, &AccountDetailView::onApplyModify);
  connect(etType, &QLineEdit::textEdited, this, &AccountDetailView::editHappen);
  connect(etName, &QLineEdit::textEdited, this, &AccountDetailView::editHappen);
  connect(etAccount, &QLineEdit::textEdited, this, &AccountDetailView::editHappen);
  connect(etPwd, &QLineEdit::textEdited, this, &AccountDetailView::editHappen);
  connect(etOthers, &QTextEdit::textChanged, this, &AccountDetailView::editHappen);
}

void AccountDetailView::editHappen() {
  if (pAccount == nullptr) {
    return; // cannot edit at all
  }
  mBtnApply->setEnabled(true);
  mBtnRecover->setEnabled(true);
}

void AccountDetailView::editNotHappen() {
  mBtnApply->setEnabled(false);
  mBtnRecover->setEnabled(false);
}

void AccountDetailView::UpdateDisplay(AccountInfo *pAcc) {
  pAccount = pAcc;
  if (pAcc == nullptr) {
    etType->clear();
    etName->clear();
    etAccount->clear();
    etPwd->clear();
    etOthers->clear();
    editNotHappen();
    return;
  }
  etType->setText(pAccount->typeStr);
  etName->setText(pAccount->nameStr);
  etAccount->setText(pAccount->accountStr);
  etPwd->setText(pAccount->pwdStr);
  etOthers->setPlainText(pAccount->othersStr);
  editNotHappen();
}

void AccountDetailView::onRecoverModify() {
  CHECK_NULLPTR_RETURN_VOID(pAccount);
  etType->setText(pAccount->typeStr);
  etName->setText(pAccount->nameStr);
  etAccount->setText(pAccount->accountStr);
  etPwd->setText(pAccount->pwdStr);
  etOthers->setPlainText(pAccount->othersStr);
  editNotHappen();
}
void AccountDetailView::onApplyModify() {
  CHECK_NULLPTR_RETURN_VOID(pAccount);
  bool detailModified{false};
  if (pAccount->typeStr != etType->text()) {
    pAccount->typeStr = etType->text();
    detailModified = true;
  }
  if (pAccount->nameStr != etName->text()) {
    pAccount->nameStr = etName->text();
    detailModified = true;
  }
  if (pAccount->accountStr != etAccount->text()) {
    pAccount->accountStr = etAccount->text();
    detailModified = true;
  }
  if (pAccount->pwdStr != etPwd->text()) {
    pAccount->pwdStr = etPwd->text();
    detailModified = true;
  }
  if (pAccount->othersStr != etOthers->toPlainText()) {
    pAccount->othersStr = etOthers->toPlainText();
    detailModified = true;
  }
  editNotHappen();
  if (detailModified) {
    pAccount->SetDetailModified();
  }
}
