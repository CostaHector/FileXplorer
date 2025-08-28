#include <QCoreApplication>
#include <QtTest>
#include "MyTestSuite.h"
#include "StringTool.h"
#include "TDir.h"
using namespace StringTool;

class StringToolTest : public MyTestSuite {
  Q_OBJECT
public:
  StringToolTest() : MyTestSuite{false} {}
private slots:
  void test_ImgsSortFileSizeFirst() {
    // both file not exist.
    QStringList inexistsImgs{"inexist B.jpg", "inexist A.jpeg"};
    StringTool::ImgsSortFileSizeFirst(inexistsImgs);
    QCOMPARE(inexistsImgs, (QStringList{"inexist A.jpeg", "inexist B.jpg"}));
    // TDir
    const QList<FsNodeEntry> entries{
        {"Chris Evans B.jpg", false, "1KB"},
        {"Chris Evans A.jpeg", false, "0"}, // file size less
    };
    TDir dir;
    QCOMPARE(dir.createEntries(entries), 2);
    QStringList existsImgs{"Chris Evans B.jpg", "Chris Evans A.jpeg"};
    StringTool::ImgsSortFileSizeFirst(existsImgs);
    QCOMPARE(existsImgs, (QStringList{"Chris Evans A.jpeg","Chris Evans B.jpg"}));
  }

  void test_ImgsSortNameLengthFirst() {
    // both file not exist.
    QStringList inexistsImgs{"inexist C.jpg", "inexist B.jpg", "inexist A.jpeg"};
    StringTool::ImgsSortNameLengthFirst(inexistsImgs);
    QCOMPARE(inexistsImgs, (QStringList{"inexist B.jpg","inexist C.jpg","inexist A.jpeg"}));
  }

  void test_RemoveDuplicateKeepSequence() {
    QStringList hists {"world","hello","world","hello"};
    QCOMPARE(StringTool::RemoveDuplicateKeepSequence(hists), -2);
    QCOMPARE(hists, (QStringList{"world", "hello"}));
  }

  void test_TrimEachElementAndRemoveEmpty() {
    QStringList keywords {
        "  \t",
        "",
        "Chris Hemsworth ",
        " Chris Hemsworth",
        "Chris Evans ",
        " Chris Evans",
        "\t "
    };
    QCOMPARE(TrimEachElementAndRemoveEmpty(keywords), 4-7);
    QCOMPARE(keywords, (QStringList{"Chris Hemsworth", "Chris Hemsworth", "Chris Evans", "Chris Evans"}));
  }

  void test_SearchHistoryListProc(){
    QStringList hists {
        "  \t",
        "",
        "Chris Hemsworth ",
        " Chris Hemsworth",
        "Chris Evans ",
        " Chris Evans",
        "\t "
    };
    SearchHistoryListProc(hists);
    QCOMPARE(hists, (QStringList{"Chris Hemsworth", "Chris Evans"}));
  }
};

#include "StringToolTest.moc"
StringToolTest g_StringToolTest;
