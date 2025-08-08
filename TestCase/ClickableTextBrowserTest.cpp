#include <QCoreApplication>
#include <QtTest>
#include "Component/FolderPreview/ClickableTextBrowser.h"
#include "pub/MyTestSuite.h"

class ClickableTextBrowserTest : public MyTestSuite {
  Q_OBJECT
public:
  ClickableTextBrowserTest() : MyTestSuite{true} {}
private slots:
  void test_search_a_and_b_ok() {
    QCOMPARE(ClickableTextBrowser::GetStandardSearchKeyWords("A and B"), "%A%B%");
  }

  void test_search_a_comma_space_b_ok() {
    QCOMPARE(ClickableTextBrowser::GetStandardSearchKeyWords("A, B"), "%A%B%");
  }
};

#include "ClickableTextBrowserTest.moc"
ClickableTextBrowserTest g_ClickableTextBrowserTest;
