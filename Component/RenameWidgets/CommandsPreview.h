#ifndef COMMANDSPREVIEW_H
#define COMMANDSPREVIEW_H

#include <QTextBrowser>
#include <QToolBar>
#include <QAction>

class CommandsPreview : public QTextBrowser {
public:
  explicit CommandsPreview(const QString& name, QWidget *parent = nullptr);
  void subscribe();
  void showEvent(QShowEvent* event) override;
  void closeEvent(QCloseEvent* event) override;
  void ReadSettings();

protected:
  void resizeEvent(QResizeEvent *event) override;
  QAction* STAY_ON_TOP{nullptr};

private:
  QString mName;
  QAction* COPY_TEXT{nullptr};
  QToolBar* mToolBar{nullptr};
  void adjustButtonPosition();
};

#endif // COMMANDSPREVIEW_H
