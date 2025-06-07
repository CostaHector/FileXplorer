#include <QtTest>
#include <QCoreApplication>
#include "TestCase/PathRelatedTool.h"
#include "TestCase/pub/MyTestSuite.h"
#include "public/FileOsWalker.h"

const QString rootpath = TestCaseRootPath() + "/test/TestEnv_FileOsWalker";

class FileOsWalkerTest : public MyTestSuite {
  Q_OBJECT
 public:
  FileOsWalkerTest() : MyTestSuite{false} {}
 private slots:
  void init() {
    // under path test/TestEnv_FileOsWalker, there are 3 item(s) in total as follow:
    // ABC - DEF - name sc.1
    // -  ABC - DEF - name sc.1.m
    // ABC - DEF - name sc.1.txt
    QVERIFY(QFile::exists(rootpath));
    QVERIFY(QFile::exists(rootpath + "\\ABC - DEF - name sc.1"));
    QVERIFY(QFile::exists(rootpath + "\\ABC - DEF - name sc.1\\ABC - DEF - name sc.1.m"));
    QVERIFY(QFile::exists(rootpath + "\\ABC - DEF - name sc.1.txt"));
  }

  void test_FileOsWalker_WithSub() {
    const QStringList rels{"ABC - DEF - name sc.1",  //
                           "ABC - DEF - name sc.1.txt"};
    FileOsWalker fow{rootpath, false};
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
    const QString pre = rootpath;
    const QStringList rels{"ABC - DEF - name sc.1",  //
                           "ABC - DEF - name sc.1.txt"};
    const bool includeDirectory = false;
    FileOsWalker fow{rootpath, false};
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
    FileOsWalker fow{rootpath, suffixInsideFilename};
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
FileOsWalkerTest gFileOsWalkerTest;
