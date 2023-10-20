#include "ConflictsItemHelper.h"

auto TestMvConflictItemIF() -> void{
    const QString& DONT_CHANGE_SRC = QFileInfo(QFileInfo(__FILE__).absolutePath()).absoluteDir().absoluteFilePath("TestCase/test/TestEnv_ConflictsItem/DONT_CHANGE");
    const auto& conflictIF0 = ConflictsItemHelper(DONT_CHANGE_SRC, DONT_CHANGE_SRC, QDir(DONT_CHANGE_SRC, "", QDir::SortFlag::NoSort, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot).entryList());
    if (conflictIF0){
        qDebug(QString(conflictIF0.commonList.join('\n')).toStdString().c_str());
    }
    qDebug("\n=================\n");

    const auto& conflictIF1 = ConflictsItemHelper(DONT_CHANGE_SRC, DONT_CHANGE_SRC);
    if (conflictIF1){
        qDebug(QString(conflictIF1.commonList.join('\n')).toStdString().c_str());
    }
    qDebug("\n=================\n");

    QList<QFileInfo>fList = QDir(DONT_CHANGE_SRC, "", QDir::SortFlag::NoSort, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot).entryInfoList();
    QStringList lAbsPath;
    for (auto& fi: fList){
        lAbsPath.append(fi.absoluteFilePath());
    }
    const auto& conflictIF2 = ConflictsItemHelper(lAbsPath, DONT_CHANGE_SRC);
    if (conflictIF2){
        qDebug(QString(conflictIF2.commonList.join('\n')).toStdString().c_str());
    }
    qDebug("\n=================\n");
}

//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char *argv[]){
    TestMvConflictItemIF();
}
#endif
