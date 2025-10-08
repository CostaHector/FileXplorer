#ifndef PREVIEWTYPETOOLBAR_H
#define PREVIEWTYPETOOLBAR_H

#include <QAction>
#include <QActionGroup>
#include <QToolBar>
#include "EnumIntAction.h"
#include "PreviewTypeTool.h"

extern template struct EnumIntAction<PreviewTypeTool::PREVIEW_TYPE_E>;

struct PreviewTypeToolBar : public QToolBar {
  Q_OBJECT
public:
  explicit PreviewTypeToolBar(const QString &title, QWidget *parent = nullptr);

  PreviewTypeTool::PREVIEW_TYPE_E mCurrentPreviewType{PreviewTypeTool::DEFULT_PREVIEW_TYPE_E};
  EnumIntAction<PreviewTypeTool::PREVIEW_TYPE_E> mPreviewTypeIntAction;

  QAction* CATEGORY_PRE{nullptr};
  QAction* PROGRESSIVE_LOAD_PRE{nullptr};
  QAction* CAROUSEL_PRE{nullptr};
signals:
  void previewTypeChanged(PreviewTypeTool::PREVIEW_TYPE_E previewType);
private:
  void subscribe();
  void onPreviewTypeActionToggled(QAction* pPreview);
};

#endif // PREVIEWTYPETOOLBAR_H
