﻿#include "RenameWidget_Numerize.h"
#include "public/MemoryKey.h"
#include "public/PublicMacro.h"
#include "Tools/RenameHelper.h"

RenameWidget_Numerize::RenameWidget_Numerize(QWidget* parent)  //
    : AdvanceRenamer{parent}                                   //
{                                                              //
  m_recursiveCB->setToolTip("Usually we don't suggest user enable numberize file names recursively.");
}

void RenameWidget_Numerize::InitExtraMemberWidget() {
  m_completeBaseName = new (std::nothrow) QLineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(m_completeBaseName)
  m_completeBaseName->setClearButtonEnabled(true);
  m_completeBaseName->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

  m_startNo = new (std::nothrow) QLineEdit{QString::number(mStartNoInt), this};  // "0"
  CHECK_NULLPTR_RETURN_VOID(m_startNo)
  m_startNo->setMaximumWidth(20);

  m_numberPattern = new (std::nothrow) QComboBox{this};  // " - %1"
  CHECK_NULLPTR_RETURN_VOID(m_numberPattern)
  m_numberPattern->setEditable(true);
  m_numberPattern->setDuplicatesEnabled(false);
  m_numberPattern->setMaximumWidth(60);
  const QStringList& noFormatCandidate{PreferenceSettings().value(MemoryKey::RENAMER_NUMERIAZER_NO_FORMAT.name, MemoryKey::RENAMER_NUMERIAZER_NO_FORMAT.v).toStringList()};
  m_numberPattern->addItems(noFormatCandidate);

  const int noFormatDefaultIndex = PreferenceSettings().value(MemoryKey::RENAMER_NUMERIAZER_NO_FORMAT_DEFAULT_INDEX.name, MemoryKey::RENAMER_NUMERIAZER_NO_FORMAT_DEFAULT_INDEX.v).toInt();
  if (noFormatDefaultIndex < 0 && noFormatDefaultIndex >= noFormatCandidate.size()) {
    qWarning("number[%d] pattern out of bound[%d, %d)", noFormatDefaultIndex, 0, noFormatCandidate.size());
  } else {
    m_numberPattern->setCurrentIndex(noFormatDefaultIndex);
  }
  m_nameExtIndependent->setCheckState(Qt::CheckState::Unchecked);
  m_recursiveCB->setCheckState(Qt::CheckState::Unchecked);
}

void RenameWidget_Numerize::InitExtraCommonVariable() {
  windowTitleFormat = "Numerize name string | %1 item(s) under [%2]";
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(":img/NAME_STR_NUMERIZER_PATH"));
}

QToolBar* RenameWidget_Numerize::InitControlTB() {
  QToolBar* numerizeControlTb{new (std::nothrow) QToolBar{this}};
  CHECK_NULLPTR_RETURN_NULLPTR(numerizeControlTb);

  numerizeControlTb->addWidget(new (std::nothrow) QLabel{"Base name:", numerizeControlTb});
  numerizeControlTb->addWidget(m_completeBaseName);
  numerizeControlTb->addSeparator();
  numerizeControlTb->addWidget(new (std::nothrow) QLabel{"Start index:", numerizeControlTb});
  numerizeControlTb->addWidget(m_startNo);
  numerizeControlTb->addSeparator();
  numerizeControlTb->addWidget(new (std::nothrow) QLabel{"No. format:", numerizeControlTb});
  numerizeControlTb->addWidget(m_numberPattern);
  numerizeControlTb->addSeparator();
  numerizeControlTb->addWidget(m_nameExtIndependent);
  numerizeControlTb->addWidget(m_recursiveCB);
  return numerizeControlTb;
}
void RenameWidget_Numerize::extraSubscribe() {
  connect(m_startNo, &QLineEdit::textChanged, this, [this](const QString& startNoStr) -> void {
    bool isNumber = false;
    int startNo = startNoStr.toInt(&isNumber);
    if (!isNumber) {
      qWarning("%s is not valid start number", qPrintable(startNoStr));
      return;
    }
    mStartNoInt = startNo;
    OnlyTriggerRenameCore();
  });

  connect(m_numberPattern, &QComboBox::currentTextChanged, this, [this]() -> void {
    int defaultFormateInd = m_numberPattern->currentIndex();
    PreferenceSettings().setValue(MemoryKey::RENAMER_NUMERIAZER_NO_FORMAT_DEFAULT_INDEX.name, defaultFormateInd);
    OnlyTriggerRenameCore();
  });
}

QStringList RenameWidget_Numerize::RenameCore(const QStringList& replaceeList) {
  if (!m_baseNameInited) {
    // init lineedit only at first time. when lineedit editted by user. lineedit should not init
    m_baseNameInited = true;
    if (!replaceeList.isEmpty()) {
      m_completeBaseName->setText(replaceeList[0]);
      m_completeBaseName->selectAll();
    }
    connect(m_completeBaseName, &QLineEdit::textChanged, this, &RenameWidget_Numerize::OnlyTriggerRenameCore);
  }
  const QStringList& suffixs = m_oExtTE->toPlainText().split(NAME_SEP);
  const QString& baseName = m_completeBaseName->text();
  const QString& namePattern = m_numberPattern->currentText();
  return RenameHelper::NumerizeReplace(replaceeList, suffixs, baseName, mStartNoInt, namePattern);
}
