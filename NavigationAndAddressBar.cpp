#include "NavigationAndAddressBar.h"

#include <QHBoxLayout>
#include <functional>
NavigationAndAddressBar::NavigationAndAddressBar(T_IntoNewPath IntoNewPath,
                                                 T_on_searchTextChanged on_searchTextChanged,
                                                 T_on_searchEnterKey on_searchEnterKey)
    : winAddress(new AddressELineEdit),
      backToBtn(new QPushButton(QIcon(":/themes/PATH_BACK_TO"), "")),
      forwardToBtn(new QPushButton(QIcon(":/themes/PATH_FORWARD_TO"), "")),
      searchLE(new QLineEdit),
      upToBtn(new QPushButton(QIcon(":/themes/PATH_UP_TO"), "")),
      m_IntoNewPath(IntoNewPath),
      m_on_searchTextChanged(on_searchTextChanged),
      m_on_searchEnterKey(on_searchEnterKey)
{
  backToBtn->setStyleSheet(
      "border-left: 1px solid gray;"
      "border-right: 1px dotted gray;"
      "border-top: 1px solid gray;"
      "border-bottom: 1px solid gray;");
  forwardToBtn->setStyleSheet(
      "border-right: 1px dotted gray;"
      "border-top: 1px solid gray;"
      "border-bottom: 1px solid gray;");
  upToBtn->setStyleSheet(
      "border-right: 1px solid gray;"
      "border-top: 1px solid gray;"
      "border-bottom: 1px solid gray;");
  backToBtn->setFixedHeight(CONTROL_HEIGHT);
  forwardToBtn->setFixedHeight(CONTROL_HEIGHT);
  upToBtn->setFixedHeight(CONTROL_HEIGHT);
  backToBtn->setFlat(true);
  forwardToBtn->setFlat(true);
  upToBtn->setFlat(true);

  searchLE->setFixedHeight(CONTROL_HEIGHT);
  searchLE->setClearButtonEnabled(false);
  searchLE->addAction(QIcon(":/themes/SEARCH"), QLineEdit::LeadingPosition);

  addWidget(backToBtn);
  setStretch(count() - 1, 1);
  addWidget(forwardToBtn);
  setStretch(count() - 1, 1);
  addWidget(upToBtn);
  setStretch(count() - 1, 2);
  addWidget(winAddress);
  setStretch(count() - 1, 16);
  addWidget(searchLE);
  setStretch(count() - 1, 4);

  InitEventWhenViewChanged();

  setContentsMargins(0, 0, 0, 0);
  setSpacing(0);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

class IntoNewPathMockClass {
 public:
  bool IntoNewPath(QString a, bool b, bool c) {
    qDebug("IntoNewPath: %s, %d, %d", a.toStdString().c_str(), b, c);
    return true;
  }
  bool on_searchTextChanged(QString a) {
    qDebug("on_searchTextChanged: %s, %d", a.toStdString().c_str());
    return true;
  }
  bool on_searchEnterKey(QString a) {
    qDebug("on_searchEnterKey: %s, %d", a.toStdString().c_str());
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
