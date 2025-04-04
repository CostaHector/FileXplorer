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
  void onOrientationChange(const QAction* pOrientation);

  void InitListView();

 protected:
  QString m_name;
  QAction *_ORIENTATION_LEFT_TO_RIGHT{nullptr}, *_ORIENTATION_TOP_TO_BOTTOM{nullptr};
  QActionGroup* _ORIENTATION_GRP{nullptr};

 private:
  QMenu* m_menu = nullptr;

  inline bool isNameExists(const QString& name) const { return LISTS_SET.contains(name); }
  static QSet<QString> LISTS_SET;
};

#endif  // CUSTOMLISTVIEW_H
