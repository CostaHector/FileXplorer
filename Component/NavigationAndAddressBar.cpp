#include "NavigationAndAddressBar.h"
#include "Actions/AddressBarActions.h"

#include <QHBoxLayout>

NavigationAndAddressBar::NavigationAndAddressBar(const QString& title, QWidget* parent)
    : QToolBar(title, parent),
      _addressLine(new AddressELineEdit),
      searchLE(new QLineEdit),
      m_IntoNewPath(nullptr),
      m_on_searchTextChanged(nullptr),
      m_on_searchEnterKey(nullptr) {
  _addressLine->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

  searchLE->addAction(QIcon(":/themes/SEARCH"), QLineEdit::LeadingPosition);
  searchLE->setClearButtonEnabled(true);
  searchLE->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);

  addActions(g_addressBarActions().ADDRESS_CONTROLS->actions());
  addSeparator();
  addWidget(_addressLine);
  addSeparator();
  addWidget(searchLE);

  setFixedHeight(CONTROL_TOOLBAR_HEIGHT);
  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);
  InitEventWhenViewChanged();
}

void NavigationAndAddressBar::subscribe(T_IntoNewPath IntoNewPath,
                                        T_on_searchTextChanged on_searchTextChanged,
                                        T_on_searchEnterKey on_searchEnterKey) {
  m_IntoNewPath = IntoNewPath;
  m_on_searchTextChanged = on_searchTextChanged;
  m_on_searchEnterKey = on_searchEnterKey;
}

auto NavigationAndAddressBar::InitEventWhenViewChanged() -> void {
  connect(g_addressBarActions()._BACK_TO, &QAction::triggered, this, &NavigationAndAddressBar::onBackward);
  connect(g_addressBarActions()._FORWARD_TO, &QAction::triggered, this, &NavigationAndAddressBar::onForward);
  connect(g_addressBarActions()._UP_TO, &QAction::triggered, this, &NavigationAndAddressBar::onUpTo);

  connect(searchLE, &QLineEdit::textChanged, this, [this]() -> void {
    if (m_on_searchTextChanged)
      m_on_searchTextChanged(searchLE->text());
  });
  connect(searchLE, &QLineEdit::returnPressed, this, [this]() -> void {
    if (m_on_searchEnterKey)
      m_on_searchEnterKey(searchLE->text());
  });
}

auto NavigationAndAddressBar::onBackward() -> bool {
  if (m_IntoNewPath and pathRD.undoAvailable()) {
    return m_IntoNewPath(pathRD.undo(), false, false);
  }
  qDebug("[Skip] backward paths pool empty");
  return true;
}

auto NavigationAndAddressBar::onForward() -> bool {
  if (m_IntoNewPath and pathRD.redoAvailable()) {
    return m_IntoNewPath(pathRD.redo(), false, false);
  }
  qDebug("[Skip] Forward paths pool empty");
  return true;
}

auto NavigationAndAddressBar::onUpTo() -> bool {
  if (m_IntoNewPath) {
    return m_IntoNewPath(_addressLine->dirname(), true, false);
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
