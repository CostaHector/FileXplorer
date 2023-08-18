#include "PublicVariable.h"
auto TextReader(const QString& textPath) -> QString{
    if (not QFileInfo(textPath).isFile()){
        qDebug("TextReader [%s] not exists", textPath.toStdString().c_str());
        return "";
    }
    QFile file(textPath);
    if (not file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return "";
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString contents(stream.readAll());
    file.close();
    return contents;
}
