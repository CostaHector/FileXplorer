#include "AdvanceRenamer.h"
#include "MemoryKey.h"
#include "PathTool.h"
#include "FileOsWalker.h"
#include "UndoRedo.h"
#include "PublicMacro.h"
#include "StyleSheet.h"
#include "NotificatorMacro.h"
#include "RenameNamesUnique.h"
#include "CommandsPreview.h"
#include <QDataStream>

constexpr char AdvanceRenamer::NAME_SEP;

AdvanceRenamer::AdvanceRenamer(QWidget* parent)  //
  : QDialog{parent}
{
  m_nameExtIndependent = new (std::nothrow) QCheckBox{"Name Ext Independent", this};
  CHECK_NULLPTR_RETURN_VOID(m_nameExtIndependent)
  m_nameExtIndependent->setToolTip(
      "Show file base name and extension respectively.\n"
      "So rename rules will work/or not work on extension");
  m_recursiveCB = new (std::nothrow) QCheckBox{"Recursive", this};
  CHECK_NULLPTR_RETURN_VOID(m_recursiveCB)
  m_recursiveCB->setToolTip(
      "Recursive rename.\n"
      "Rules will also work on itself and its subdirectories");
  regexValidLabel = new (std::nothrow) StateLabel{"Regex expression state", this};
  CHECK_NULLPTR_RETURN_VOID(regexValidLabel)
  regexValidLabel->setVisible(false);

  m_mainLayout = new (std::nothrow) QVBoxLayout{this};
  CHECK_NULLPTR_RETURN_VOID(m_mainLayout);

  ReadSettings();
  setWindowFlag(Qt::WindowMaximizeButtonHint);  // WindowMinMaxButtonsHint;
}

void AdvanceRenamer::showEvent(QShowEvent* event) {
  StyleSheet::UpdateTitleBar(this);
  QDialog::showEvent(event);
}

#include <QApplication>
#include <QScreen>
bool NeedSaveCurrentGeometry(const QWidget& w) {
  const QScreen* pScreen = w.screen();
  return pScreen != nullptr && pScreen->availableGeometry().contains(w.geometry());
}

void AdvanceRenamer::closeEvent(QCloseEvent* event) {
  if (NeedSaveCurrentGeometry(*this)) {
    Configuration().setValue("ADVANCE_RENAMER_GEOMETRY", saveGeometry());
  }
  QDialog::closeEvent(event);
}

void AdvanceRenamer::ReadSettings() {
  if (Configuration().contains("ADVANCE_RENAMER_GEOMETRY")) {
    restoreGeometry(Configuration().value("ADVANCE_RENAMER_GEOMETRY").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
}

void AdvanceRenamer::initCommonSetting() {
  m_recursiveCB->setChecked(Configuration().value(MemoryKey::RENAMER_INCLUDING_DIR.name, MemoryKey::RENAMER_INCLUDING_DIR.v).toBool());
  const bool bNameExtIndependent{Configuration().value(MemoryKey::RENAMER_NAME_EXT_INDEPENDENT.name, MemoryKey::RENAMER_NAME_EXT_INDEPENDENT.v).toBool()};
  m_nameExtIndependent->setChecked(bNameExtIndependent);
  m_oExtTE->setVisible(bNameExtIndependent);
  m_nExtTE->setVisible(bNameExtIndependent);
}

QHBoxLayout* AdvanceRenamer::GetNameEditsLayout() {
  m_relNameTE = new (std::nothrow) QPlainTextEdit{this};
  CHECK_NULLPTR_RETURN_NULLPTR(m_relNameTE)
  m_oBaseTE = new (std::nothrow) QPlainTextEdit{this};
  CHECK_NULLPTR_RETURN_NULLPTR(m_oBaseTE)
  m_oExtTE = new (std::nothrow) QPlainTextEdit{this};
  CHECK_NULLPTR_RETURN_NULLPTR(m_oExtTE)
  m_nBaseTE = new (std::nothrow) QPlainTextEdit{this};
  CHECK_NULLPTR_RETURN_NULLPTR(m_nBaseTE)
  m_nExtTE = new (std::nothrow) QPlainTextEdit{this};
  CHECK_NULLPTR_RETURN_NULLPTR(m_nExtTE)
  m_relNameTE->setReadOnly(true);
  m_relNameTE->setWordWrapMode(QTextOption::WrapMode::NoWrap);
  m_oBaseTE->setReadOnly(true);
  m_oBaseTE->setWordWrapMode(QTextOption::WrapMode::NoWrap);
  m_oExtTE->setReadOnly(true);
  m_nBaseTE->setReadOnly(false);
  m_nBaseTE->setWordWrapMode(QTextOption::WrapMode::NoWrap);
  m_nExtTE->setReadOnly(false);

  auto* nameEditLayout = new (std::nothrow) QHBoxLayout;
  CHECK_NULLPTR_RETURN_NULLPTR(nameEditLayout);
  nameEditLayout->addWidget(m_relNameTE);
  nameEditLayout->addWidget(m_oBaseTE);
  nameEditLayout->addWidget(m_oExtTE);
  nameEditLayout->addWidget(m_nBaseTE);
  nameEditLayout->addWidget(m_nExtTE);
  nameEditLayout->setStretch(0, 3);
  nameEditLayout->setStretch(1, 8);
  nameEditLayout->setStretch(2, 1);
  nameEditLayout->setStretch(3, 8);
  nameEditLayout->setStretch(4, 1);  
  return nameEditLayout;
}

QDialogButtonBox* AdvanceRenamer::GetDlgButtonBox() {
  auto* buttonBox = new (std::nothrow) QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this};
  CHECK_NULLPTR_RETURN_NULLPTR(buttonBox)
  buttonBox->setOrientation(Qt::Orientation::Horizontal);

  auto* pOkBtn = buttonBox->button(QDialogButtonBox::Ok);
  pOkBtn->setShortcut(QKeySequence(Qt::Key::Key_F10));
  pOkBtn->setToolTip(QString("<b>%1 (%2)</b><br/> Apply changes right now.")  //
                         .arg(pOkBtn->text(), pOkBtn->shortcut().toString()));
  pOkBtn->setStyleSheet(StyleSheet::SUBMIT_BTN_STYLE);

  auto* pHelpBtn = buttonBox->button(QDialogButtonBox::Help);
  pHelpBtn->setIcon(QIcon(":img/COMMAND_PREVIEW"));
  pHelpBtn->setText("See commands...");
  pHelpBtn->setCheckable(true);
  return buttonBox;
}

void AdvanceRenamer::init() {
  InitExtraCommonVariable();
  InitExtraMemberWidget();

  m_controlBar = InitControlTB();
  CHECK_NULLPTR_RETURN_VOID(m_controlBar)
  m_nameEditLayout = GetNameEditsLayout();
  CHECK_NULLPTR_RETURN_VOID(m_nameEditLayout)
  m_buttonBox = GetDlgButtonBox();
  CHECK_NULLPTR_RETURN_VOID(m_buttonBox)

  m_mainLayout->addWidget(m_controlBar);
  m_mainLayout->addLayout(m_nameEditLayout);
  m_mainLayout->addWidget(m_buttonBox);
  m_mainLayout->setSpacing(0);
  m_mainLayout->setContentsMargins(5, 5, 5, 5);

  /* don't move this section up (Don't set state before UI)*/
  initCommonSetting(); // 共用配置
  initExclusiveSetting(); // 独有配置
  /* don't move this section up */

  Subscribe();
  extraSubscribe();
}

void AdvanceRenamer::Subscribe() {
  connect(m_nameExtIndependent, &QCheckBox::stateChanged, this, &AdvanceRenamer::onNameExtRespective);
  connect(m_recursiveCB, &QCheckBox::stateChanged, this, &AdvanceRenamer::onIncludingSub);

  connect(m_nBaseTE->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](const int position) {
    m_relNameTE->verticalScrollBar()->setValue(position);
    m_oBaseTE->verticalScrollBar()->setValue(position);
    m_oExtTE->verticalScrollBar()->setValue(position);
    m_nExtTE->verticalScrollBar()->setValue(position);
  });

  connect(m_buttonBox->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, this, [this]() {
    onApply(false);
    QDialog::accept();
  });
  connect(m_buttonBox->button(QDialogButtonBox::StandardButton::Help), &QPushButton::toggled, this, [this](const bool checked) {
    if (m_commandsPreview == nullptr) {
      m_commandsPreview = new (std::nothrow) CommandsPreview{"COMMANDS_PREVIEW", this};
      CHECK_NULLPTR_RETURN_VOID(m_commandsPreview);
      m_mainLayout->insertWidget(m_mainLayout->count() - 1, m_commandsPreview);
    }
    m_commandsPreview->setVisible(checked);
    if (checked) {
      onApply(true);
    }
  });
  connect(m_buttonBox->button(QDialogButtonBox::StandardButton::Cancel), &QPushButton::clicked, this, &QDialog::close);
}

bool AdvanceRenamer::onApply(const bool isOnlyHelp) {
  const QStringList& relNameList = m_relNameTE->toPlainText().split(NAME_SEP);
  const QStringList& oldCompleteNameList = m_oBaseTE->toPlainText().split(NAME_SEP);
  const QStringList& oldSuffixList = m_oExtTE->toPlainText().split(NAME_SEP);

  const QStringList& newCompleteNameList = m_nBaseTE->toPlainText().split(NAME_SEP);
  const QStringList& newSuffixList = m_nExtTE->toPlainText().split(NAME_SEP);

  RenameNamesUnique renameHelper{mWorkPath, relNameList, oldCompleteNameList, oldSuffixList, newCompleteNameList, newSuffixList, m_recursiveCB->isChecked()};
  renameHelper();
  if (!renameHelper) {
    const QString& rejectMsg = renameHelper.Details();
    LOG_ERR_NP("Cannot do rename commands", rejectMsg);
    return false;
  }
  // rename files first, than its folders;
  using namespace FileOperatorType;
  const BATCH_COMMAND_LIST_TYPE& reversedcmds(renameHelper.getRenameCommands());
  if (isOnlyHelp) {
    QString cmds;
    for (const ACMD& cmd : reversedcmds) {
      cmds += cmd.toStr();
      cmds += '\n';
    }
    m_commandsPreview->setPlainText(cmds);
    return true;
  }
  if (!g_undoRedo.Do(reversedcmds)) {
    LOG_ERR_NP("Batch commands partially failed", "See details in log");
    return false;
  }
  LOG_OK_P("[Ok]Batch commands rename", "Commands count %d", reversedcmds.size());
  return true;
}

void AdvanceRenamer::onRegex(const int regexState) {
  const bool isRegexEnabled{regexState == Qt::Checked};
  Configuration().setValue(MemoryKey::RENAMER_REGEX_ENABLED.name, isRegexEnabled);
  OnlyTriggerRenameCore();
}

void AdvanceRenamer::onIncludingSub(int includingSubState) {
  const bool isIncludingDir = includingSubState == Qt::Checked;
  Configuration().setValue(MemoryKey::RENAMER_INCLUDING_DIR.name, isIncludingDir);
  InitTextEditContent(mWorkPath, mSelectedNames);
}

void AdvanceRenamer::onNameExtRespective(int includingSuffixState) {
  const bool bNameExtIndependent{includingSuffixState == Qt::Checked};
  Configuration().setValue(MemoryKey::RENAMER_NAME_EXT_INDEPENDENT.name, bNameExtIndependent);
  m_oExtTE->setVisible(bNameExtIndependent);
  m_nExtTE->setVisible(bNameExtIndependent);
  if (bNameExtIndependent) {
    for (int i = 0; i < mExts.size(); ++i) {
      std::tie(mNames[i], mExts[i]) = PathTool::GetBaseNameExt(mNames[i]);
    }
  } else {
    for (int i = 0; i < mExts.size(); ++i) {
      mNames[i] += mExts[i];
      mExts[i].clear();
    }
  }
  UpdateNameAndExt();
}

void AdvanceRenamer::UpdateNameAndExt() {
  const QString& suffixs = mExts.join(NAME_SEP);
  m_oBaseTE->setPlainText(mNames.join(NAME_SEP));
  m_oExtTE->setPlainText(suffixs);

  const auto& newCompleteNames = RenameCore(mNames);
  m_nBaseTE->setPlainText(newCompleteNames.join(NAME_SEP));
  m_nExtTE->setPlainText(suffixs);
}

void AdvanceRenamer::InitTextEditContent(const QString& workPath, const QStringList& selectedNames) {
  mWorkPath = workPath;            // will never change
  mSelectedNames = selectedNames;  // will never change

  const bool bSubDir{m_recursiveCB->isChecked()};
  const bool bSuffixInsideFilename{!m_nameExtIndependent->isChecked()};

  FileOsWalker osWalker{mWorkPath, bSuffixInsideFilename};
  osWalker(mSelectedNames, bSubDir);
  FilterNames(osWalker);
  mRelToNameWithNoRoot.swap(osWalker.relToNames);
  mNames.swap(osWalker.completeNames);
  mExts.swap(osWalker.suffixs);
  m_relNameTE->setPlainText(mRelToNameWithNoRoot.join(NAME_SEP));

  UpdateNameAndExt();

  setWindowTitle(windowTitleFormat.arg(mNames.size()).arg(mWorkPath));
}

void AdvanceRenamer::OnlyTriggerRenameCore() {
  // will not call OSWalker; only update complete name;
  setWindowTitle(windowTitleFormat.arg(mNames.size()).arg(mWorkPath));
  const auto& newCompleteNames = RenameCore(mNames);
  m_nBaseTE->setPlainText(newCompleteNames.join(NAME_SEP));
}
