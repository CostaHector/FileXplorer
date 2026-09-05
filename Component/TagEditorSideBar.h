#ifndef TAGEDITORSIDEBAR_H
#define TAGEDITORSIDEBAR_H
#include <QToolBar>
#include "ActionsContainerWid.h"

class TagEditorSideBar : public QToolBar {
public:
  static TagEditorSideBar* GInstance(TagEditorSideBar* pInitValue=nullptr, bool bRelease=false);
  explicit TagEditorSideBar(const QString &title = "TagEditor SideBar", QWidget *parent = nullptr);
private:
  ActionsContainerWid* m_tagEditor{nullptr};
};

#endif // TAGEDITORSIDEBAR_H
