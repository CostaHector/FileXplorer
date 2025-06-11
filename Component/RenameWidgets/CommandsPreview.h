#ifndef COMMANDSPREVIEW_H
#define COMMANDSPREVIEW_H

#include <QPlainTextEdit>
#include <QToolButton>
#include <QAction>
class CommandsPreview : public QPlainTextEdit {
 public:
  explicit CommandsPreview(QWidget *parent = nullptr);
  void subscribe();
  void showEvent(QShowEvent* event) override;
  void closeEvent(QCloseEvent* event) override;
  void ReadSettings();

 protected:
  void resizeEvent(QResizeEvent *event) override;

 private:
  QAction* COPY_TEXT{nullptr};
  QToolButton* mToolButton{nullptr};
  void setupCopyButton();
  void adjustButtonPosition();
};

#endif // COMMANDSPREVIEW_H
