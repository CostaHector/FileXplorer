#include "AccountCardEditor.h"
#include <QFormLayout>
#include <QPushButton>

AccountCardEditor::AccountCardEditor(QWidget *parent) : QWidget(parent) {
  mTypeEdit    = new QLineEdit(this);
  mNameEdit    = new QLineEdit(this);
  mAccountEdit = new QLineEdit(this);
  mPwdEdit     = new QLineEdit(this);

  mOthersEdit = new QTextEdit(this);
  mOthersEdit->setAcceptRichText(false);
  mOthersEdit->setTabChangesFocus(true);

  mStageButton   = new QPushButton(tr("Stage"), this);
  mStageButton->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_S));
  mStageButton->setToolTip(
      QString{"<b>%1 (%2)</b><br/> Apply changes to this row in the list. Not written to the account file until you save."}
          .arg(mStageButton->text(), mStageButton->shortcut().toString()));
  mCancelButton = new QPushButton(tr("Cancel"), this);

  auto* form = new QFormLayout;
  form->setContentsMargins(8, 8, 8, 0);
  form->addRow(tr("Type"),     mTypeEdit);
  form->addRow(tr("Name"),     mNameEdit);
  form->addRow(tr("Account"),  mAccountEdit);
  form->addRow(tr("Password"), mPwdEdit);
  form->addRow(tr("Others"),   mOthersEdit);

  auto* buttonRow = new QHBoxLayout;
  buttonRow->setContentsMargins(8, 0, 8, 8);
  buttonRow->addStretch();
  buttonRow->addWidget(mStageButton);
  buttonRow->addWidget(mCancelButton);

  auto* root = new QVBoxLayout(this);
  root->setContentsMargins(0, 0, 0, 0);
  root->addLayout(form);
  root->addLayout(buttonRow);

  setAttribute(Qt::WA_StyledBackground, true);
}
