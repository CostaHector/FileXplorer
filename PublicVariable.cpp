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

namespace HEADERVIEW_SORT_INDICATOR_ORDER{
QString SortOrderEnum2String(const Qt::SortOrder orderEnum){
    if (string2SortOrderEnumListTable.isEmpty()){
        qDebug("[Error] string2SortOrderListTable is empty");
        return "";
    }
    for (auto it=string2SortOrderEnumListTable.cbegin();it!=string2SortOrderEnumListTable.cend();++it){
        if (it.value() == orderEnum){
            return it.key();
        }
    }
    return string2SortOrderEnumListTable.cbegin().key();
}
}

