#include "AdvanceRenamer.h"
#include "MemoryKey.h"
#include "Configuration.h"
#include "PathTool.h"
#include "FileOsWalker.h"
#include "UndoRedo.h"
#include "PublicMacro.h"
#include "StyleSheet.h"
#include "NotificatorMacro.h"
#include "RenameNamesUnique.h"
#include "CommandsPreview.h"

#include <QDataStream>
#include <QScrollBar>
#include <QPushButton>

constexpr char AdvanceRenamer::NAME_SEP;

int AdvanceRenamer::execCore(AdvanceRenamer* self) {
  CHECK_NULLPTR_RETURN_INT(self, -1);
  return self->exec();
}

AdvanceRenamer::AdvanceRenamer(QWidget* parent) //
  : QDialog{parent} {
  m_nameExtIndependent = new (std::nothrow) QCheckBox{tr("Name Ext Independent"), this};
  CHECK_NULLPTR_RETURN_VOID(m_nameExtIndependent)
  m_nameExtIndependent->setToolTip("Show file base name and extension respectively.\n"
                                   "So rename rules will work/or not work on extension");

  m_recursiveCB = new (std::nothrow) QCheckBox{tr("Recursive"), this};
  CHECK_NULLPTR_RETURN_VOID(m_recursiveCB)
  m_recursiveCB->setToolTip("Recursive rename.\n"
                            "Rules will also work on itself and its subdirectories");

  regexValidLabel = new (std::nothrow) StateLabel{tr("Regex expression state"), this};
  CHECK_NULLPTR_RETURN_VOID(regexValidLabel)
  regexValidLabel->setVisible(false);

  m_mainLayout = new (std::nothrow) QVBoxLayout{this};
  CHECK_NULLPTR_RETURN_VOID(m_mainLayout);

  ReadSettings();
  setWindowFlags(Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
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
    Configuration().setValue("Geometry/ADVANCE_RENAMER", saveGeometry());
  }
  QDialog::closeEvent(event);
}

void AdvanceRenamer::ReadSettings() {
  if (Configuration().contains("Geometry/ADVANCE_RENAMER")) {
    restoreGeometry(Configuration().value("Geometry/ADVANCE_RENAMER").toByteArray());
  } else {
    setGeometry(SizeTool::DEFAULT_GEOMETRY);
  }
}

void AdvanceRenamer::initCommonSetting() {
  m_recursiveCB->setChecked(Configuration().value(RenamerKey::INCLUDING_DIR.name, RenamerKey::INCLUDING_DIR.toVariant()).toBool());
  const bool bNameExtIndependent{Configuration().value(RenamerKey::NAME_EXT_INDEPENDENT.name, RenamerKey::NAME_EXT_INDEPENDENT.toVariant()).toBool()};
  m_nameExtIndependent->setChecked(bNameExtIndependent);
  m_oExtTE->setVisible(bNameExtIndependent);
  m_nExtTE->setVisible(bNameExtIndependent);
}

QHBoxLayout* AdvanceRenamer::GetNameEditsLayout() {
  m_relNameTE = new (std::nothrow) QPlainTextEdit{this};
  CHECK_NULLPTR_RETURN_NULLPTR(m_relNameTE)
  m_relNameTE->setProperty("UseCodeFontFamily", true);
  m_relNameTE->setReadOnly(true);
  m_relNameTE->setWordWrapMode(QTextOption::WrapMode::NoWrap);

  m_oBaseTE = new (std::nothrow) QPlainTextEdit{this};
  CHECK_NULLPTR_RETURN_NULLPTR(m_oBaseTE)
  m_oBaseTE->setProperty("UseCodeFontFamily", true);
  m_oBaseTE->setReadOnly(true);
  m_oBaseTE->setWordWrapMode(QTextOption::WrapMode::NoWrap);

  m_oExtTE = new (std::nothrow) QPlainTextEdit{this};
  CHECK_NULLPTR_RETURN_NULLPTR(m_oExtTE)
  m_oExtTE->setProperty("UseCodeFontFamily", true);
  m_oExtTE->setReadOnly(true);

  m_nBaseTE = new (std::nothrow) MultiCursorEditor{this};
  CHECK_NULLPTR_RETURN_NULLPTR(m_nBaseTE)
  m_nBaseTE->setProperty("UseCodeFontFamily", true);
  m_nBaseTE->setWordWrapMode(QTextOption::WrapMode::NoWrap);

  m_nExtTE = new (std::nothrow) QPlainTextEdit{this};
  CHECK_NULLPTR_RETURN_NULLPTR(m_nExtTE)
  m_nExtTE->setProperty("UseCodeFontFamily", true);
  m_nExtTE->setWordWrapMode(QTextOption::WrapMode::NoWrap);

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

  if (QPushButton* pOkBtn = buttonBox->button(QDialogButtonBox::Ok)) {
    pOkBtn->setIcon(QIcon(":img/SAVED"));
    pOkBtn->setShortcut(QKeySequence(Qt::Key::Key_F10));
    pOkBtn->setToolTip(QString("<b>%1 (%2)</b><br/> Apply changes right now.") //
                           .arg(pOkBtn->text(), pOkBtn->shortcut().toString()));
    StyleSheet::UpdateApplyPushButton(pOkBtn);
  }

  if (QPushButton* pHelpBtn = buttonBox->button(QDialogButtonBox::Help)) {
    pHelpBtn->setText(tr("See commands..."));
    pHelpBtn->setCheckable(true);
    pHelpBtn->setChecked(false);
  }
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
  initCommonSetting();    // 共用配置
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

  connect(m_buttonBox, &QDialogButtonBox::accepted, this, [this]() {
    SetApplyResult(onApply(false));
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
  connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::close);
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
  if (!UndoRedo::GetInst().Do(reversedcmds)) {
    LOG_ERR_NP("Batch commands partially failed", "See details in log");
    return false;
  }
  LOG_OK_P("[Ok]Batch commands rename", "Commands count %d", reversedcmds.size());
  return true;
}

void AdvanceRenamer::onRegex(const int regexState) {
  const bool isRegexEnabled{regexState == Qt::Checked};
  Configuration().setValue(RenamerKey::REGEX_ENABLED.name, isRegexEnabled);
  OnlyTriggerRenameCore();
}

void AdvanceRenamer::onIncludingSub(int includingSubState) {
  const bool isIncludingDir = includingSubState == Qt::Checked;
  Configuration().setValue(RenamerKey::INCLUDING_DIR.name, isIncludingDir);
  InitTextEditContent(mWorkPath, mSelectedNames);
}

void AdvanceRenamer::onNameExtRespective(int bStateIndependent) {
  const bool bNameExtIndependent{bStateIndependent == Qt::Checked};
  Configuration().setValue(RenamerKey::NAME_EXT_INDEPENDENT.name, bNameExtIndependent);
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

void AdvanceRenamer::setNewBaseNames(const QStringList& newBaseNames) {
  m_nBaseTE->setPlainText(newBaseNames.join(NAME_SEP));
}

void AdvanceRenamer::UpdateNameAndExt() {
  const QString& suffixs = mExts.join(NAME_SEP);
  m_oBaseTE->setPlainText(mNames.join(NAME_SEP));
  m_oExtTE->setPlainText(suffixs);

  const auto& newCompleteNames = RenameCore(mNames);
  setNewBaseNames(newCompleteNames);
  m_nExtTE->setPlainText(suffixs);
}

void AdvanceRenamer::InitTextEditContent(const QString& workPath, const QStringList& selectedNames) {
  mWorkPath = workPath;           // will never change
  mSelectedNames = selectedNames; // will never change

  const bool bSubDir{m_recursiveCB->isChecked()};
  const bool bSuffixInsideFilename{!m_nameExtIndependent->isChecked()};

  FileOsWalker osWalker{mWorkPath, bSuffixInsideFilename};
  osWalker(mSelectedNames, bSubDir);
  FilterNames(osWalker);
  mRelToNameWithNoRoot.swap(osWalker.relToNames);
  mNames.swap(osWalker.completeNames);
  mExts.swap(osWalker.suffixs);
  mSelectedFilesFullPath.swap(osWalker.filesFullPath);
  m_relNameTE->setPlainText(mRelToNameWithNoRoot.join(NAME_SEP));

  UpdateNameAndExt();

  setWindowTitle(windowTitleFormat.arg(mNames.size()).arg(mWorkPath));
}

void AdvanceRenamer::OnlyTriggerRenameCore() {
  // will not call OSWalker; only update complete name;
  setWindowTitle(windowTitleFormat.arg(mNames.size()).arg(mWorkPath));
  const auto& newCompleteNames = RenameCore(mNames);
  setNewBaseNames(newCompleteNames);
}

QStringList AdvanceRenamer::MultiLineStr2StrList(const KV& kv) {
  if (kv.dataType() != GeneralDataType::Type::MULTI_LINE_STR) {
    LOG_W("Key[%s] type[%d] not support", kv.name, kv.dataType());
    return {};
  }
  const QString multiLineSeperatedByNewLine = Configuration().value(kv.name, kv.v.data.str).toString();
  const QStringList strList{multiLineSeperatedByNewLine.split(NAME_SEP, Qt::KeepEmptyParts)};
  LOG_W("before[%s], count:%d", kv.v.data.str, strList.size());
  return strList;
}
