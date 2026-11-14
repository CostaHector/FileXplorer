#include "AccountCardEditor.h"
#include <QFormLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QPlainTextEdit>

AccountCardEditor::AccountCardEditor(QWidget *parent) : QWidget(parent) {
  mTypeEdit    = new QLineEdit(this);
  mNameEdit    = new QLineEdit(this);
  mAccountEdit = new QLineEdit(this);
  mPwdEdit     = new QLineEdit(this);

  mOthersEdit = new QPlainTextEdit(this);
  mOthersEdit->setTabChangesFocus(true);

  mRevertButton = new QPushButton(tr("Revert"), this);
  mStageButton  = new QPushButton(tr("Stage"), this);
  mStageButton->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_S));
  mStageButton->setToolTip(QString{"<b>%1 (%2)</b><br/> Apply changes to this row in the list. Not written to the account file until you save."}
                               .arg(mStageButton->text(), mStageButton->shortcut().toString()));

  auto* form = new QFormLayout;
  form->setContentsMargins(8, 8, 8, 0);
  form->addRow(tr("Type"),     mTypeEdit);
  form->addRow(tr("Name"),     mNameEdit);
  form->addRow(tr("Account"),  mAccountEdit);
  form->addRow(tr("Password"), mPwdEdit);
  form->addRow(tr("Others"),   mOthersEdit);
  form->addRow(mRevertButton,   mStageButton);

  setAttribute(Qt::WA_StyledBackground, true);

  connect(mRevertButton, &QPushButton::clicked, this, &AccountCardEditor::revertCurrentChanges);
  connect(mStageButton, &QPushButton::clicked, this, &AccountCardEditor::stageCurrentChanges);
}
