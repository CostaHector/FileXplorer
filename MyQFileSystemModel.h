#ifndef MYQFILESYSTEMMODEL_H
#define MYQFILESYSTEMMODEL_H

#include <QFileSystemModel>
#include <QSettings>
#include <QMap>

class MyQFileSystemModel : public QFileSystemModel
{
public:
    explicit MyQFileSystemModel(QObject *parent = nullptr);

public slots:
    void whenRootPathChanged(const QString& newpath);

    void whenDirectoryLoaded(const QString& path);

    static QString SortOrderEnum2String(Qt::SortOrder orderEnum);
    static const QMap<QString, Qt::SortOrder> string2SortOrderEnumListTable;


protected:
    static QMap<QString, QPixmap> previews;
    static constexpr int cacheWidth = 256;
    static constexpr int cacheHeight = 256;
    int m_imagesSizeLoaded = 0;
    static constexpr int IMAGES_SIZE_LOADDABLE_MAX = 2*1024*1024; // 2MB
    static constexpr int IMAGES_COUNT_LOAD_ONCE_MAX = 10; // 10 pics
};

#endif // MYQFILESYSTEMMODEL_H
