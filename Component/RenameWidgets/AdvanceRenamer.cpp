#include "AdvanceRenamer.h"
#include "PublicVariable.h"
#include "Component/NotificatorFrame.h"
#include "Tools/NameTool.h"
#include "Tools/PathTool.h"
#include "Tools/RenameHelper.h"
#include "Tools/RenameNamesUnique.h"
#include "UndoRedo.h"

const QString AdvanceRenamer::INVALID_CHARS("*?\"<>|");
const QSet<QChar> AdvanceRenamer::INVALID_FILE_NAME_CHAR_SET(INVALID_CHARS.cbegin(), INVALID_CHARS.cend());

auto RenameWidget_Insert::InitExtraMemberWidget() -> void {
  insertStrCB->setEditable(true);
  insertStrCB->setCompleter(nullptr);
  insertStrCB->addItems(PreferenceSettings().value(MemoryKey::RENAMER_OLD_STR_LIST.name, MemoryKey::RENAMER_OLD_STR_LIST.v).toStringList());
  insertStrCB->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  insertStrCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

  insertAtCB->setEditable(true);
  insertAtCB->setCompleter(nullptr);
  insertAtCB->addItems(PreferenceSettings().value(MemoryKey::RENAMER_INSERT_INDEXES_LIST.name, MemoryKey::RENAMER_INSERT_INDEXES_LIST.v).toStringList());
  insertAtCB->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  insertAtCB->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
}

QStringList RenameWidget_Insert::RenameCore(const QStringList& replaceeList) {
  const QString& insertString = insertStrCB->currentText();
  const QString& insertAtStr = insertAtCB->currentText();
  bool isnumeric = false;
  int insertAt = insertAtStr.toInt(&isnumeric);
  if (!isnumeric) {
    qCritical("Insert index[%s] must be a number", qPrintable(insertAtStr));
    return replaceeList;
  }
  return RenameHelper::InsertRename(replaceeList, insertString, insertAt);
}

auto RenameWidget_Replace::InitExtraMemberWidget() -> void {
  oldStrCB->addItems(PreferenceSettings().value(MemoryKey::RENAMER_OLD_STR_LIST.name, MemoryKey::RENAMER_OLD_STR_LIST.v).toStringList());
  oldStrCB->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  oldStrCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  oldStrCB->setEditable(true);
  oldStrCB->setCompleter(nullptr);  // block auto complete

  newStrCB->addItems(PreferenceSettings().value(MemoryKey::RENAMER_NEW_STR_LIST.name, MemoryKey::RENAMER_NEW_STR_LIST.v).toStringList());
  newStrCB->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  newStrCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  newStrCB->setEditable(true);
  newStrCB->setCompleter(nullptr);

  regex->setToolTip("Regular expression enable switch");
}

QStringList RenameWidget_Replace::RenameCore(const QStringList& replaceeList) {
  const QString& oldString = oldStrCB->currentText();
  const QString& newString = newStrCB->currentText();
  const bool regexEnable = regex->isChecked();
  if (regexEnable) {
    QRegularExpression repRegex(oldString);
    if (repRegex.isValid()) {
      regexValidLabel->ToSaved();
    } else {
      regexValidLabel->ToNotSaved();
      qWarning("regular expression invalid[%s]", qPrintable(oldString));
      return {};
    }
  }
  return RenameHelper::ReplaceRename(replaceeList, oldString, newString, regexEnable);
}

void RenameWidget_Numerize::InitExtraMemberWidget() {
  int startIndex = PreferenceSettings().value(MemoryKey::RENAMER_NUMERIAZER_START_INDEX.name, MemoryKey::RENAMER_NUMERIAZER_START_INDEX.v).toInt();
  const QStringList& noFormatCandidate{PreferenceSettings().value(MemoryKey::RENAMER_NUMERIAZER_NO_FORMAT.name, MemoryKey::RENAMER_NUMERIAZER_NO_FORMAT.v).toStringList()};
  int noFormatDefaultIndex = PreferenceSettings().value(MemoryKey::RENAMER_NUMERIAZER_NO_FORMAT_DEFAULT_INDEX.name, MemoryKey::RENAMER_NUMERIAZER_NO_FORMAT_DEFAULT_INDEX.v).toInt();
  m_startNo = new (std::nothrow) QLineEdit(QString::number(startIndex));  // "0"
  if (m_startNo == nullptr) {
    qCritical("m_startNo is nullptr");
    return;
  }
  m_startNo->setMaximumWidth(20);
  m_numberPattern = new (std::nothrow) QComboBox;  // " - %1"
  if (m_numberPattern == nullptr) {
    qCritical("m_numberPattern is nullptr");
    return;
  }
  m_numberPattern->setEditable(true);
  m_numberPattern->setDuplicatesEnabled(false);
  m_numberPattern->setMaximumWidth(60);
  m_numberPattern->addItems(noFormatCandidate);
  if (0 <= noFormatDefaultIndex && noFormatDefaultIndex < noFormatCandidate.size()) {
    m_numberPattern->setCurrentIndex(noFormatDefaultIndex);
  }
}

void RenameWidget_Numerize::InitExtraCommonVariable() {
  windowTitleFormat = "Numerize name string | %1 item(s) under [%2]";
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(":img/NAME_STR_NUMERIZER_PATH"));
}

QToolBar* RenameWidget_Numerize::InitControlTB() {
  QToolBar* replaceControl{new QToolBar};
  replaceControl->addWidget(new QLabel("Base name:"));
  replaceControl->addWidget(m_completeBaseName);
  replaceControl->addSeparator();
  replaceControl->addWidget(new QLabel("Start index:"));
  replaceControl->addWidget(m_startNo);
  replaceControl->addSeparator();
  replaceControl->addWidget(new QLabel("No. format:"));
  replaceControl->addWidget(m_numberPattern);
  replaceControl->addSeparator();
  replaceControl->addWidget(EXT_INSIDE_FILENAME);
  replaceControl->addWidget(ITEMS_INSIDE_SUBDIR);
  return replaceControl;
}
void RenameWidget_Numerize::extraSubscribe() {
  connect(m_startNo, &QLineEdit::textChanged, this, [this](const QString& startNoStr) -> void {
    bool isNumber = false;
    int startNo = startNoStr.toInt(&isNumber);
    if (!isNumber) {
      qWarning("%s is not valid start number", qPrintable(startNoStr));
      return;
    }
    PreferenceSettings().setValue(MemoryKey::RENAMER_NUMERIAZER_START_INDEX.name, startNo);
    OnlyTriggerRenameCore();
  });

  connect(m_numberPattern, &QComboBox::currentTextChanged, this, [this]() -> void {
    int defaultFormateInd = m_numberPattern->currentIndex();
    PreferenceSettings().setValue(MemoryKey::RENAMER_NUMERIAZER_NO_FORMAT_DEFAULT_INDEX.name, defaultFormateInd);
    OnlyTriggerRenameCore();
  });
  connect(m_completeBaseName, &QLineEdit::textChanged, this, &RenameWidget_Numerize::OnlyTriggerRenameCore);
}

QStringList RenameWidget_Numerize::RenameCore(const QStringList& replaceeList) {
  const QString& namePattern = m_numberPattern->currentText();
  QString startNoStr = m_startNo->text();
  bool isnumeric = false;
  const int startInd = startNoStr.toInt(&isnumeric);
  if (!isnumeric) {
    qWarning("start index is not number[%s]", qPrintable(startNoStr));
    return replaceeList;
  }
  if (!m_baseNameInited) {  // init lineedit only at first time. when lineedit editted by user. lineedit should not init
    m_completeBaseName->setText(replaceeList[0]);
    m_completeBaseName->selectAll();
    m_baseNameInited = true;
  }
  const QStringList& suffixs = m_oExtTE->toPlainText().split('\n');
  const QString& baseName = m_completeBaseName->text();
  return RenameHelper::NumerizeReplace(replaceeList, suffixs, baseName, startInd, namePattern);
}

auto RenameWidget_Case::RenameCore(const QStringList& replaceeList) -> QStringList {
  if (replaceeList.isEmpty()) {
    return replaceeList;
  }
  auto* caseAct = caseAG->checkedAction();  // todo checked
  if (caseAct == nullptr) {
    qWarning("No rule enabled");
    return replaceeList;
  }
  return RenameWidget_Case::ChangeCaseRename(replaceeList, caseAct->text());
}

QStringList RenameWidget_Case::ChangeCaseRename(const QStringList& replaceeList, const QString& caseRuleName) {
  if (replaceeList.isEmpty()) {
    return {};
  }
  QStringList replacedList;
  if (g_renameAg()._UPPER_CASE->isChecked()) {
    for (const QString& nm : replaceeList) {
      replacedList.append(nm.toUpper());
    }
  } else if (g_renameAg()._LOWER_CASE->isChecked()) {
    for (const QString& nm : replaceeList) {
      replacedList.append(nm.toLower());
    }
  } else if (g_renameAg()._LOOSE_CAPITALIZE->isChecked()) {  // henry cavill -> Henry cavill and HENRY CAVILL -> HENRY CAVILL
    for (const QString& nm : replaceeList) {
      replacedList.append(NameTool::CapitaliseEachWordFirstLetterOnly(nm));
    }
  } else if (g_renameAg()._STRICT_CAPITALIZE->isChecked()) {  // henry cavill -> Henry cavill and HENRY CAVILL -> Henry cavill
    for (const QString& nm : replaceeList) {
      replacedList.append(NameTool::CapitaliseEachWordFirstLetterLowercaseOthers(nm));
    }
  } else if (g_renameAg()._SWAP_CASE->isChecked()) {
    for (const QString& nm : replaceeList) {
      replacedList.append(NameTool::ToggleSentenceCase(nm));
    }
  } else {
    qDebug("Case rule[%s] not supported", qPrintable(caseRuleName));
    return {};
  }
  return replacedList;
}

AdvanceRenamer::AdvanceRenamer(QWidget* parent)
    : QDialog(parent),
      windowTitleFormat("%1 | %2"),
      EXT_INSIDE_FILENAME(new QCheckBox("Also Extensions")),
      ITEMS_INSIDE_SUBDIR(new QCheckBox("Also subdirectory")),
      regexValidLabel(new StateLabel("Regex expression state")),
      m_relNameTE(new QPlainTextEdit()),
      m_oBaseTE(new QPlainTextEdit()),
      m_oExtTE(new QPlainTextEdit()),
      m_nBaseTE(new QPlainTextEdit()),
      m_nExtTE(new QPlainTextEdit()),
      m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help)),
      m_commandsPreview(new QPlainTextEdit) {
  // Qt.FramelessWindowHint|Qt.WindowSystemMenuHint;
  setWindowFlag(Qt::WindowMaximizeButtonHint);  // WindowMinMaxButtonsHint;

  EXT_INSIDE_FILENAME->setToolTip("Extension inside file name.\nRules will also work on suffix");
  ITEMS_INSIDE_SUBDIR->setToolTip("Items contains subdirectory.\nRules will also work on itself and its subdirectories");

  EXT_INSIDE_FILENAME->setChecked(PreferenceSettings().value(MemoryKey::RENAMER_INCLUDING_FILE_EXTENSION.name, MemoryKey::RENAMER_INCLUDING_FILE_EXTENSION.v).toBool());
  ITEMS_INSIDE_SUBDIR->setChecked(PreferenceSettings().value(MemoryKey::RENAMER_INCLUDING_DIR.name, MemoryKey::RENAMER_INCLUDING_DIR.v).toBool());

  m_buttonBox->setOrientation(Qt::Orientation::Horizontal);
  auto* pOkBtn = m_buttonBox->button(QDialogButtonBox::Ok);
  pOkBtn->setShortcut(QKeySequence(Qt::Key::Key_F10));
  pOkBtn->setToolTip(QString("<b>%1 (%2)</b><br/> Apply changes right now.").arg(pOkBtn->text(), pOkBtn->shortcut().toString()));
  pOkBtn->setStyleSheet(SUBMIT_BTN_STYLE);
  m_buttonBox->button(QDialogButtonBox::Help)->setText("See commands...");

  ReadSettings();
}

void AdvanceRenamer::closeEvent(QCloseEvent* event) {
  PreferenceSettings().setValue("ADVANCE_RENAMER_GEOMETRY", saveGeometry());
  return QDialog::closeEvent(event);
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

  m_replaceControlBar = InitControlTB();

  m_relNameTE->setWordWrapMode(QTextOption::WrapMode::NoWrap);
  m_relNameTE->setReadOnly(true);
  m_relNameTE->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_relNameTE->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  m_oBaseTE->setWordWrapMode(QTextOption::WrapMode::NoWrap);
  m_oBaseTE->setReadOnly(true);
  m_oBaseTE->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_oBaseTE->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  m_oExtTE->setReadOnly(true);
  m_oExtTE->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_oExtTE->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  m_nBaseTE->setWordWrapMode(QTextOption::WrapMode::NoWrap);
  m_nBaseTE->setReadOnly(false);
  m_nBaseTE->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  m_nBaseTE->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  m_nExtTE->setReadOnly(false);
  m_nExtTE->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_nExtTE->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  m_nameEditLayout = new QHBoxLayout;
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

  m_mainLayout = new QVBoxLayout();
  m_mainLayout->addWidget(m_replaceControlBar);
  m_mainLayout->addLayout(m_nameEditLayout);
  m_mainLayout->addWidget(m_buttonBox);

  setLayout(m_mainLayout);
  Subscribe();
  extraSubscribe();

  const bool isNameIncludingExtension = EXT_INSIDE_FILENAME->checkState() == Qt::Checked;
  m_oExtTE->setVisible(!isNameIncludingExtension);
  m_nExtTE->setVisible(!isNameIncludingExtension);
}

void AdvanceRenamer::Subscribe() {
  connect(EXT_INSIDE_FILENAME, &QCheckBox::stateChanged, this, &AdvanceRenamer::onIncludeSuffix);
  connect(ITEMS_INSIDE_SUBDIR, &QCheckBox::stateChanged, this, &AdvanceRenamer::onIncludingSub);

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

auto AdvanceRenamer::onApply(const bool isOnlyHelp, const bool isInterative) -> bool {
  const QStringList& relNameList = m_relNameTE->toPlainText().split('\n');
  const QStringList& oldCompleteNameList = m_oBaseTE->toPlainText().split('\n');
  const QStringList& oldSuffixList = m_oExtTE->toPlainText().split('\n');

  const QStringList& newCompleteNameList = m_nBaseTE->toPlainText().split('\n');
  const QStringList& newSuffixList = m_nExtTE->toPlainText().split('\n');

  RenameNamesUnique renameHelper(m_pre, relNameList, oldCompleteNameList, oldSuffixList, newCompleteNameList, newSuffixList, ITEMS_INSIDE_SUBDIR->isChecked());
  renameHelper();
  if (not renameHelper) {
    const QString& rejectMsg = renameHelper.Details();
    qWarning("Cannot do rename commands[%s]", qPrintable(rejectMsg));
    Notificator::badNews("Cannot do rename commands", rejectMsg);
    return false;
  }
  const auto& reversedcmds(renameHelper.getRenameCommands());  // rename files first, than its folders;

  if (isOnlyHelp) {
    for (const auto& cmd : reversedcmds) {
      m_commandsPreview->appendPlainText(cmd.toStr());
    }
    m_commandsPreview->setWindowTitle(QString("Rename names unique | Total %1 Command(s)").arg(reversedcmds.size()));
    m_commandsPreview->setMinimumWidth(1024);
    m_commandsPreview->raise();
    m_commandsPreview->show();
    return true;
  }
  bool isAllSuccess = g_undoRedo.Do(reversedcmds);
  if (isInterative) {
    if (isAllSuccess) {
      qInfo("Batch rename ok %d command(s).", reversedcmds.size());
      Notificator::goodNews("Batch rename ok", QString("%1 command(s).").arg(reversedcmds.size()));
    }
  }
  close();
  return isAllSuccess;
}

auto AdvanceRenamer::onIncludingSub(int includingSubState) -> void {
  const bool isIncludingDir = includingSubState == Qt::Checked;
  PreferenceSettings().setValue(MemoryKey::RENAMER_INCLUDING_DIR.name, isIncludingDir);
  InitTextContent(m_pre, rels);
}

auto AdvanceRenamer::onIncludeSuffix(int includingSuffixState) -> void {
  const bool isNameIncludingExtension = includingSuffixState == Qt::Checked;
  PreferenceSettings().setValue(MemoryKey::RENAMER_INCLUDING_FILE_EXTENSION.name, isNameIncludingExtension);
  m_oExtTE->setVisible(not isNameIncludingExtension);
  m_nExtTE->setVisible(not isNameIncludingExtension);
  InitTextContent(m_pre, rels);
}

void AdvanceRenamer::InitTextContent(const QString& p, const QStringList& r) {
  m_pre = p;
  rels = r;
  OSWalker_RETURN osWalkerRet = PATHTOOL::OSWalker(m_pre, rels, ITEMS_INSIDE_SUBDIR->isChecked(), EXT_INSIDE_FILENAME->isChecked());
  const auto& relToNames = osWalkerRet.relToNames;
  completeNames = osWalkerRet.completeNames;  // may baseName only or baseName+extension, depend on includingSuffixState
  const auto& suffixs = osWalkerRet.suffixs;
  isFiles = osWalkerRet.isFiles;

  m_relNameTE->setPlainText(relToNames.join('\n'));

  m_oBaseTE->setPlainText(completeNames.join('\n'));
  m_oExtTE->setPlainText(suffixs.join('\n'));

  setWindowTitle(windowTitleFormat.arg(completeNames.size()).arg(m_pre));
  const auto& newCompleteNames = RenameCore(completeNames);
  m_nBaseTE->setPlainText(newCompleteNames.join('\n'));
  m_nExtTE->setPlainText(suffixs.join('\n'));
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QDir dir(QFileInfo(QFileInfo(__FILE__).absolutePath()).absolutePath());
  QStringList rels = dir.entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::DirsFirst);

  QApplication a(argc, argv);
  RenameWidget_Insert winI;
  RenameWidget_Replace winR;
  RenameWidget_Delete winD;
  RenameWidget_Numerize winN;
  RenameWidget_Case winC;

  winI.init();
  winR.init();
  winD.init();
  winN.init();
  winC.init();

  winI.InitTextContent(dir.absolutePath(), rels);
  winR.InitTextContent(dir.absolutePath(), rels);
  winD.InitTextContent(dir.absolutePath(), rels);
  winN.InitTextContent(dir.absolutePath(), rels);
  winC.InitTextContent(dir.absolutePath(), rels);
  // winR.InitTextContent(pre, rels)
  // winD.InitTextContent(pre, rels)
  // winN.InitTextContent(pre, rels)
  winI.show();
  winR.show();
  winD.show();
  winN.show();
  winC.show();
  return a.exec();
}
#endif

#include "Tools/ToConsecutiveFileNameNo.h"

QStringList RenameWidget_ConsecutiveFileNo::RenameCore(const QStringList& replaceeList) {
  if (replaceeList.isEmpty()) {
    return replaceeList;
  }
  const QString& startNoStr = m_fileNoStartIndex->text();
  bool isnumeric = false;
  int startNo = startNoStr.toInt(&isnumeric);
  if (not isnumeric) {
    qCritical("start no[%s] must be a number", qPrintable(startNoStr));
    Notificator::critical("start no[%s] must be a number", startNoStr);
    return replaceeList;
  }
  return ToConsecutiveFileNameNo(startNo)(replaceeList);
}
