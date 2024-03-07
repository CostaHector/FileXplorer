#include "NavigationAndAddressBar.h"
#include "Actions/AddressBarActions.h"

#include <QHBoxLayout>

NavigationAndAddressBar::NavigationAndAddressBar(const QString& title, QWidget* parent)
    : QToolBar(title, parent),
      m_addressLine(new AddressELineEdit{this}),
      m_searchLE(new QLineEdit{this}),
      m_fsFilter{new FileSystemTypeFilter},
      m_IntoNewPath(nullptr),
      m_on_searchTextChanged(nullptr),
      m_on_searchEnterKey(nullptr) {
  m_addressLine->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  m_addressLine->setFixedHeight(CONTROL_TOOLBAR_HEIGHT);

  m_searchLE->addAction(QIcon(":/themes/SEARCH"), QLineEdit::LeadingPosition);
  m_searchLE->setClearButtonEnabled(true);
  m_searchLE->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
  m_searchLE->setFixedHeight(CONTROL_TOOLBAR_HEIGHT);

  m_fsFilter->setFixedHeight(CONTROL_TOOLBAR_HEIGHT);

  addActions(g_addressBarActions().ADDRESS_CONTROLS->actions());
  addSeparator();
  addWidget(m_addressLine);
  addSeparator();
  addWidget(m_fsFilter);
  addSeparator();
  addWidget(m_searchLE);

  setFixedHeight(CONTROL_TOOLBAR_HEIGHT);
  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);
  InitEventWhenViewChanged();
}

void NavigationAndAddressBar::BindFileSystemViewCallback(T_IntoNewPath IntoNewPath,
                                                         T_on_searchTextChanged on_searchTextChanged,
                                                         T_on_searchEnterKey on_searchEnterKey,
                                                         QFileSystemModel* _fsm) {
  m_IntoNewPath = IntoNewPath;
  m_on_searchTextChanged = on_searchTextChanged;
  m_on_searchEnterKey = on_searchEnterKey;
  m_fsFilter->BindFileSystemModel(_fsm);
}

auto NavigationAndAddressBar::InitEventWhenViewChanged() -> void {
  connect(g_addressBarActions()._BACK_TO, &QAction::triggered, this, &NavigationAndAddressBar::onBackward);
  connect(g_addressBarActions()._FORWARD_TO, &QAction::triggered, this, &NavigationAndAddressBar::onForward);
  connect(g_addressBarActions()._UP_TO, &QAction::triggered, this, &NavigationAndAddressBar::onUpTo);

  connect(m_searchLE, &QLineEdit::textChanged, this, [this]() -> void {
    if (m_on_searchTextChanged)
      m_on_searchTextChanged(m_searchLE->text());
  });
  connect(m_searchLE, &QLineEdit::returnPressed, this, [this]() -> void {
    if (m_on_searchEnterKey)
      m_on_searchEnterKey(m_searchLE->text());
  });
}

auto NavigationAndAddressBar::onBackward() -> bool {
  if (m_IntoNewPath and m_pathRD.undoAvailable()) {
    return m_IntoNewPath(m_pathRD.undo(), false, false);
  }
  qDebug("[Skip] backward paths pool empty");
  return true;
}

auto NavigationAndAddressBar::onForward() -> bool {
  if (m_IntoNewPath and m_pathRD.redoAvailable()) {
    return m_IntoNewPath(m_pathRD.redo(), false, false);
  }
  qDebug("[Skip] Forward paths pool empty");
  return true;
}

auto NavigationAndAddressBar::onUpTo() -> bool {
  if (m_IntoNewPath) {
    return m_IntoNewPath(m_addressLine->dirname(), true, false);
  }
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

class IntoNewPathMockClass {
 public:
  bool IntoNewPath(QString a, bool b, bool c) {
    qDebug("IntoNewPath: %s, %d, %d", qPrintable(a), b, c);
    return true;
  }
  bool on_searchTextChanged(QString a) {
    qDebug("on_searchTextChanged: %s, %d", qPrintable(a));
    return true;
  }
  bool on_searchEnterKey(QString a) {
    qDebug("on_searchEnterKey: %s, %d", qPrintable(a));
    return true;
  }
};

int main(int argc, char* argv[]) {
  using std::placeholders::_1;
  using std::placeholders::_2;
  using std::placeholders::_3;

  QApplication a(argc, argv);

  IntoNewPathMockClass printer;
  auto f1 = std::bind(&IntoNewPathMockClass::IntoNewPath, &printer, _1, _2, _3);
  auto f2 = std::bind(&IntoNewPathMockClass::on_searchTextChanged, &printer, _1);
  auto f3 = std::bind(&IntoNewPathMockClass::on_searchEnterKey, &printer, _1);

  NavigationAndAddressBar lo(f1, f2, f3);

  QWidget w;
  w.setLayour(lo);

  w.show();
  w.winAddress->UpdatePath(QFileInfo(__FILE__).absolutePath());
  return a.exec();
  ;
}
#endif
