#include <QCoreApplication>
#include <QtTest>
#include "TestCase/pubTestTool/MyTestSuite.h"

QStringList NeedProcessPath(const QStringList& paths) {
  if (paths.size() < 2) {
    return paths;
  }
  QStringList input{paths};
  input.sort();
  QStringList output;
  output.reserve(input.size());
  output.append(input[0]);
  // a[i] may be parent path
  for (int i = 0, j = i + 1; j < input.size(); ++j) {
    if (input[j].startsWith(input[i] + '/')) {
      // j is sub of i
      continue;
    }
    output.append(input[j]);
    i = j;
  }
  return output;
}

class SelectedItemsTest : public MyTestSuite {
  Q_OBJECT
 public:
  SelectedItemsTest() : MyTestSuite{false} {}
 private slots:
  void test_zero_or_1_element_only() {
    QCOMPARE(NeedProcessPath({}), (QStringList{}));
    QCOMPARE(NeedProcessPath({"C:/"}), (QStringList{"C:/"}));
    QCOMPARE(NeedProcessPath({"C:/file.txt"}), (QStringList{"C:/file.txt"}));
  }

  void test_all_need_process() {
    QCOMPARE(NeedProcessPath({"C:/", "D:/", "E:/"}),  //
             (QStringList{"C:/", "D:/", "E:/"}));
    QCOMPARE(NeedProcessPath({"C:/path1", "C:/path2", "C:/path3"}),  //
             (QStringList{"C:/path1", "C:/path2", "C:/path3"}));
    // "C:/path123" NOT start with "C:/path1/"
    QCOMPARE(NeedProcessPath({"C:/path1", "C:/path12", "C:/path123"}),  //
             (QStringList{"C:/path1", "C:/path12", "C:/path123"}));
  }
  void test_only_need_process_parent_path() {
    QCOMPARE(NeedProcessPath({"C:/path1", "C:/path1/file", "C:/path1/folder"}),  //
             (QStringList{"C:/path1"}));
    QCOMPARE(NeedProcessPath({"C:/path1/folder", "C:/path1/file", "C:/path1"}),  //
             (QStringList{"C:/path1"}));
  }
  void test_complexity_path() {
    QCOMPARE(NeedProcessPath({"C:/path1/folder", "C:/path1/file", "C:/path1", "C:/path2"}),  //
             (QStringList{"C:/path1", "C:/path2"}));
  }
};

SelectedItemsTest g_SelectedItemsTest;
#include "SelectedItemsTest.moc"
