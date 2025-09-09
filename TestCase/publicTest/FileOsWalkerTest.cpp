#include <QtTest>
#include <QCoreApplication>
#include "PlainTestSuite.h"
#include "TDir.h"
#include "FileOsWalker.h"

class FileOsWalkerTest : public PlainTestSuite {
  Q_OBJECT
 public:
  FileOsWalkerTest() : PlainTestSuite{}, mRootpath{mDir.path()} {}
  TDir mDir;
  const QString mRootpath;
 private slots:
  void initTestCase() {
    QVERIFY(mDir.touch("ABC - DEF - name sc.1/ABC - DEF - name sc.1.m", ""));
    QVERIFY(mDir.touch("ABC - DEF - name sc.1.txt", ""));
  }

  void test_FileOsWalker_WithSub() {
    const QStringList rels{"ABC - DEF - name sc.1",  //
                           "ABC - DEF - name sc.1.txt"};
    FileOsWalker fow{mRootpath, false};
    const bool includeDirectory = true;
    fow(rels, includeDirectory);

    const QStringList relToNames{"",                       //
                                 "ABC - DEF - name sc.1",  //
                                 ""};
    const QStringList completeNames{"ABC - DEF - name sc.1",  //
                                    "ABC - DEF - name sc.1",  //
                                    "ABC - DEF - name sc.1"};
    const QStringList suffixs{"", ".m", ".txt"};
    const QList<bool> isFiles{false, true, true};
    QCOMPARE(fow.relToNames, relToNames);
    QCOMPARE(fow.completeNames, completeNames);
    QCOMPARE(fow.suffixs, suffixs);
    QCOMPARE(fow.isFiles, isFiles);
  }

  void test_FileOsWalker_WithNoSub() {
    const QString pre = mRootpath;
    const QStringList rels{"ABC - DEF - name sc.1",  //
                           "ABC - DEF - name sc.1.txt"};
    const bool includeDirectory = false;
    FileOsWalker fow{mRootpath, false};
    fow(rels, includeDirectory);

    const QStringList relToNames{"",  //
                                 ""};
    const QStringList completeNames{"ABC - DEF - name sc.1",  //
                                    "ABC - DEF - name sc.1"};
    const QStringList suffixs{"", ".txt"};
    const QList<bool> isFiles{false, true};
    QCOMPARE(fow.relToNames, relToNames);
    QCOMPARE(fow.completeNames, completeNames);
    QCOMPARE(fow.suffixs, suffixs);
    QCOMPARE(fow.isFiles, isFiles);
  }

  void test_FileOsWalker_SufInside() {
    const QStringList rels{"ABC - DEF - name sc.1",  //
                           "ABC - DEF - name sc.1.txt"};
    const bool suffixInsideFilename = true;
    FileOsWalker fow{mRootpath, suffixInsideFilename};
    const bool includeDirectory = true;
    fow(rels, includeDirectory);

    const QStringList relToNames{"",                               //
                                 "ABC - DEF - name sc.1",          //
                                 ""};                              //
    const QStringList completeNames{"ABC - DEF - name sc.1",       //
                                    "ABC - DEF - name sc.1.m",     //
                                    "ABC - DEF - name sc.1.txt"};  //
    const QStringList suffixs{"", "", ""};
    const QList<bool> isFiles{false, true, true};
    QCOMPARE(fow.relToNames, relToNames);
    QCOMPARE(fow.completeNames, completeNames);
    QCOMPARE(fow.suffixs, suffixs);
    QCOMPARE(fow.isFiles, isFiles);
  }
};

#include "FileOsWalkerTest.moc"
REGISTER_TEST(FileOsWalkerTest, false)
