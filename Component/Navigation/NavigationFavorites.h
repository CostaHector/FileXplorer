#ifndef NAVIGATIONFAVORITES_H
#define NAVIGATIONFAVORITES_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include "FavoritesTreeView.h"

class NavigationFavorites : public QWidget {
 public:
  explicit NavigationFavorites(QWidget* parent = nullptr);
  const FavoritesTreeView* view() const { return m_favView; }

 private:
  QSize sizeHint() const override {
     return {QWidget::sizeHint().width(), 400};
  }
  void onStartFilter();

  QLineEdit* m_searchLineEdit{nullptr};
  FavoritesTreeView* m_favView{nullptr};
  QVBoxLayout* m_layout{nullptr};

  QAction* m_startSearchAct{nullptr};
};

#endif  // NAVIGATIONFAVORITES_H
