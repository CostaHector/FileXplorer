#ifndef JSONVIEW_H
#define JSONVIEW_H

#include "CustomTableView.h"
#include "Model/JsonTableModel.h"
class JsonTableView: public CustomTableView
{
 public:
  explicit JsonTableView(QWidget* parent = nullptr);
  int ReadADirectory(const QString& path);
  int AppendADirectory(const QString& path);
 private:
  JsonTableModel* mJsonModel{nullptr};
};

#endif // JSONVIEW_H
