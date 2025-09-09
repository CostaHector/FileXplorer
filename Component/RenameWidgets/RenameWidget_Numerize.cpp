#include "RenameWidget_Numerize.h"
#include "MemoryKey.h"
#include "PublicMacro.h"
#include "NotificatorMacro.h"
#include "RenameHelper.h"

RenameWidget_Numerize::RenameWidget_Numerize(QWidget* parent)  //
  : AdvanceRenamer{parent}                                   //
{ }

void RenameWidget_Numerize::initExclusiveSetting() {
  m_recursiveCB->setToolTip("Usually we don't suggest user enable numberize file names recursively.");
}

void RenameWidget_Numerize::InitExtraMemberWidget() {
  m_completeBaseName = new (std::nothrow) QLineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(m_completeBaseName)
  m_completeBaseName->setClearButtonEnabled(true);
  m_completeBaseName->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

  m_startNo = new (std::nothrow) QLineEdit{"0", this};  // "0"
  CHECK_NULLPTR_RETURN_VOID(m_startNo)
  m_startNo->setMaximumWidth(20);
  m_isUniqueCounterPerExtension = new (std::nothrow) QCheckBox{"Extension Unique Counter", this};
  m_isUniqueCounterPerExtension->setToolTip("Controls whether file renaming uses a shared counter across extensions:\n"
                                            "✔ Enabled: Files with the same base name share a counter (e.g., 'A 1.jpeg', 'A 1.jpg').\n"
                                            "✖ Disabled: Each extension gets an independent counter (e.g., 'A 1.jpeg', 'A 2.jpg').\n"
                                            "Use case: Preserve version links for multi-format files (e.g., JPEG/WEBP variants).");
  const bool uniqueCnter{Configuration().value(MemoryKey::RENAMER_NUMERIAZER_UNIQUE_EXT_COUNTER.name, MemoryKey::RENAMER_NUMERIAZER_UNIQUE_EXT_COUNTER.v).toBool()};
  m_isUniqueCounterPerExtension->setChecked(uniqueCnter);

  m_numberPattern = new (std::nothrow) QComboBox{this};  // " - %1"
  CHECK_NULLPTR_RETURN_VOID(m_numberPattern)
  m_numberPattern->setEditable(true);
  m_numberPattern->setDuplicatesEnabled(false);
  m_numberPattern->setMaximumWidth(60);
  const QStringList& noFormatCandidate{Configuration().value(MemoryKey::RENAMER_NUMERIAZER_NO_FORMAT.name, MemoryKey::RENAMER_NUMERIAZER_NO_FORMAT.v).toStringList()};
  m_numberPattern->addItems(noFormatCandidate);

  const int noFormatDefaultIndex = Configuration().value(MemoryKey::RENAMER_NUMERIAZER_NO_FORMAT_DEFAULT_INDEX.name, MemoryKey::RENAMER_NUMERIAZER_NO_FORMAT_DEFAULT_INDEX.v).toInt();
  if (noFormatDefaultIndex < 0 && noFormatDefaultIndex >= noFormatCandidate.size()) {
    LOG_W("number[%d] pattern out of bound[%d, %d)", noFormatDefaultIndex, 0, noFormatCandidate.size());
  } else {
    m_numberPattern->setCurrentIndex(noFormatDefaultIndex);
  }
  m_nameExtIndependent->setCheckState(Qt::CheckState::Unchecked);
  m_recursiveCB->setCheckState(Qt::CheckState::Unchecked);
}

void RenameWidget_Numerize::InitExtraCommonVariable() {
  windowTitleFormat = "Numerize name string | %1 item(s) under [%2]";
  setWindowTitle(windowTitleFormat);
#ifndef RUNNING_UNIT_TESTS
  setWindowIcon(QIcon(":img/NAME_STR_NUMERIZER_PATH"));
#endif
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
  numerizeControlTb->addWidget(m_isUniqueCounterPerExtension);
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
      LOG_ERR_P("[Abort] Start number str invalid", "[%s] use %d instead", qPrintable(startNoStr), startNo);
      return;
    }
    OnlyTriggerRenameCore();
  });

  connect(m_isUniqueCounterPerExtension, &QCheckBox::stateChanged, this, [this](int checked)->void{
    Configuration().setValue(MemoryKey::RENAMER_NUMERIAZER_UNIQUE_EXT_COUNTER.name, checked == Qt::Checked);
    OnlyTriggerRenameCore();
  });

  connect(m_numberPattern, &QComboBox::currentTextChanged, this, [this]() -> void {
    int defaultFormateInd = m_numberPattern->currentIndex();
    Configuration().setValue(MemoryKey::RENAMER_NUMERIAZER_NO_FORMAT_DEFAULT_INDEX.name, defaultFormateInd);
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
  const QStringList& suffixs = mExts;
  const QString& baseName = m_completeBaseName->text();
  const QString& namePattern = m_numberPattern->currentText();
  const bool bUniqueExtCounter = m_isUniqueCounterPerExtension->checkState() == Qt::Checked;

  const int startNoInt = m_startNo->text().toInt();
  return RenameHelper::NumerizeReplace(replaceeList, suffixs, baseName, startNoInt, namePattern, bUniqueExtCounter);
}
