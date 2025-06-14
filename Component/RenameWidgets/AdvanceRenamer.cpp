#include "AdvanceRenamer.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"
#include "public/PathTool.h"
#include "public/FileOsWalker.h"
#include "public/UndoRedo.h"
#include "public/PublicMacro.h"
#include "public/StyleSheet.h"
#include "Component/Notificator.h"
#include "Tools/RenameNamesUnique.h"
#include "CommandsPreview.h"

constexpr char AdvanceRenamer::NAME_SEP;

AdvanceRenamer::AdvanceRenamer(QWidget* parent)  //
    : QDialog{parent},                           //
      windowTitleFormat("%1 | %2")               //
{
  m_nameExtIndependent = new (std::nothrow) QCheckBox{"Name Ext Independent"};
  CHECK_NULLPTR_RETURN_VOID(m_nameExtIndependent)
  m_nameExtIndependent->setToolTip(
      "Show file base name and extension respectively.\n"
      "So rename rules will work/or not work on extension");
  m_recursiveCB = new (std::nothrow) QCheckBox{"Recursive"};
  CHECK_NULLPTR_RETURN_VOID(m_recursiveCB)
  m_recursiveCB->setToolTip(
      "Recursive rename.\n"
      "Rules will also work on itself and its subdirectories");
  regexValidLabel = new (std::nothrow) StateLabel{"Regex expression state"};
  CHECK_NULLPTR_RETURN_VOID(regexValidLabel)
  m_relNameTE = new (std::nothrow) QPlainTextEdit;
  CHECK_NULLPTR_RETURN_VOID(m_relNameTE)
  m_oBaseTE = new (std::nothrow) QPlainTextEdit;
  CHECK_NULLPTR_RETURN_VOID(m_oBaseTE)
  m_oExtTE = new (std::nothrow) QPlainTextEdit;
  CHECK_NULLPTR_RETURN_VOID(m_oExtTE)
  m_nBaseTE = new (std::nothrow) QPlainTextEdit;
  CHECK_NULLPTR_RETURN_VOID(m_nBaseTE)
  m_nExtTE = new (std::nothrow) QPlainTextEdit;
  CHECK_NULLPTR_RETURN_VOID(m_nExtTE)
  m_buttonBox = new (std::nothrow) QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help};
  CHECK_NULLPTR_RETURN_VOID(m_buttonBox)

  m_buttonBox->setOrientation(Qt::Orientation::Horizontal);
  auto* pOkBtn = m_buttonBox->button(QDialogButtonBox::Ok);
  pOkBtn->setShortcut(QKeySequence(Qt::Key::Key_F10));
  pOkBtn->setToolTip(QString("<b>%1 (%2)</b><br/> Apply changes right now.")  //
                         .arg(pOkBtn->text(), pOkBtn->shortcut().toString()));
  pOkBtn->setStyleSheet(StyleSheet::SUBMIT_BTN_STYLE);
  m_buttonBox->button(QDialogButtonBox::Help)->setIcon(QIcon(":img/COMMAND_PREVIEW"));
  m_buttonBox->button(QDialogButtonBox::Help)->setText("See commands...");

  m_mainLayout = new (std::nothrow) QVBoxLayout();
  CHECK_NULLPTR_RETURN_VOID(m_mainLayout);

  m_relNameTE->setReadOnly(true);
  m_relNameTE->setWordWrapMode(QTextOption::WrapMode::NoWrap);
  m_relNameTE->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_relNameTE->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  m_oBaseTE->setReadOnly(true);
  m_oBaseTE->setWordWrapMode(QTextOption::WrapMode::NoWrap);
  m_oBaseTE->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_oBaseTE->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  m_oExtTE->setReadOnly(true);
  m_oExtTE->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_oExtTE->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  m_nBaseTE->setReadOnly(false);
  m_nBaseTE->setWordWrapMode(QTextOption::WrapMode::NoWrap);
  m_nBaseTE->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  m_nBaseTE->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  m_nExtTE->setReadOnly(false);
  m_nExtTE->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_nExtTE->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  m_nameEditLayout = new (std::nothrow) QHBoxLayout;
  CHECK_NULLPTR_RETURN_VOID(m_nameEditLayout);
  m_nameEditLayout->addWidget(m_relNameTE);
  m_nameEditLayout->addWidget(m_oBaseTE);
  m_nameEditLayout->addWidget(m_oExtTE);
  m_nameEditLayout->addWidget(m_nBaseTE);
  m_nameEditLayout->addWidget(m_nExtTE);
  m_nameEditLayout->setStretch(0, 3);
  m_nameEditLayout->setStretch(1, 8);
  m_nameEditLayout->setStretch(2, 1);
  m_nameEditLayout->setStretch(3, 8);
  m_nameEditLayout->setStretch(4, 1);
  m_nameEditLayout->setSpacing(0);
  m_nameEditLayout->setContentsMargins(0, 0, 0, 0);

  ReadSettings();

  // Qt.FramelessWindowHint|Qt.WindowSystemMenuHint;
  setWindowFlag(Qt::WindowMaximizeButtonHint);  // WindowMinMaxButtonsHint;
}

void AdvanceRenamer::showEvent(QShowEvent* event) {
  QDialog::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void AdvanceRenamer::closeEvent(QCloseEvent* event) {
  PreferenceSettings().setValue("ADVANCE_RENAMER_GEOMETRY", saveGeometry());
  QDialog::closeEvent(event);
}

void AdvanceRenamer::ReadSettings() {
  if (PreferenceSettings().contains("ADVANCE_RENAMER_GEOMETRY")) {
    restoreGeometry(PreferenceSettings().value("ADVANCE_RENAMER_GEOMETRY").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
}

void AdvanceRenamer::init() {
  InitExtraCommonVariable();
  InitExtraMemberWidget();
  m_controlBar = InitControlTB();
  m_mainLayout->addWidget(m_controlBar);
  m_mainLayout->addLayout(m_nameEditLayout);
  m_mainLayout->addWidget(m_buttonBox);
  setLayout(m_mainLayout);

  /* don't move this section up (Don't set state before UI)*/
  m_recursiveCB->setChecked(PreferenceSettings().value(MemoryKey::RENAMER_INCLUDING_DIR.name, MemoryKey::RENAMER_INCLUDING_DIR.v).toBool());
  const bool bNameExtIndependent{PreferenceSettings().value(MemoryKey::RENAMER_NAME_EXT_INDEPENDENT.name, MemoryKey::RENAMER_NAME_EXT_INDEPENDENT.v).toBool()};
  m_nameExtIndependent->setChecked(bNameExtIndependent);
  m_oExtTE->setVisible(bNameExtIndependent);
  m_nExtTE->setVisible(bNameExtIndependent);
  /* don't move this section up */

  extraSubscribe();
  Subscribe();
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

  connect(m_buttonBox->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, this, [this]() { onApply(false, true); });
  connect(m_buttonBox->button(QDialogButtonBox::StandardButton::Help), &QPushButton::clicked, this, [this]() { onApply(true, false); });
  connect(m_buttonBox->button(QDialogButtonBox::StandardButton::Cancel), &QPushButton::clicked, this, &AdvanceRenamer::close);
}

bool AdvanceRenamer::onApply(const bool isOnlyHelp, const bool isInterative) {
  const QStringList& relNameList = m_relNameTE->toPlainText().split(NAME_SEP);
  const QStringList& oldCompleteNameList = m_oBaseTE->toPlainText().split(NAME_SEP);
  const QStringList& oldSuffixList = m_oExtTE->toPlainText().split(NAME_SEP);

  const QStringList& newCompleteNameList = m_nBaseTE->toPlainText().split(NAME_SEP);
  const QStringList& newSuffixList = m_nExtTE->toPlainText().split(NAME_SEP);

  RenameNamesUnique renameHelper{mWorkPath, relNameList, oldCompleteNameList, oldSuffixList, newCompleteNameList, newSuffixList, m_recursiveCB->isChecked()};
  renameHelper();
  if (!renameHelper) {
    const QString& rejectMsg = renameHelper.Details();
    LOG_BAD("Cannot do rename commands", rejectMsg);
    return false;
  }
  // rename files first, than its folders;
  using namespace FileOperatorType;
  const BATCH_COMMAND_LIST_TYPE& reversedcmds(renameHelper.getRenameCommands());
  if (isOnlyHelp) {
    if (m_commandsPreview == nullptr) {
      m_commandsPreview = new (std::nothrow) CommandsPreview{};
      CHECK_NULLPTR_RETURN_FALSE(m_commandsPreview);
    }
    m_commandsPreview->clear();
    for (const ACMD& cmd : reversedcmds) {
      m_commandsPreview->appendPlainText(cmd.toStr());
    }
    m_commandsPreview->setWindowTitle(QString("Rename names unique | Total %1 Command(s)").arg(reversedcmds.size()));
    m_commandsPreview->raise();
    m_commandsPreview->show();
    return true;
  }
  bool isAllSuccess = g_undoRedo.Do(reversedcmds);
  if (isInterative) {
    if (isAllSuccess) {
      LOG_GOOD("Batch commands rename ok", QString::number(reversedcmds.size()));
    } else {
      LOG_BAD("Batch commands partially failed", "See details in log");
    }
  }
  close();
  return isAllSuccess;
}

void AdvanceRenamer::onRegex(const int regexState) {
  const bool isRegexEnabled{regexState == Qt::Checked};
  PreferenceSettings().setValue(MemoryKey::RENAMER_REGEX_ENABLED.name, isRegexEnabled);
  OnlyTriggerRenameCore();
}

void AdvanceRenamer::onIncludingSub(int includingSubState) {
  const bool isIncludingDir = includingSubState == Qt::Checked;
  PreferenceSettings().setValue(MemoryKey::RENAMER_INCLUDING_DIR.name, isIncludingDir);
  InitTextEditContent(mWorkPath, mSelectedNames);
}

void AdvanceRenamer::onNameExtRespective(int includingSuffixState) {
  const bool bNameExtIndependent{includingSuffixState == Qt::Checked};
  PreferenceSettings().setValue(MemoryKey::RENAMER_NAME_EXT_INDEPENDENT.name, bNameExtIndependent);
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
  mNames.swap(osWalker.completeNames);
  mExts.swap(osWalker.suffixs);
  m_relNameTE->setPlainText(osWalker.relToNames.join(NAME_SEP));

  UpdateNameAndExt();

  setWindowTitle(windowTitleFormat.arg(mNames.size()).arg(mWorkPath));
}

void AdvanceRenamer::OnlyTriggerRenameCore() {
  // will not call OSWalker; only update complete name;
  setWindowTitle(windowTitleFormat.arg(mNames.size()).arg(mWorkPath));
  const auto& newCompleteNames = RenameCore(mNames);
  m_nBaseTE->setPlainText(newCompleteNames.join(NAME_SEP));
}
