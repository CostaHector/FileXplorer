#ifndef MYQFILESYSTEMMODEL_H
#define MYQFILESYSTEMMODEL_H

#include "Component/CustomStatusBar.h"
#include <QFileSystemModel>
#include <QMap>
#include <QObject>
#include <QSettings>

class MyQFileSystemModel : public QFileSystemModel {
public:
  explicit MyQFileSystemModel(CustomStatusBar *_statusBar = nullptr, QObject *parent = nullptr);

  Qt::ItemFlags flags(const QModelIndex &index) const override;

  bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
  bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
  Qt::DropActions supportedDropActions() const override;
  Qt::DropActions supportedDragActions() const override;

public slots:
  void whenRootPathChanged(const QString &newpath);
  void whenDirectoryLoaded(const QString &path);

protected:
  CustomStatusBar *logger;
  int m_imagesSizeLoaded = 0;

  static int previewsCnt;
  static constexpr int cacheWidth = 256;
  static constexpr int cacheHeight = 256;

  static constexpr int IMAGES_COUNT_LOAD_ONCE_MAX = 3;             // 10 pics
};

#endif // MYQFILESYSTEMMODEL_H
