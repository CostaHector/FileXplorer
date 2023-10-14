#ifndef RENAMEWIDGET_H
#define RENAMEWIDGET_H

#include "PublicVariable.h"

#include <QDialog>
#include <QToolBar>
#include <QSet>
#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QSet>
#include <PublicTool.h>
#include <QScrollBar>

#include "Component/StateLabel.h"
#include "Component/Toaster.h"

class RenameWidget : public QDialog
{
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

    explicit RenameWidget(QWidget* parent = nullptr):
        QDialog(parent),
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
        te(new QPlainTextEdit)
    {
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

    auto Subscribe() -> void{
        connect(includingSuffix, &QCheckBox::stateChanged, this, &RenameWidget::onIncludeSuffix);
        connect(includingSub, &QCheckBox::stateChanged, this, &RenameWidget::onIncludingSub);

        connect(newCompleteName->verticalScrollBar(), &QScrollBar::sliderMoved, relName->verticalScrollBar(), &QScrollBar::setValue);
        connect(newCompleteName->verticalScrollBar(), &QScrollBar::sliderMoved, oldCompleteName->verticalScrollBar(), &QScrollBar::setValue);
        connect(newCompleteName->verticalScrollBar(), &QScrollBar::sliderMoved, oldSuffix->verticalScrollBar(), &QScrollBar::setValue);
        connect(newCompleteName->verticalScrollBar(), &QScrollBar::sliderMoved, newSuffix->verticalScrollBar(), &QScrollBar::setValue);

        connect(buttonBox->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, this, [this](){onApply(false, true);});
        connect(buttonBox->button(QDialogButtonBox::StandardButton::Help), &QPushButton::clicked, this, [this](){onApply(true,false);});
        connect(buttonBox->button(QDialogButtonBox::StandardButton::Cancel), &QPushButton::clicked, this, &RenameWidget::close);
    }

public:
    auto Checker(const QStringList& olds, const QStringList& news) -> bool {
        if (olds.size() != news.size()) {
            const QString& msg=QString("Check failed [length inequal], but left[%1]!=right[%2]").arg(olds.size()).arg(news.size());
            qDebug(msg.toStdString().c_str());
            QMessageBox::warning(this, "Check failed [length inequal]", msg);
            return false;
        }
        QSet<QString> selfDupLst(news.cbegin(), news.cend());
        if (selfDupLst.size() < news.size()) {
            const QString& msg=QString("Check failed [new name not unique], news[%1], unique[%2]").arg(selfDupLst.size()).arg(news.size());
            qDebug(msg.toStdString().c_str());
            QMessageBox::warning(this, "Check failed [new name not unique]", msg);
            return false;
        }

        QStringList invalidFileNames;
        for (const QString fileName: news){
            if (not IsFileNameValid(fileName)){
                invalidFileNames.append(fileName);
            }
        }
        if (not invalidFileNames.isEmpty()){
            const QString& invalidFileNameStr = invalidFileNames.join('\n');
            const QString& msg = QString("Check Failed [invalid new name]. %1 invalid name(s) find:\n%2").arg(invalidFileNames.size()).arg(invalidFileNameStr.toStdString().c_str());
            qDebug(msg.toStdString().c_str());
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

        if (not (relNameList.size()==oldCompleteNameList.size() and oldCompleteNameList.size()==oldSuffixList.size() and oldSuffixList.size()==newCompleteNameList.size() and newCompleteNameList.size()==newSuffixList.size())){
            const QString& msg = QString("Dont add/delete line only in one column<br/>(%1,%2,%3,%4,%5)").arg(relNameList.size()).arg(oldCompleteNameList.size()).arg(oldSuffixList.size()).arg(newCompleteNameList.size()).arg(newSuffixList.size());
            QMessageBox::warning(this, "Check Failed length inequal", msg);
            return false;
        }

        auto itEmpty = std::find_if(newCompleteNameList.cbegin(), newCompleteNameList.cend(), [](const QString& s)->bool{return s.isEmpty();});
        if (itEmpty != newCompleteNameList.cend()){
            const QString& msg("File name cannot be empty");
            qDebug("Check Failed some complete filename is empty, %s", msg.toStdString().c_str());
            QMessageBox::warning(this, "Check Failed some complete filename is empty", msg);
            return false;
        }

        QStringList olds;
        QStringList news;
        for(auto i = 0; i< oldCompleteNameList.size(); ++i) {
            QString bs = (oldSuffixList[i].isEmpty()? "":".") + oldSuffixList[i];
            QString ns = (newSuffixList[i].isEmpty()? "":".") + newSuffixList[i];
            QString relTmp = relNameList[i] + (relNameList[i].isEmpty()? "":"/");
            olds.append(relTmp + oldCompleteNameList[i] + bs);
            news.append(relTmp + newCompleteNameList[i] + ns);
        }
        if (not Checker(olds, news)) {
            if (isInterative) {
                (new Toaster(this, QString("Checked failed | %1").arg(olds.size()), false))->exec();
            }
            return false;
        }

        // skip mediate tempR;
        QList<QStringList> cmds;
        for (int i = 0; i < olds.size(); ++i){
            cmds.append({"rename", pre, olds[i], pre, news[i]});
        }

        decltype(cmds) reversedcmds(cmds.crbegin(), cmds.crend());// rename files first, than its folders;

        if (isOnlyHelp) {
            for (const QStringList& cmd:reversedcmds) {
                te->appendPlainText(cmd.join('\t'));
            }
            te->setWindowTitle(QString("%1 Command(s)").arg(cmds.size()));
            te->setMinimumWidth(1024);
            te->show();
            return true;
        }
        return true;
        //                bool isAllSuccess = g_undoRedo.Do(cmds);
        //                if (isInterative) {
        //                    if (isAllSuccess) {
        //                        close();
        //                        (new Toaster(this, QString("%1 command(s).").arg(cmds.size())), isAllSuccess))->exec_();
        //                }
        //                return isAllSuccess;
    }
    auto onRegex(const Qt::CheckState regexState) -> void{
        // regexState;
        OnlyTriggerRenameCore();
    }
    auto onIncludingSub(int includingSubState) -> void{
        InitTextContent(pre, rels);
    }
    auto onIncludeSuffix(int includingSuffixState) -> void{
        bool vis = not (includingSuffixState == Qt::Checked);
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

        setWindowTitle(windowTitleFormat.arg(pre).arg(completeNames.size()));

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
    virtual auto InitControlTB()->QToolBar* = 0;
    virtual void extraSubscribe() = 0;
    virtual auto RenameCore(const QStringList& replaceeList) -> QStringList = 0;

    static inline auto IsFileNameValid(const QString& filename) -> bool{
        QSet<QChar> nameSet(filename.cbegin(), filename.cend());
        return nameSet.intersect(RenameWidget::INVALID_FILE_NAME_CHAR_SET).isEmpty();
    }
};

#include <QComboBox>

class RenameWidget_Insert: public RenameWidget{
public:
    QLineEdit* insertStr;
    QComboBox* insertStrCB;
    QLineEdit* insertAt;
    QComboBox* insertAtCB;

    RenameWidget_Insert(QWidget* parent = nullptr):
        RenameWidget(parent),
        insertStr(new QLineEdit),
        insertStrCB(new QComboBox),
        insertAt(new QLineEdit),
        insertAtCB(new QComboBox){
        };

    auto InitExtraCommonVariable()->void override{
        windowTitleFormat = QString("Insert name string | %1 item(s) under [%2]");
        setWindowTitle(windowTitleFormat);
        setWindowIcon(QIcon(":/themes/NAME_STR_INSERTER_PATH"));
    }
    auto InitControlTB() -> QToolBar* override{
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

    auto extraSubscribe()->void override{
        connect(insertAt, &QLineEdit::textChanged, this, &RenameWidget::OnlyTriggerRenameCore);
        connect(insertStr, &QLineEdit::textChanged, this, &RenameWidget::OnlyTriggerRenameCore);
    }

    auto InitExtraMemberWidget() -> void override{
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
    auto RenameCore(const QStringList& replaceeList) -> QStringList override{
        if (replaceeList.isEmpty()){
            return replaceeList;
        }
        const QString& insertString = insertStr->text();
        if (insertString.isEmpty()){
            return replaceeList;
        }
        const QString& insertAtStr = insertAt->text();

        bool isInt=false;
        int insertAt = insertAtStr.toInt(&isInt);
        if (not isInt){
            qDebug("insert index is not number[%s]", insertAtStr.toStdString().c_str());
            return replaceeList;
        }

        QStringList afterInsert;
        for(const QString& replacee: replaceeList){
            int realInsertAt = (insertAt > replacee.size())? replacee.size():insertAt;
            afterInsert.append(replacee.left(realInsertAt) + insertString + replacee.mid(realInsertAt));
        }
        return afterInsert;
    }
};
#endif // RENAMEWIDGET_H
