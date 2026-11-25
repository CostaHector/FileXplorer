#include "CastDBCardEditor.h"
#include "JsonFieldBoundary.h"

#include <QFormLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QPlainTextEdit>

CastDBCardEditor::CastDBCardEditor(QWidget *parent) : QWidget(parent) {
  mActorNameEdit    = new QLineEdit(this);
  mRateEdit         = new QSpinBox(this);
  mRateEdit->setRange(JsonFieldBoundary::MovieRateE::RATE_MIN_UNINITIALIZED_V, JsonFieldBoundary::MovieRateE::RATE_MAX_V);
  mRateEdit->setSingleStep(1);
  mAliasEdit        = new QLineEdit(this);
  mTagsEdit         = new QLineEdit(this);
  mOriBelongToEdit  = new QLineEdit(this);
  mHeightEdit       = new QSpinBox(this);
  mHeightEdit->setRange(-1, 230);
  mHeightEdit->setSingleStep(1);
  mHeightEdit->setSuffix(" cm");
  mSizeEdit         = new QSpinBox(this);
  mSizeEdit->setRange(-1, 50);
  mSizeEdit->setSingleStep(1);
  mSizeEdit->setSuffix(" cm");
  mDetailEdit       = new QPlainTextEdit(this);
  mDetailEdit->setTabChangesFocus(true);

  mRevertButton = new QPushButton(tr("Revert"), this);
  mStageButton  = new QPushButton(tr("Stage"), this);
  mStageButton->setShortcut(QKeySequence(Qt::Key::Key_F10));
  mStageButton->setToolTip(QString{"<b>%1 (%2)</b><br/> Apply changes to this row in the list. Not written to the account file until you save."}
                               .arg(mStageButton->text(), mStageButton->shortcut().toString()));

  auto* form = new QFormLayout{this};
  form->setContentsMargins(8, 8, 8, 0);
  form->addRow(tr("Name"),     mActorNameEdit);
  form->addRow(tr("Rate"),  mRateEdit);
  form->addRow(tr("Alias"),  mAliasEdit);
  form->addRow(tr("Tags"),  mTagsEdit);
  form->addRow(tr("Ori"),     mOriBelongToEdit);
  form->addRow(tr("Height"), mHeightEdit);
  form->addRow(tr("Size"), mSizeEdit);
  form->addRow(tr("Detail"),   mDetailEdit);
  form->addRow(mRevertButton,  mStageButton);

  setAttribute(Qt::WA_StyledBackground, true);

  connect(mRevertButton, &QPushButton::clicked, this, &CastDBCardEditor::revertCurrentChanges);
  connect(mStageButton, &QPushButton::clicked, this, &CastDBCardEditor::stageCurrentChanges);
}