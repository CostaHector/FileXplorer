#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "StringTool.h"
#include "TDir.h"
using namespace StringTool;

class StringToolTest : public PlainTestSuite {
  Q_OBJECT
public:
  using PlainTestSuite::PlainTestSuite;

private slots:
  void ImgsSortFileSizeFirst_ok() {
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

  void ImgsSortNameLengthFirst_ok() {
    // both file not exist.
    QStringList inexistsImgs{"inexist C.jpg", "inexist B.jpg", "inexist A.jpeg"};
    StringTool::ImgsSortNameLengthFirst(inexistsImgs);
    QCOMPARE(inexistsImgs, (QStringList{"inexist B.jpg","inexist C.jpg","inexist A.jpeg"}));
  }

  void ImgsSortByNumber_ok() {
    // 0. empty list
    {
      QStringList imgs;
      StringTool::ImgsSortByNumber(imgs);
      QVERIFY(imgs.isEmpty());
    }

    // 1. 基本：乱序编号 → 升序，_tn 最后
    {
      QStringList imgs{"A 1.jpg", "A 2.jpg", "A 3.jpg", "A 20.jpg", "A 0.jpeg", "A_tn.jpg"};
      StringTool::ImgsSortByNumber(imgs);
      QCOMPARE(imgs, QStringList({"A 0.jpeg", "A 1.jpg", "A 2.jpg", "A 3.jpg", "A 20.jpg", "A_tn.jpg"}));
    }

    // 2. 编号按数值比，不是字典序：1 < 2 < 3 < 10 < 20
    {
      QStringList imgs{"A 10.jpg", "A 2.jpg", "A 20.jpg", "A 1.jpg", "A 3.jpg"};
      StringTool::ImgsSortByNumber(imgs);
      QCOMPARE(imgs, QStringList({"A 1.jpg", "A 2.jpg", "A 3.jpg", "A 10.jpg", "A 20.jpg"}));
    }

    // 3. 只有 _tn，顺序不变
    {
      QStringList imgs{"A_tn.jpg"};
      StringTool::ImgsSortByNumber(imgs);
      QCOMPARE(imgs, QStringList({"A_tn.jpg"}));
    }


    // 4. 多个 _tn（理论上不会出现，但保证都落最后且相对顺序稳定）
    {
      QStringList imgs{"A_tn.jpg", "A 2.jpg", "A_tn.png", "A 1.jpg"};
      StringTool::ImgsSortByNumber(imgs);
      QCOMPARE(imgs, QStringList({"A 1.jpg", "A 2.jpg", "A_tn.jpg", "A_tn.png"}));
    }

    // 5. 同编号不同扩展名：排序稳定，保持原顺序(相同的元素, 相对位置不变)
    {
      QStringList imgs{"A 1.png", "A 1.jpg"};
      StringTool::ImgsSortByNumber(imgs);
      QCOMPARE(imgs, QStringList({"A 1.png", "A 1.jpg"}));
    }

    // 6. 非纯数字但以数字结尾（A a1.jpg）：toInt 失败，归到末尾
    {
      QStringList imgs{"A a1.jpg", "A 2.jpg", "A 1.jpg"};
      StringTool::ImgsSortByNumber(imgs);
      QCOMPARE(imgs, QStringList({"A 1.jpg", "A 2.jpg", "A a1.jpg"}));
    }

    // 7. 没有扩展名的文件：dotPos < 0，归到末尾
    {
      QStringList imgs{"A 2.jpg", "A_noext", "A 1.jpg"};
      StringTool::ImgsSortByNumber(imgs);
      QCOMPARE(imgs, QStringList({"A 1.jpg", "A 2.jpg", "A_noext"}));
    }

    // 8. 混入无关文件（cover.jpg）：无空格，归到末尾
    {
      QStringList imgs{"cover.jpg", "A 2.jpg", "A 1.jpg"};
      StringTool::ImgsSortByNumber(imgs);
      QCOMPARE(imgs, QStringList({"A 1.jpg", "A 2.jpg", "cover.jpg"}));
    }
  }

  void ImgsMinByNumber_ok() {
    // 0. empty list
    {
      QStringList imgs;
      QVERIFY(StringTool::ImgsMinByNumber(imgs).isEmpty());
    }

    // 1. 基本：多张数字图，返回编号最小的
    {
      QStringList imgs{"A 20.jpg", "A 1.jpg", "A 3.jpg", "A 0.jpeg"};
      QCOMPARE(StringTool::ImgsMinByNumber(imgs), QString("A 0.jpeg"));
    }

    // 2. 编号 0 最小
    {
      QStringList imgs{"A 1.jpg", "A 0.jpg"};
      QCOMPARE(StringTool::ImgsMinByNumber(imgs), QString("A 0.jpg"));
    }

    // 3. 编号按数值比，不是字典序：A 2 比 A 10 小
    {
      QStringList imgs{"A 10.jpg", "A 2.jpg", "A 20.jpg"};
      QCOMPARE(StringTool::ImgsMinByNumber(imgs), QString("A 2.jpg"));
    }

    // 4. 只有 _tn：返回 _tn 兜底
    {
      QStringList imgs{"A_tn.jpg"};
      QCOMPARE(StringTool::ImgsMinByNumber(imgs), QString("A_tn.jpg"));
    }

    // 5. _tn 和数字图混合：数字图优先
    {
      QStringList imgs{"A_tn.jpg", "A 5.jpg"};
      QCOMPARE(StringTool::ImgsMinByNumber(imgs), QString("A 5.jpg"));
    }

    // 9. 非纯数字但以数字结尾（A a1.jpg）不参与比较
    {
      QStringList imgs{"A 5.jpg", "A a1.jpg"};
      QCOMPARE(StringTool::ImgsMinByNumber(imgs), QString("A 5.jpg"));
    }

    // 7. 只有 A a1.jpg（toInt 失败）：兜底返回第一张
    {
      QStringList imgs{"A a1.jpg"};
      QCOMPARE(StringTool::ImgsMinByNumber(imgs), QString("A a1.jpg"));
    }

    // 8. 同编号： 返回先遇到的那个 稳定
    {
      QStringList imgs{"A 1.png", "A 1.jpg"};
      QCOMPARE(StringTool::ImgsMinByNumber(imgs), QString("A 1.png"));
    }

    // 9. 混入 cover.jpg（无空格）：数字图优先
    {
      QStringList imgs{"cover.jpg", "A 5.jpg"};
      QCOMPARE(StringTool::ImgsMinByNumber(imgs), QString("A 5.jpg"));
    }
  }

  void RemoveDuplicateKeepSequence_ok() {
    QStringList hists {"world","hello","world","hello"};
    QCOMPARE(StringTool::RemoveDuplicateKeepSequence(hists), -2);
    QCOMPARE(hists, (QStringList{"world", "hello"}));
  }

  void TrimEachElementAndRemoveEmpty_ok() {
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

  void SearchHistoryListProc_ok(){
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

  void PathJoinPixmapSize_ok() {
    QCOMPARE(PathJoinPixmapSize("a.jpg", 64, 128, true), "a.jpg_64x128_1");
    QCOMPARE(PathJoinPixmapSize("b.jpg", 128, 64, false), "b.jpg_128x64_0");
  }

  void EscapeCsv_ok() {
    QCOMPARE(EscapeCsv("Hello world"), QByteArray("Hello world"));
    // comma, \n, "
    QCOMPARE(EscapeCsv("Hello, world"), QByteArray(R"("Hello, world")"));
    QCOMPARE(EscapeCsv("Hello\nWorld"), QByteArray("\"Hello\nWorld\""));
    QCOMPARE(EscapeCsv("\"Fancy\" meeting you here"), QByteArray(R"("""Fancy"" meeting you here")"));
  }
};

#include "StringToolTest.moc"
REGISTER_TEST(StringToolTest, false)
