#ifndef RIBBONJSON_H
#define RIBBONJSON_H

#include <QToolBar>
#include <QString>
#include "MenuToolButton.h"
class RibbonJson : public QToolBar {
  Q_OBJECT
 public:
  explicit RibbonJson(const QString& title = "Json Ribbons", QWidget* parent = nullptr);

 private:
  QToolBar* mSyncCacheFileSystemTb{nullptr};

  QToolBar* mFieldCaseOperTb{nullptr};

  QToolBar* mInitFormatStudioCastField{nullptr};
  QToolBar* mInferStudioCastFromSelection{nullptr};

  MenuToolButton* mStudioMenu{nullptr};
  MenuToolButton* mCastMenu{nullptr};
  MenuToolButton* mTagsMenu{nullptr};

  QToolBar* mUpdateFieldTb{nullptr};
};

#endif  // RIBBONJSON_H
