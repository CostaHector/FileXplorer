#include "ImagesFileSystemModel.h"
#include "PublicVariable.h"

ImagesFileSystemModel::ImagesFileSystemModel(QObject* parent, bool showThumbnails_)
    : MyQFileSystemModel(parent), m_showThumbnails(showThumbnails_), m_iconProvider{nullptr} {
  setFilter(QDir::Drives|QDir::Files);
  setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
  setNameFilterDisables(false);

  if (m_showThumbnails) {
    if (m_iconProvider == nullptr) {
      m_iconProvider = new ImgIconProvider;
    }
    setIconProvider(m_iconProvider);
  }
}
