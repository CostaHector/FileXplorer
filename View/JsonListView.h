#ifndef JSONLISTVIEW_H
#define JSONLISTVIEW_H

#include "Model/JsonModel.h"
#include "View/CustomListView.h"

#include <QListView>
#include <QMenu>

class JsonListView : public CustomListView {
 public:
  JsonListView(JsonModel* model_, QWidget* parent = nullptr);
  void subscribe();

  int currentRow() const { return currentIndex().row(); }
  void setCurrentRow(int row) { setCurrentIndex(m_jsonModel->index(row)); }
  int count() const { return m_jsonModel->rowCount(); };
  QString filePath(int row) const { return m_jsonModel->filePath(m_jsonModel->index(row)); }

  auto hasLast() const -> bool;
  auto hasNext() const -> bool;

  auto last() -> void;
  auto next() -> void;
  auto onSetPerfCount(const bool checked) -> void;
  void autoNext();

  auto onNext() -> void;
  auto onLast() -> void;

  int load(const QString& path);

 private:
  JsonModel* m_jsonModel;
  QMenu* m_jsonMenu;
};

#endif  // JSONLISTVIEW_H
