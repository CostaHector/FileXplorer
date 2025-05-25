#include "AdvanceRenamer.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"
#include "public/PathTool.h"
#include "public/UndoRedo.h"
#include "public/PublicMacro.h"
#include "Component/Notificator.h"
#include "Tools/RenameNamesUnique.h"

AdvanceRenamer::AdvanceRenamer(QWidget* parent)  //
    : QDialog(parent), windowTitleFormat("%1 | %2") {
  m_extensionInNameCB = new (std::nothrow) QCheckBox{"Extension in name"};
  CHECK_NULLPTR_RETURN_VOID(m_extensionInNameCB)
  m_recursiveCB = new (std::nothrow) QCheckBox{"Recursive"};
  CHECK_NULLPTR_RETURN_VOID(m_recursiveCB)
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
  m_commandsPreview = new (std::nothrow) QPlainTextEdit;
  CHECK_NULLPTR_RETURN_VOID(m_commandsPreview)
  // Qt.FramelessWindowHint|Qt.WindowSystemMenuHint;
  setWindowFlag(Qt::WindowMaximizeButtonHint);  // WindowMinMaxButtonsHint;

  m_extensionInNameCB->setToolTip("Extension in file name.\nRules will also work on suffix");
  m_extensionInNameCB->setChecked(PreferenceSettings().value(MemoryKey::RENAMER_INCLUDING_FILE_EXTENSION.name, MemoryKey::RENAMER_INCLUDING_FILE_EXTENSION.v).toBool());
  m_recursiveCB->setToolTip("Recursive rename.\nRules will also work on itself and its subdirectories");
  m_recursiveCB->setChecked(PreferenceSettings().value(MemoryKey::RENAMER_INCLUDING_DIR.name, MemoryKey::RENAMER_INCLUDING_DIR.v).toBool());

  m_buttonBox->setOrientation(Qt::Orientation::Horizontal);
  QPushButton* pOkBtn = m_buttonBox->button(QDialogButtonBox::Ok);
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

  const bool isNameIncludingExtension = m_extensionInNameCB->checkState() == Qt::Checked;
  m_oExtTE->setVisible(!isNameIncludingExtension);
  m_nExtTE->setVisible(!isNameIncludingExtension);
}

void AdvanceRenamer::Subscribe() {
  connect(m_extensionInNameCB, &QCheckBox::stateChanged, this, &AdvanceRenamer::onIncludeSuffix);
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
  const QStringList& relNameList = m_relNameTE->toPlainText().split('\n');
  const QStringList& oldCompleteNameList = m_oBaseTE->toPlainText().split('\n');
  const QStringList& oldSuffixList = m_oExtTE->toPlainText().split('\n');

  const QStringList& newCompleteNameList = m_nBaseTE->toPlainText().split('\n');
  const QStringList& newSuffixList = m_nExtTE->toPlainText().split('\n');

  RenameNamesUnique renameHelper{m_pre, relNameList, oldCompleteNameList, oldSuffixList, newCompleteNameList, newSuffixList, m_recursiveCB->isChecked()};
  renameHelper();
  if (!renameHelper) {
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
void AdvanceRenamer::onRegex(const int regexState) {
  const bool isRegexEnabled{regexState == Qt::Checked};
  PreferenceSettings().setValue(MemoryKey::RENAMER_REGEX_ENABLED.name, isRegexEnabled);
  OnlyTriggerRenameCore();
}

void AdvanceRenamer::onIncludingSub(int includingSubState) {
  const bool isIncludingDir = includingSubState == Qt::Checked;
  PreferenceSettings().setValue(MemoryKey::RENAMER_INCLUDING_DIR.name, isIncludingDir);
  InitTextContent(m_pre, rels);
}

void AdvanceRenamer::onIncludeSuffix(int includingSuffixState) {
  const bool isNameIncludingExtension = includingSuffixState == Qt::Checked;
  PreferenceSettings().setValue(MemoryKey::RENAMER_INCLUDING_FILE_EXTENSION.name, isNameIncludingExtension);
  m_oExtTE->setVisible(not isNameIncludingExtension);
  m_nExtTE->setVisible(not isNameIncludingExtension);
  InitTextContent(m_pre, rels);
}

void AdvanceRenamer::InitTextContent(const QString& p, const QStringList& r) {
  m_pre = p;
  rels = r;

  const bool bSubDir = m_recursiveCB->isChecked();
  const bool bSuffixInside = m_extensionInNameCB->isChecked();
  FileOsWalker osWalker{m_pre, bSuffixInside};
  osWalker(rels, bSubDir);
  const auto& relToNames = osWalker.relToNames;
  completeNames = osWalker.completeNames;  // may baseName only or baseName+extension, depend on includingSuffixState
  const auto& suffixs = osWalker.suffixs;
  isFiles = osWalker.isFiles;

  m_relNameTE->setPlainText(relToNames.join('\n'));

  m_oBaseTE->setPlainText(completeNames.join('\n'));
  m_oExtTE->setPlainText(suffixs.join('\n'));

  setWindowTitle(windowTitleFormat.arg(completeNames.size()).arg(m_pre));
  const auto& newCompleteNames = RenameCore(completeNames);
  m_nBaseTE->setPlainText(newCompleteNames.join('\n'));
  m_nExtTE->setPlainText(suffixs.join('\n'));
}

void AdvanceRenamer::OnlyTriggerRenameCore() {
  // will not call OSWalker.;
  // only update complete name;
  setWindowTitle(windowTitleFormat.arg(completeNames.size()).arg(m_pre));
  const auto& newCompleteNames = RenameCore(completeNames);
  m_nBaseTE->setPlainText(newCompleteNames.join('\n'));
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
