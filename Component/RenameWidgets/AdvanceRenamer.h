#ifndef ADVANCERENAMER_H
#define ADVANCERENAMER_H

#include <PublicTool.h>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QSet>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

#include "Component/StateLabel.h"

class AdvanceRenamer : public QDialog {
 public:
  explicit AdvanceRenamer(QWidget* parent = nullptr);

  virtual void closeEvent(QCloseEvent* event) override;
  void ReadSettings();

  void init();

  void Subscribe() {
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
  auto onApply(const bool isOnlyHelp = false, const bool isInterative = false) -> bool;
  auto onRegex(const int regexState) -> void {
    // regexState;
    OnlyTriggerRenameCore();
  }
  auto onIncludingSub(int includingSubState) -> void;
  auto onIncludeSuffix(int includingSuffixState) -> void;

  auto InitTextContent(const QString& p, const QStringList& r) -> void {
    m_pre = p;
    rels = r;

    const Qt::CheckState includingSubState = ITEMS_INSIDE_SUBDIR->checkState();
    const Qt::CheckState includingSuffixState = EXT_INSIDE_FILENAME->checkState();

    OSWalker_RETURN osWalkerRet = OSWalker(m_pre, rels, includingSubState == Qt::Checked, includingSuffixState == Qt::Checked);
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
  auto OnlyTriggerRenameCore() -> void {
    // will not call OSWalker.;
    // only update complete name;
    setWindowTitle(windowTitleFormat.arg(completeNames.size()).arg(m_pre));
    const auto& newCompleteNames = RenameCore(completeNames);
    m_nBaseTE->setPlainText(newCompleteNames.join('\n'));
  }

  virtual void InitExtraCommonVariable() = 0;
  virtual void InitExtraMemberWidget() = 0;
  virtual auto InitControlTB() -> QToolBar* = 0;
  virtual void extraSubscribe() = 0;
  virtual auto RenameCore(const QStringList& replaceeList) -> QStringList = 0;

  static inline auto IsFileNameInvalid(const QString& filename) -> bool {
    QSet<QChar> nameSet(filename.cbegin(), filename.cend());
    return nameSet.intersects(AdvanceRenamer::INVALID_FILE_NAME_CHAR_SET);
  }

 public:
  static const QString INVALID_CHARS;
  static const QSet<QChar> INVALID_FILE_NAME_CHAR_SET;
  QString windowTitleFormat;

  QString m_pre;
  QStringList rels;
  QStringList completeNames;
  QList<bool> isFiles;

  QCheckBox* EXT_INSIDE_FILENAME;
  QCheckBox* ITEMS_INSIDE_SUBDIR;
  StateLabel* regexValidLabel;

  QPlainTextEdit* m_relNameTE;
  QPlainTextEdit* m_oBaseTE;
  QPlainTextEdit* m_oExtTE;  // absolutefilepath = m_pre + m_relNameTE + m_oBaseTE + m_oExtTE
  QPlainTextEdit* m_nBaseTE;
  QPlainTextEdit* m_nExtTE;
  QDialogButtonBox* m_buttonBox;

  QToolBar* m_replaceControlBar{nullptr};
  QHBoxLayout* m_nameEditLayout{nullptr};
  QVBoxLayout* m_mainLayout{nullptr};

  QPlainTextEdit* m_commandsPreview;
};

class RenameWidget_ConsecutiveFileNo : public AdvanceRenamer {
 public:
  QLineEdit* m_fileNoStartIndex{new QLineEdit{"0", this}};

  RenameWidget_ConsecutiveFileNo(QWidget* parent = nullptr) : AdvanceRenamer(parent) {
    EXT_INSIDE_FILENAME->setCheckState(Qt::CheckState::Checked);
    ITEMS_INSIDE_SUBDIR->setEnabled(false);
    ITEMS_INSIDE_SUBDIR->setCheckState(Qt::CheckState::Unchecked);
  };

  auto InitExtraCommonVariable() -> void override {
    windowTitleFormat = QString("Consecutive file number | %1 item(s) under [%2]");
    setWindowTitle(windowTitleFormat);
  }
  auto InitControlTB() -> QToolBar* override {
    QToolBar* replaceControl = new QToolBar;
    replaceControl->addWidget(new QLabel("start no:"));
    replaceControl->addWidget(m_fileNoStartIndex);
    return replaceControl;
  }

  auto extraSubscribe() -> void override { connect(m_fileNoStartIndex, &QLineEdit::textEdited, this, &AdvanceRenamer::OnlyTriggerRenameCore); }

  auto InitExtraMemberWidget() -> void override {}
  auto RenameCore(const QStringList& replaceeList) -> QStringList override;
};

class RenameWidget_Insert : public AdvanceRenamer {
 public:
  QComboBox* insertStrCB;
  QComboBox* insertAtCB;

  RenameWidget_Insert(QWidget* parent = nullptr) : AdvanceRenamer(parent), insertStrCB(new QComboBox), insertAtCB(new QComboBox){};

  auto InitExtraCommonVariable() -> void override {
    windowTitleFormat = QString("Insert name string | %1 item(s) under [%2]");
    setWindowTitle(windowTitleFormat);
    setWindowIcon(QIcon(":/themes/NAME_STR_DELETER_PATH"));
  }
  auto InitControlTB() -> QToolBar* override {
    QToolBar* replaceControl = new QToolBar;
    replaceControl->addWidget(new QLabel("String:"));
    replaceControl->addWidget(insertStrCB);
    replaceControl->addWidget(new QLabel("Index:"));
    replaceControl->addWidget(insertAtCB);
    replaceControl->addSeparator();
    replaceControl->addWidget(EXT_INSIDE_FILENAME);
    replaceControl->addWidget(ITEMS_INSIDE_SUBDIR);
    replaceControl->addSeparator();
    replaceControl->addWidget(regexValidLabel);
    return replaceControl;
  }

  auto extraSubscribe() -> void override {
    connect(insertAtCB, &QComboBox::currentTextChanged, this, &AdvanceRenamer::OnlyTriggerRenameCore);
    connect(insertStrCB, &QComboBox::currentTextChanged, this, &AdvanceRenamer::OnlyTriggerRenameCore);
  }

  auto InitExtraMemberWidget() -> void override;
  auto RenameCore(const QStringList& replaceeList) -> QStringList override;
};

class RenameWidget_Replace : public AdvanceRenamer {
 public:
  QComboBox* oldStrCB;
  QComboBox* newStrCB;
  QCheckBox* regex;
  RenameWidget_Replace(QWidget* parent = nullptr)
      : AdvanceRenamer(parent), oldStrCB(new QComboBox), newStrCB(new QComboBox), regex(new QCheckBox("Regex")) {}

  auto InitExtraCommonVariable() -> void override {
    windowTitleFormat = QString("Replace name string | %1 item(s) under [%2]");
    setWindowTitle(windowTitleFormat);
    setWindowIcon(QIcon(":/themes/NAME_STR_REPLACER_PATH"));
  }

  auto InitControlTB() -> QToolBar* override {
    QToolBar* replaceControl = new QToolBar;
    replaceControl->addWidget(new QLabel("Old:"));
    replaceControl->addWidget(oldStrCB);
    replaceControl->addWidget(new QLabel("New:"));
    replaceControl->addWidget(newStrCB);
    replaceControl->addSeparator();
    replaceControl->addWidget(regex);
    replaceControl->addWidget(EXT_INSIDE_FILENAME);
    replaceControl->addWidget(ITEMS_INSIDE_SUBDIR);
    replaceControl->addSeparator();
    replaceControl->addWidget(regexValidLabel);
    return replaceControl;
  }
  auto extraSubscribe() -> void override {
    connect(oldStrCB, &QComboBox::currentTextChanged, this, &RenameWidget_Replace::OnlyTriggerRenameCore);
    connect(regex, &QCheckBox::stateChanged, this, &RenameWidget_Replace::onRegex);
    connect(newStrCB, &QComboBox::currentTextChanged, this, &RenameWidget_Replace::OnlyTriggerRenameCore);
  }

  auto InitExtraMemberWidget() -> void override;

  auto RenameCore(const QStringList& replaceeList) -> QStringList override;
};

class RenameWidget_Delete : public RenameWidget_Replace {
 public:
  RenameWidget_Delete(QWidget* parent = nullptr) : RenameWidget_Replace(parent) {
    newStrCB->setCurrentText("");
    newStrCB->setEnabled(false);
    newStrCB->setToolTip("New str is identically equal to empty str");
  }

  auto InitExtraCommonVariable() -> void override {
    windowTitleFormat = "Delete name string | %1 item(s) under [%2]";
    setWindowTitle(windowTitleFormat);
    setWindowIcon(QIcon(":/themes/NAME_STR_DELETER_PATH"));
  }
};

class RenameWidget_Numerize : public AdvanceRenamer {
 public:
  explicit RenameWidget_Numerize(QWidget* parent = nullptr) : AdvanceRenamer(parent) {
    EXT_INSIDE_FILENAME->setCheckState(Qt::CheckState::Unchecked);
    ITEMS_INSIDE_SUBDIR->setEnabled(false);
    ITEMS_INSIDE_SUBDIR->setCheckState(Qt::CheckState::Unchecked);
  }
  auto InitExtraCommonVariable() -> void override {
    windowTitleFormat = "Numerize name string | %1 item(s) under [%2]";
    setWindowTitle(windowTitleFormat);
    setWindowIcon(QIcon(":/themes/NAME_STR_NUMERIZER_PATH"));
  }
  auto InitControlTB() -> QToolBar* override {
    QToolBar* replaceControl(new QToolBar);
    replaceControl->addWidget(new QLabel("Complete base name:"));
    replaceControl->addWidget(m_completeBaseName);
    replaceControl->addSeparator();
    replaceControl->addWidget(new QLabel("Start No:"));
    replaceControl->addWidget(m_startNo);
    replaceControl->addSeparator();
    replaceControl->addWidget(EXT_INSIDE_FILENAME);
    replaceControl->addWidget(ITEMS_INSIDE_SUBDIR);
    return replaceControl;
  }
  auto extraSubscribe() -> void override {
    connect(m_startNo, &QLineEdit::textChanged, this, &RenameWidget_Numerize::OnlyTriggerRenameCore);
    connect(m_completeBaseName, &QLineEdit::textChanged, this, &RenameWidget_Numerize::OnlyTriggerRenameCore);
  }
  auto InitExtraMemberWidget() -> void override {}
  auto RenameCore(const QStringList& replaceeList) -> QStringList override;

 private:
  QLineEdit* m_startNo{new QLineEdit("0")};
  QLineEdit* m_completeBaseName{new QLineEdit};
  bool m_baseNameInited = false;
};

#include "Actions/RenameActions.h"
class RenameWidget_Case : public AdvanceRenamer {
 public:
  QActionGroup* caseAG;
  QToolBar* caseTB;

  explicit RenameWidget_Case(QWidget* parent = nullptr) : AdvanceRenamer(parent), caseAG(g_renameAg().NAME_CASE), caseTB(new QToolBar) {}
  auto InitExtraCommonVariable() -> void override {
    windowTitleFormat = "Case name string | %1 item(s) under [%2]";
    setWindowTitle(windowTitleFormat);
    setWindowIcon(QIcon(":/themes/NAME_STR_CASE"));
  }
  auto InitControlTB() -> QToolBar* override {
    QToolBar* replaceControl(new QToolBar);
    replaceControl->addWidget(new QLabel("Case:"));
    replaceControl->addWidget(caseTB);
    replaceControl->addSeparator();
    replaceControl->addWidget(EXT_INSIDE_FILENAME);
    replaceControl->addWidget(ITEMS_INSIDE_SUBDIR);
    return replaceControl;
  }
  auto extraSubscribe() -> void override { connect(caseTB, &QToolBar::actionTriggered, this, &AdvanceRenamer::OnlyTriggerRenameCore); }
  auto InitExtraMemberWidget() -> void override {
    caseTB->addActions(caseAG->actions());
    caseTB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return;
  }

  auto RenameCore(const QStringList& replaceeList) -> QStringList override;

  static QStringList ChangeCaseRename(const QStringList& replaceeList, const QString& caseRuleName);
};

class RenameWidget_SwapSection : public AdvanceRenamer {
 public:
  QToolBar* swapTB;
  QActionGroup* caseAG;

  explicit RenameWidget_SwapSection(QWidget* parent = nullptr) : AdvanceRenamer(parent), swapTB(new QToolBar), caseAG(new QActionGroup(this)) {
    EXT_INSIDE_FILENAME->setChecked(false);
  }
  auto InitExtraCommonVariable() -> void override {
    windowTitleFormat = "Swap section name string | %1 item(s) under [%2]";
    setWindowTitle(windowTitleFormat);
    setWindowIcon(QIcon(":/themes/NAME_STR_SWAPPER_PATH"));
  }
  auto InitControlTB() -> QToolBar* override {
    QToolBar* replaceControl(new QToolBar);
    replaceControl->addWidget(new QLabel("Swap:"));
    replaceControl->addWidget(swapTB);
    replaceControl->addSeparator();
    replaceControl->addWidget(ITEMS_INSIDE_SUBDIR);
    return replaceControl;
  }
  auto extraSubscribe() -> void override { connect(swapTB, &QToolBar::actionTriggered, this, &AdvanceRenamer::OnlyTriggerRenameCore); }
  auto InitExtraMemberWidget() -> void override {
    QAction* section12 = new QAction("1 <> 2");
    QAction* section23 = new QAction("2 <> 3");
    QAction* section01 = new QAction("0 <> 1");

    caseAG->addAction(section12);
    caseAG->addAction(section23);
    caseAG->addAction(section01);

    caseAG->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);
    for (QAction* act : caseAG->actions()) {
      act->setCheckable(true);
    }
    section12->setChecked(true);

    swapTB->addActions(caseAG->actions());
    swapTB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  }
  auto RenameCore(const QStringList& replaceeList) -> QStringList override;
};

#endif  // ADVANCERENAMER_H
