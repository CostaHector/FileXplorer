#ifndef SEARCHMODECOMBOBOX_H
#define SEARCHMODECOMBOBOX_H

#include <QComboBox>

#include "Tools/SearchProxyModel.h"

class SearchModeComboBox : public QComboBox {
 public:
  explicit SearchModeComboBox(QWidget *parent = nullptr);
  void BindSearchModel(SearchProxyModel* searchProxyModel);
 private:
  SearchProxyModel* _searchProxyModel = nullptr;
};

#endif // SEARCHMODECOMBOBOX_H
