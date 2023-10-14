#include "RenameWidget.h"

const QString RenameWidget::INVALID_CHARS("*?\"<>|");
const QSet<QChar> RenameWidget::INVALID_FILE_NAME_CHAR_SET(INVALID_CHARS.cbegin(), INVALID_CHARS.cend());

QStringList RenameWidget_Insert::RenameCore(const QStringList &replaceeList){
    if (replaceeList.isEmpty()){
        return replaceeList;
    }
    const QString& insertString = insertStr->text();
    if (insertString.isEmpty()){
        return replaceeList;
    }
    const QString& insertAtStr = insertAt->text();

    bool isnumeric=false;
    int insertAt = insertAtStr.toInt(&isnumeric);
    if (not isnumeric){
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

QStringList RenameWidget_Replace::RenameCore(const QStringList &replaceeList) {
    const QString& oldString = oldStr->text();
    const QString& newString = newStr->text();
    auto regexEnable = regex->isChecked();
    if (oldString.isEmpty()){
        return replaceeList;
    }
    if (not regexEnable){
        QStringList replacedLst(replaceeList);
        for (QString& s:replacedLst){
            s.replace(oldString, newString);
        }
        return replacedLst;
    }

    QRegExp repRegex(oldString);
    if (not repRegex.isValid()){
        const QString& msg = QString("invalid regex[%1]").arg(oldString);
        qDebug(msg.toStdString().c_str());
        regexValidLabel->ToNotSaved();
        return replaceeList;
    }
    regexValidLabel->ToSaved();
    QStringList replacedLst(replaceeList);
    for (QString& s: replacedLst){
        s.replace(repRegex, newString);
    }
    return replacedLst;
}


QStringList RenameWidget_Numerize::RenameCore(const QStringList &replaceeList) {
    QString startNoStr = startNo->text();

    bool isnumeric = false;
    int startNo = startNoStr.toInt(&isnumeric);
    if (not isnumeric){
        qDebug("start index is not number[%s]", startNoStr.toStdString().c_str());
        return replaceeList;
    }

    if (completeBaseName->text().isEmpty()){
        // set default complete basename
        completeBaseName->setText(replaceeList[0]);
        completeBaseName->selectAll();
    }
    const QString& completeBaseNameString = completeBaseName->text();
    const QStringList& suffixs = oldSuffix->toPlainText().split('\n');

    QMap<QString, int> sufCntMap;
    for (const QString& suf: suffixs){
        if (sufCntMap.contains(suf)){
            sufCntMap[suf] = 1;
        }else{
            ++sufCntMap[suf];
        }
    }
    QMap<QString, int> sufCurIndex;
    for (const QString& suf : sufCntMap.keys()){
        if (sufCntMap[suf] > 1){
            sufCurIndex[suf] = startNo;
        }
    }
    QStringList numerizedNames;
    for (const QString& suf : suffixs){
        if (not sufCntMap.contains(suf)){
            numerizedNames.append(completeBaseNameString);
            continue;
        }
        const QString& newBaseName = QString("%1%2(%3)").
                                     arg(completeBaseNameString).
                                     arg(completeBaseNameString.isEmpty()? "":" ").
                                     arg(sufCurIndex[suf]);
        numerizedNames.append(newBaseName);
        sufCurIndex[suf] += 1;
    }
    return numerizedNames;
}


//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char *argv[]){
    QDir dir(QFileInfo(QFileInfo(__FILE__).absolutePath()).absolutePath());
    QStringList rels  = dir.entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::DirsFirst);

    QApplication a(argc, argv);
    RenameWidget_Insert winI;
    RenameWidget_Replace winR;
    RenameWidget_Delete winD;
    RenameWidget_Numerize winN;
    winI.init();
    winR.init();
    winD.init();
    winN.init();
    winI.InitTextContent(dir.absolutePath(), rels);
    winR.InitTextContent(dir.absolutePath(), rels);
    winD.InitTextContent(dir.absolutePath(), rels);
    winN.InitTextContent(dir.absolutePath(), rels);
    // winR.InitTextContent(pre, rels)
    // winD.InitTextContent(pre, rels)
    // winN.InitTextContent(pre, rels)
    winI.show();
    winR.show();
    winD.show();
    winN.show();
    return a.exec();
}
#endif

