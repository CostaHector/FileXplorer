#ifndef PROPERTIESWINDOW_H
#define PROPERTIESWINDOW_H

#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QToolBar>
class PropertiesWindow : public QDialog {
  Q_OBJECT
 public:
  explicit PropertiesWindow(const QStringList& items, QWidget* parent = nullptr);
  QSize sizeHint() const { return QSize(600, 400); }
 protected:
  bool UpdateMessage();
  bool operator()(const QStringList& items);

 private:
  void subscribe();

  QStringList m_items;
  QPlainTextEdit* m_propertiesInfoTextEdit;
  QDialogButtonBox* m_buttonBox;
  QAction* m_showFilesSize;
  QAction* m_showVidsDuration;
  QAction* m_showMD5;
  QToolBar* m_extraToolbar;
};

#endif  // PROPERTIESWINDOW_H
