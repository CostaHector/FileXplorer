#include "OtherItemFileSystemModel.h"

OtherItemFileSystemModel::OtherItemFileSystemModel(QObject *parent, bool showThumbnails_) :
    MyQFileSystemModel(parent)
{
    setNameFilters({"*.html", "*.json", "*.txt", "*.md"});
}
