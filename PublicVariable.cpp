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

namespace MemoryKey {
GVarStrFile BACKGROUND_IMAGE("");
GVarBool SHOW_BACKGOUND_IMAGE(false);
GVarStrFolder PATH_LAST_TIME_COPY_TO("");
GVarBool SHOW_FOLDER_PREVIEW_HTML(true);
GVarBool SHOW_FOLDER_PREVIEW_WIDGET(true);
GVarBool SHOW_FOLDER_PREVIEW_IMAGE(false);
GVarBool SHOW_FOLDER_PREVIEW_JSON_EDITOR(false);
}

