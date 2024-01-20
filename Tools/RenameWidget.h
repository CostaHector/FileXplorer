#ifndef RENAMEWIDGET_H
#define RENAMEWIDGET_H

#include "PublicVariable.h"

#include <PublicTool.h>
#include <QCheckBox>
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
#include "Component/notificator.h"
#include "UndoRedo.h"

class RenameWidget : public QDialog {
 public:
  static const QString INVALID_CHARS;
  static const QSet<QChar> INVALID_FILE_NAME_CHAR_SET;
  QString windowTitleFormat;
  QString pre;
  QStringList rels;
  QStringList completeNames;
  QList<bool> isFiles;

  QCheckBox* includingSuffix;
  QCheckBox* includingSub;
  StateLabel* regexValidLabel;

  QPlainTextEdit* relName;
  QPlainTextEdit* oldCompleteName;
  QPlainTextEdit* oldSuffix;
  QPlainTextEdit* newCompleteName;
  QPlainTextEdit* newSuffix;
  QDialogButtonBox* buttonBox;
  QPlainTextEdit* te;

  explicit RenameWidget(QWidget* parent = nullptr)
      : QDialog(parent),
        windowTitleFormat("%1 | %2"),
        includingSuffix(new QCheckBox("Including suffix")),
        includingSub(new QCheckBox("Subdirectory")),
        regexValidLabel(new StateLabel("Regex expression state")),
        relName(new QPlainTextEdit()),
        oldCompleteName(new QPlainTextEdit()),
        oldSuffix(new QPlainTextEdit()),
        newCompleteName(new QPlainTextEdit()),
        newSuffix(new QPlainTextEdit()),
        buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help)),
        te(new QPlainTextEdit) {
    // Qt.FramelessWindowHint|Qt.WindowSystemMenuHint;
    setMinimumWidth(1024);
    setWindowFlag(Qt::WindowMaximizeButtonHint);  // WindowMinMaxButtonsHint;

    buttonBox->setOrientation(Qt::Orientation::Horizontal);
    buttonBox->button(QDialogButtonBox::Ok)->setStyleSheet(SUBMIT_BTN_STYLE);
  }

  void init() {
    InitExtraCommonVariable();

    InitExtraMemberWidget();
    includingSuffix->setToolTip("Rules will also work on suffix");

    includingSub->setToolTip("Rules will also work on itself and its subdirectories");

    QToolBar* replaceControl = InitControlTB();

    relName->setWordWrapMode(QTextOption::WrapMode::NoWrap);
    relName->setReadOnly(true);
    relName->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    oldCompleteName->setWordWrapMode(QTextOption::WrapMode::NoWrap);
    oldCompleteName->setReadOnly(true);
    oldCompleteName->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    oldSuffix->setReadOnly(true);
    oldSuffix->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    newCompleteName->setWordWrapMode(QTextOption::WrapMode::NoWrap);
    newCompleteName->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    newSuffix->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QHBoxLayout* nameLayout = new QHBoxLayout;
    nameLayout->addWidget(relName);
    nameLayout->addWidget(oldCompleteName);
    nameLayout->addWidget(oldSuffix);
    nameLayout->addWidget(newCompleteName);
    nameLayout->addWidget(newSuffix);

    nameLayout->setStretch(0, 3);
    nameLayout->setStretch(1, 8);
    nameLayout->setStretch(2, 1);
    nameLayout->setStretch(3, 8);
    nameLayout->setStretch(4, 1);
    nameLayout->setSpacing(0);
    nameLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addWidget(replaceControl);
    mainLayout->addLayout(nameLayout);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    Subscribe();
    extraSubscribe();

    oldSuffix->setVisible(not includingSuffix->isChecked());
    newSuffix->setVisible(not includingSuffix->isChecked());
  }

  auto Subscribe() -> void {
    connect(includingSuffix, &QCheckBox::stateChanged, this, &RenameWidget::onIncludeSuffix);
    connect(includingSub, &QCheckBox::stateChanged, this, &RenameWidget::onIncludingSub);

    connect(newCompleteName->verticalScrollBar(), &QScrollBar::sliderMoved, relName->verticalScrollBar(), &QScrollBar::setValue);
    connect(newCompleteName->verticalScrollBar(), &QScrollBar::sliderMoved, oldCompleteName->verticalScrollBar(), &QScrollBar::setValue);
    connect(newCompleteName->verticalScrollBar(), &QScrollBar::sliderMoved, oldSuffix->verticalScrollBar(), &QScrollBar::setValue);
    connect(newCompleteName->verticalScrollBar(), &QScrollBar::sliderMoved, newSuffix->verticalScrollBar(), &QScrollBar::setValue);

    connect(buttonBox->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, this, [this]() { onApply(false, true); });
    connect(buttonBox->button(QDialogButtonBox::StandardButton::Help), &QPushButton::clicked, this, [this]() { onApply(true, false); });
    connect(buttonBox->button(QDialogButtonBox::StandardButton::Cancel), &QPushButton::clicked, this, &RenameWidget::close);
  }

 public:
  auto Checker(const QStringList& olds, const QStringList& news) -> bool {
    if (olds.size() != news.size()) {
      const QString& msg = QString("Check failed [length inequal], but left[%1]!=right[%2]").arg(olds.size()).arg(news.size());
      qDebug("%s", qPrintable(msg));
      QMessageBox::warning(this, "Check failed [length inequal]", msg);
      return false;
    }
    QSet<QString> selfDupLst(news.cbegin(), news.cend());
    if (selfDupLst.size() < news.size()) {
      const QString& msg = QString("Check failed [new name not unique], news[%1], unique[%2]").arg(selfDupLst.size()).arg(news.size());
      qDebug("%s", qPrintable(msg));
      QMessageBox::warning(this, "Check failed [new name not unique]", msg);
      return false;
    }

    QStringList invalidFileNames;
    for (const QString& fileName : news) {
      if (not IsFileNameValid(fileName)) {
        invalidFileNames.append(fileName);
      }
    }
    if (not invalidFileNames.isEmpty()) {
      const QString& invalidFileNameStr = invalidFileNames.join('\n');
      const QString& msg = QString("Check Failed [invalid new name]. %1 invalid name(s) find:\n%2")
                               .arg(invalidFileNames.size())
                               .arg(invalidFileNameStr.toStdString().c_str());
      qDebug("%s", qPrintable(msg));
      QMessageBox::warning(this, "Check Failed [invalid new name]", msg);
      return false;
    }
    return true;
  }
  auto onApply(const bool isOnlyHelp = false, const bool isInterative = false) -> bool {
    QStringList relNameList = relName->toPlainText().split('\n');
    QStringList oldCompleteNameList = oldCompleteName->toPlainText().split('\n');
    QStringList oldSuffixList = oldSuffix->toPlainText().split('\n');

    QStringList newCompleteNameList = newCompleteName->toPlainText().split('\n');
    QStringList newSuffixList = newSuffix->toPlainText().split('\n');

    if (not(relNameList.size() == oldCompleteNameList.size() and oldCompleteNameList.size() == oldSuffixList.size() and
            oldSuffixList.size() == newCompleteNameList.size() and newCompleteNameList.size() == newSuffixList.size())) {
      const QString& msg = QString("Dont add/delete line only in one column<br/>(%1,%2,%3,%4,%5)")
                               .arg(relNameList.size())
                               .arg(oldCompleteNameList.size())
                               .arg(oldSuffixList.size())
                               .arg(newCompleteNameList.size())
                               .arg(newSuffixList.size());
      QMessageBox::warning(this, "Check Failed length inequal", msg);
      return false;
    }

    auto itEmpty = std::find_if(newCompleteNameList.cbegin(), newCompleteNameList.cend(), [](const QString& s) -> bool { return s.isEmpty(); });
    if (itEmpty != newCompleteNameList.cend()) {
      const QString& msg("File name cannot be empty");
      qDebug("Check Failed some complete filename is empty, %s", msg.toStdString().c_str());
      QMessageBox::warning(this, "Check Failed some complete filename is empty", msg);
      return false;
    }

    QStringList olds;
    QStringList news;
    for (auto i = 0; i < oldCompleteNameList.size(); ++i) {
      QString bs = (oldSuffixList[i].isEmpty() ? "" : ".") + oldSuffixList[i];
      QString ns = (newSuffixList[i].isEmpty() ? "" : ".") + newSuffixList[i];
      QString relTmp = relNameList[i] + (relNameList[i].isEmpty() ? "" : "/");
      olds.append(relTmp + oldCompleteNameList[i] + bs);
      news.append(relTmp + newCompleteNameList[i] + ns);
    }
    if (not Checker(olds, news)) {
      if (isInterative) {
        Notificator::warning("[Abort] Name conflicts", QString("olds[%1], news[%2]").arg(olds.size()).arg(news.size()));
      }
      return false;
    }

    // skip mediate tempR;
    FileOperation::BATCH_COMMAND_LIST_TYPE cmds;
    for (int i = 0; i < olds.size(); ++i) {
      if (olds[i] == news[i]) {
        continue;
      }
      cmds.append({"rename", pre, olds[i], pre, news[i]});
    }

    decltype(cmds) reversedcmds(cmds.crbegin(), cmds.crend());  // rename files first, than its folders;

    if (isOnlyHelp) {
      for (const QStringList& cmd : reversedcmds) {
        te->appendPlainText(cmd.join('\t'));
      }
      te->setWindowTitle(QString("%1 Command(s)").arg(cmds.size()));
      te->setMinimumWidth(1024);
      te->show();
      return true;
    }
    bool isAllSuccess = g_undoRedo.Do(reversedcmds);
    if (isInterative) {
      if (isAllSuccess) {
        Notificator::information("Batch rename", QString("%1 command(s).").arg(reversedcmds.size()));
      }
    }
    close();
    return isAllSuccess;
  }
  auto onRegex(const int regexState) -> void {
    // regexState;
    OnlyTriggerRenameCore();
  }
  auto onIncludingSub(int includingSubState) -> void { InitTextContent(pre, rels); }
  auto onIncludeSuffix(int includingSuffixState) -> void {
    bool vis = not(includingSuffixState == Qt::Checked);
    oldSuffix->setVisible(vis);
    newSuffix->setVisible(vis);
    InitTextContent(pre, rels);
  }

  auto InitTextContent(const QString& p, const QStringList& r) -> void {
    pre = p;
    rels = r;

    const Qt::CheckState includingSubState = includingSub->checkState();
    const Qt::CheckState includingSuffixState = includingSuffix->checkState();

    OSWalker_RETURN osWalkerRet = OSWalker(pre, rels, includingSubState == Qt::Checked, includingSuffixState == Qt::Checked);
    const auto& relToNames = osWalkerRet.relToNames;
    completeNames = osWalkerRet.completeNames;
    const auto& suffixs = osWalkerRet.suffixs;
    isFiles = osWalkerRet.isFiles;

    setWindowTitle(windowTitleFormat.arg(completeNames.size()).arg(pre));

    relName->setPlainText(relToNames.join('\n'));

    oldCompleteName->setPlainText(completeNames.join('\n'));
    oldSuffix->setPlainText(suffixs.join('\n'));

    const auto& newCompleteNames = RenameCore(completeNames);
    newCompleteName->setPlainText(newCompleteNames.join('\n'));
    newSuffix->setPlainText(suffixs.join('\n'));
  }
  auto OnlyTriggerRenameCore() -> void {
    // will not call OSWalker.;
    // only update complete name;
    const auto& newCompleteNames = RenameCore(completeNames);
    newCompleteName->setPlainText(newCompleteNames.join('\n'));
  }

  virtual void InitExtraCommonVariable() = 0;
  virtual void InitExtraMemberWidget() = 0;
  virtual auto InitControlTB() -> QToolBar* = 0;
  virtual void extraSubscribe() = 0;
  virtual auto RenameCore(const QStringList& replaceeList) -> QStringList = 0;

  static inline auto IsFileNameValid(const QString& filename) -> bool {
    QSet<QChar> nameSet(filename.cbegin(), filename.cend());
    return nameSet.intersects(RenameWidget::INVALID_FILE_NAME_CHAR_SET);
  }
};

#include <QComboBox>

class RenameWidget_Insert : public RenameWidget {
 public:
  QLineEdit* insertStr;
  QComboBox* insertStrCB;
  QLineEdit* insertAt;
  QComboBox* insertAtCB;

  RenameWidget_Insert(QWidget* parent = nullptr)
      : RenameWidget(parent), insertStr(new QLineEdit), insertStrCB(new QComboBox), insertAt(new QLineEdit), insertAtCB(new QComboBox){};

  auto InitExtraCommonVariable() -> void override {
    windowTitleFormat = QString("Insert name string | %1 item(s) under [%2]");
    setWindowTitle(windowTitleFormat);
    setWindowIcon(QIcon(":/themes/NAME_STR_INSERTER_PATH"));
  }
  auto InitControlTB() -> QToolBar* override {
    QToolBar* replaceControl = new QToolBar;
    replaceControl->addWidget(new QLabel("Str:"));
    replaceControl->addWidget(insertStrCB);
    replaceControl->addWidget(new QLabel("Insert:"));
    replaceControl->addWidget(insertAtCB);
    replaceControl->addSeparator();
    replaceControl->addWidget(includingSuffix);
    replaceControl->addWidget(includingSub);
    replaceControl->addSeparator();
    replaceControl->addWidget(regexValidLabel);
    return replaceControl;
  }

  auto extraSubscribe() -> void override {
    connect(insertAt, &QLineEdit::textChanged, this, &RenameWidget::OnlyTriggerRenameCore);
    connect(insertStr, &QLineEdit::textChanged, this, &RenameWidget::OnlyTriggerRenameCore);
  }

  auto InitExtraMemberWidget() -> void override {
    insertStrCB->addItems({" - 810p", " - 720p", " - 1080p", " - 4K", " - FHD", " - UHD"});
    insertStrCB->setLineEdit(insertStr);
    insertStrCB->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    insertStrCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
    insertStr->selectAll();

    insertAtCB->addItems({"128", "0", "50", "100", "200"});
    insertAtCB->setLineEdit(insertAt);
    insertAtCB->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    insertAtCB->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
  }
  auto RenameCore(const QStringList& replaceeList) -> QStringList override;
};

class RenameWidget_Replace : public RenameWidget {
 public:
  QLineEdit* oldStr;
  QComboBox* oldStrCB;
  QLineEdit* newStr;
  QComboBox* newStrCB;
  QCheckBox* regex;
  RenameWidget_Replace(QWidget* parent = nullptr)
      : RenameWidget(parent),
        oldStr(new QLineEdit),
        oldStrCB(new QComboBox),
        newStr(new QLineEdit),
        newStrCB(new QComboBox),
        regex(new QCheckBox("Regex")) {}

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
    replaceControl->addWidget(includingSuffix);
    replaceControl->addWidget(includingSub);
    replaceControl->addSeparator();
    replaceControl->addWidget(regexValidLabel);
    return replaceControl;
  }
  auto extraSubscribe() -> void override {
    connect(oldStr, &QLineEdit::textChanged, this, &RenameWidget_Replace::OnlyTriggerRenameCore);
    connect(regex, &QCheckBox::stateChanged, this, &RenameWidget_Replace::onRegex);
    connect(newStr, &QLineEdit::textChanged, this, &RenameWidget_Replace::OnlyTriggerRenameCore);
  }

  auto InitExtraMemberWidget() -> void override {
    oldStrCB->addItems({" BB ", " BB", " - 1080p", " - 4K", " - FHD", " - UHD"});
    oldStrCB->setLineEdit(oldStr);
    oldStrCB->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    oldStrCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
    oldStr->selectAll();

    newStrCB->addItems({" ", "", " - 1080p"});
    newStrCB->setLineEdit(newStr);
    newStrCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
    newStr->selectAll();

    regex->setToolTip("Regular expression enable switch");
  }

  auto RenameCore(const QStringList& replaceeList) -> QStringList override;
};

class RenameWidget_Delete : public RenameWidget_Replace {
 public:
  RenameWidget_Delete(QWidget* parent = nullptr) : RenameWidget_Replace(parent) {
    newStr->setText("");
    newStrCB->setEnabled(false);
    newStrCB->setToolTip("New str is identically equal to empty str");
  }

  auto InitExtraCommonVariable() -> void override {
    windowTitleFormat = "Delete name string | %1 item(s) under [%2]";
    setWindowTitle(windowTitleFormat);
    setWindowIcon(QIcon(":/themes/NAME_STR_DELETER_PATH"));
  }
  auto extraSubscribe() -> void override {
    connect(oldStr, &QLineEdit::textChanged, this, &RenameWidget_Replace::OnlyTriggerRenameCore);
    connect(regex, &QCheckBox::stateChanged, this, &RenameWidget_Replace::onRegex);
  }
  auto InitControlTB() -> QToolBar* override {
    QToolBar* replaceControl(new QToolBar);
    replaceControl->addWidget(new QLabel("Old:"));
    replaceControl->addWidget(oldStrCB);
    replaceControl->addSeparator();
    replaceControl->addWidget(regex);
    replaceControl->addWidget(includingSuffix);
    replaceControl->addWidget(includingSub);
    replaceControl->addSeparator();
    replaceControl->addWidget(regexValidLabel);
    return replaceControl;
  }
};

class RenameWidget_Numerize : public RenameWidget {
 public:
  QLineEdit* m_startNo;
  QLineEdit* m_completeBaseName;

  explicit RenameWidget_Numerize(QWidget* parent = nullptr) : RenameWidget(parent), m_startNo(new QLineEdit("0")), m_completeBaseName(new QLineEdit) {
    includingSuffix->setChecked(false);
    includingSub->setChecked(false);
  }
  auto InitExtraCommonVariable() -> void override {
    windowTitleFormat = "Numerize name string | %1 item(s) under [%2]";
    setWindowTitle(windowTitleFormat);
    setWindowIcon(QIcon(":/themes/NAME_STR_NUMERIZER_PATH"));
  }
  auto InitControlTB() -> QToolBar* override {
    QToolBar* replaceControl(new QToolBar);
    replaceControl->addWidget(new QLabel("Complete base name"));
    replaceControl->addWidget(m_completeBaseName);
    replaceControl->addWidget(new QLabel("Start No:"));
    replaceControl->addWidget(m_startNo);
    return replaceControl;
  }
  auto extraSubscribe() -> void override {
    connect(m_startNo, &QLineEdit::textChanged, this, &RenameWidget_Numerize::OnlyTriggerRenameCore);
    connect(m_completeBaseName, &QLineEdit::textChanged, this, &RenameWidget_Numerize::OnlyTriggerRenameCore);
  }
  auto InitExtraMemberWidget() -> void override {
    m_startNo = new QLineEdit("0");
    m_completeBaseName = new QLineEdit;
  }
  auto RenameCore(const QStringList& replaceeList) -> QStringList override;
};

#include "Actions/RenameActions.h"
class RenameWidget_Case : public RenameWidget {
 public:
  QActionGroup* caseAG;
  QToolBar* caseTB;

  explicit RenameWidget_Case(QWidget* parent = nullptr) : RenameWidget(parent), caseAG(g_renameAg().NAME_CASE), caseTB(new QToolBar) {}
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
    replaceControl->addWidget(includingSuffix);
    replaceControl->addWidget(includingSub);
    return replaceControl;
  }
  auto extraSubscribe() -> void override { connect(caseTB, &QToolBar::actionTriggered, this, &RenameWidget::OnlyTriggerRenameCore); }
  auto InitExtraMemberWidget() -> void override {
    caseTB->addActions(caseAG->actions());
    caseTB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return;
  }
  //    FIRST_LETTER_OF_EACH_WORD_COMP = re.compile("(^|\s)(\S)");

  static QString CapitaliseEachWordFirstLetterOnly(const QString& sentence) {
    QStringList words = sentence.split(' ', Qt::SkipEmptyParts);
    for (QString& word : words) {
      word.front() = word.front().toUpper();
    }
    return words.join(" ");
  }
  static QString CapitaliseEachWordFirstLetterLowercaseOthers(QString sentence) {
    const QString& sentenceInLowercase = sentence.toLower();
    return CapitaliseEachWordFirstLetterOnly(sentenceInLowercase);
  }

  static QString ToggleSentenceCase(const QString& sentence) {
    QString toggled;
    for (QChar c : sentence) {
      if (c.isLetter()) {
        toggled += (c.toLatin1() ^ 0x20);  // trick upper to lower case by bitwise operator
      } else {
        toggled += c;
      }
    }
    return toggled;
  }

  auto RenameCore(const QStringList& replaceeList) -> QStringList override;

  static QStringList ChangeCaseRename(const QStringList& replaceeList, const QString& caseRuleName);
};

class RenameWidget_SwapSection : public RenameWidget {
 public:
  QToolBar* swapTB;
  QActionGroup* caseAG;

  explicit RenameWidget_SwapSection(QWidget* parent = nullptr) : RenameWidget(parent), swapTB(new QToolBar), caseAG(new QActionGroup(this)) {
    includingSuffix->setChecked(false);
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
    replaceControl->addWidget(includingSub);
    return replaceControl;
  }
  auto extraSubscribe() -> void override { connect(swapTB, &QToolBar::actionTriggered, this, &RenameWidget::OnlyTriggerRenameCore); }
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

#endif  // RENAMEWIDGET_H
