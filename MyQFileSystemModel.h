#ifndef MYQFILESYSTEMMODEL_H
#define MYQFILESYSTEMMODEL_H

#include <QFileSystemModel>
#include <QSettings>
#include <QMap>
#include <QObject>

class MyQFileSystemModel : public QFileSystemModel {
public:
    explicit MyQFileSystemModel(QObject *parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex &index) const override;

public slots:
    void whenRootPathChanged(const QString& newpath);
    void whenDirectoryLoaded(const QString& path);


protected:
    static QMap<QString, QPixmap> previews;
    static constexpr int cacheWidth = 256;
    static constexpr int cacheHeight = 256;
    int m_imagesSizeLoaded = 0;
    static constexpr int IMAGES_SIZE_LOADDABLE_MAX = 2*1024*1024; // 2MB
    static constexpr int IMAGES_COUNT_LOAD_ONCE_MAX = 10; // 10 pics
};

#endif // MYQFILESYSTEMMODEL_H
