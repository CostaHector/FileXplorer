#include "RenameWidget.h"

const QString RenameWidget::INVALID_CHARS("*?\"<>|");
const QSet<QChar> RenameWidget::INVALID_FILE_NAME_CHAR_SET(INVALID_CHARS.cbegin(), INVALID_CHARS.cend());



#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char *argv[]){
    QDir dir(QFileInfo(QFileInfo(__FILE__).absolutePath()).absolutePath());
    QStringList rels  = dir.entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::DirsFirst);

    QApplication a(argc, argv);
    RenameWidget_Insert winI;
    winI.init();
    winI.InitTextContent(dir.absolutePath(), rels);
    // winR.InitTextContent(pre, rels)
    // winD.InitTextContent(pre, rels)
    // winN.InitTextContent(pre, rels)
    winI.show();
    return a.exec();
}
#endif
