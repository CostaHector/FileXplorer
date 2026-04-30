#ifndef PREVIEWDOCKWIDGET_H
#define PREVIEWDOCKWIDGET_H

#include <QDockWidget>
#include "EnumIntAction.h"
#include "PreviewTypeTool.h"
#include "ToolBarWidget.h"
#include <QLabel>

extern template struct EnumIntAction<PreviewTypeTool::PREVIEW_TYPE_E>;

class PreviewDockWidget : public QDockWidget {
  Q_OBJECT
 public:
  explicit PreviewDockWidget(const QString& title, QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
  ~PreviewDockWidget();

  PreviewTypeTool::PREVIEW_TYPE_E GetCurrentPreviewType() const;

 signals:
  void previewTypeChanged(PreviewTypeTool::PREVIEW_TYPE_E previewType);

 public slots:
  void onWindowsTitleChanged(const QString& newWindowsTitle);
  void setVisible(bool visible) override;

 protected:
  void showEvent(QShowEvent* event) override;

 private:
  void subscribe();
  void onPreviewTypeActionToggled(QAction* pPreview);

  QLabel* m_windowsTitleLabel{nullptr};

  QAction* CATEGORY_PRE{nullptr};
  QAction* PROGRESSIVE_LOAD_PRE{nullptr};
  QAction* CAROUSEL_PRE{nullptr};
  QAction* m_minimizePanel{nullptr};
  QAction* m_floatingPanel{nullptr};

  ToolBarWidget* m_titleBar{nullptr};
  EnumIntAction<PreviewTypeTool::PREVIEW_TYPE_E> mPreviewTypeIntAction;
};
#endif  // PREVIEWDOCKWIDGET_H
