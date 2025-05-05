#ifndef DATABASETOOLBAR_H
#define DATABASETOOLBAR_H

#include <QToolBar>

class DatabaseToolBar : public QToolBar {
 public:
  explicit DatabaseToolBar(const QString& title, QWidget* parent = nullptr);
  QToolBar* m_dbControlTB{nullptr};
  QToolBar* m_extraFunctionTB{nullptr};
  QToolBar* m_functionsTB{nullptr};
  QToolBar* m_dbViewHideShowTB{nullptr};
};

#endif  // DATABASETOOLBAR_H
