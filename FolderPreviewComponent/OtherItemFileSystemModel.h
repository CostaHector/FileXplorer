#ifndef OTHERITEMFILESYSTEMMODEL_H
#define OTHERITEMFILESYSTEMMODEL_H

#include <MyQFileSystemModel.h>

class OtherItemFileSystemModel : public MyQFileSystemModel
{
public:
    explicit OtherItemFileSystemModel(QObject *parent = nullptr, bool showThumbnails_=false);
};

#endif // OTHERITEMFILESYSTEMMODEL_H
