#ifndef LOGFLOATINGPREVIEWER_H
#define LOGFLOATINGPREVIEWER_H
#include <QToolButton>
#include <QSize>
#include "CommandsPreview.h"

class LogFloatingPreviewer : public CommandsPreview {
public:
  explicit LogFloatingPreviewer(const QString& name, QWidget* parent = nullptr);

  void BindToolButton(QToolButton* tb);
  QSize sizeHint() const override {
    return {SIZE_WIDTH, SIZE_HEIGHT};
  }

protected:
  void keyPressEvent(QKeyEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void UpdateLogsContents(const int maxLines = 100, bool bMoveToEnd=true);
  void onHideShow(bool checked);
private:
  void MovePosition();

  QToolButton* _logPreviewTb {nullptr};
  static constexpr int SIZE_WIDTH{1024};
  static constexpr int SIZE_HEIGHT{600};
};

#endif // LOGFLOATINGPREVIEWER_H
