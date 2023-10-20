#include "OtherItemFileSystemModel.h"

OtherItemFileSystemModel::OtherItemFileSystemModel(QObject *parent, bool showThumbnails_) :
    MyQFileSystemModel(nullptr, parent)
{
    setNameFilters({"*.html", "*.json", "*.txt", "*.md"});
}
