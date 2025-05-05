#ifndef RIBBONMOVIEDB_H
#define RIBBONMOVIEDB_H

#include <QToolBar>

class RibbonMovieDB : public QToolBar {
 public:
  explicit RibbonMovieDB(const QString& title, QWidget* parent = nullptr);
  QToolBar* m_dbControlTB{nullptr};
  QToolBar* m_extraFunctionTB{nullptr};
  QToolBar* m_functionsTB{nullptr};
  QToolBar* m_dbViewHideShowTB{nullptr};
};

#endif  // RIBBONMOVIEDB_H
