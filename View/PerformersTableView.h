#ifndef PERFORMERSTABLEVIEW_H
#define PERFORMERSTABLEVIEW_H

#include <QContextMenuEvent>
#include <QMenu>
#include <QTableView>
#include "CustomTableView.h"

class PerformersTableView : public CustomTableView {
 public:
  explicit PerformersTableView(QWidget* parent = nullptr);
};

#endif  // PERFORMERSTABLEVIEW_H
