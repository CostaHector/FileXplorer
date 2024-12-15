#ifndef CUSTOMLISTVIEW_H
#define CUSTOMLISTVIEW_H

#include <QAction>
#include <QListView>
#include <QMenu>

class CustomListView : public QListView {
 public:
  explicit CustomListView(const QString& name, QWidget* parent = nullptr);

  virtual void contextMenuEvent(QContextMenuEvent* event) override;

  void BindMenu(QMenu* menu);

  void InitListView();

 private:
  QString m_name;

  QMenu* m_menu = nullptr;

  inline bool isNameExists(const QString& name) const { return LISTS_SET.contains(name); }
  static QSet<QString> LISTS_SET;
};

#endif  // CUSTOMLISTVIEW_H
