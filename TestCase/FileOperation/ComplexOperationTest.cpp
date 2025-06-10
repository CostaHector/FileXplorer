#include <QtTest>
#include <QCoreApplication>
#include "TestCase/pub/MyTestSuite.h"
#include "TestCase/PathRelatedTool.h"
#include "FileOperation/ComplexOperation.h"
#include "public/PathTool.h"

const QString rootpath = TestCaseRootPath() + "/test/TestEnv_ComplexOperation";
using namespace ComplexOperation;

const QStringList absPaths{
    rootpath + '/' + "a/a1/a11.txt",
    rootpath + '/' + "a/a1.txt",
    rootpath + '/' + "b/b.md",
    rootpath + '/' + "c",
};
const QString dest{
    rootpath + '/' + "d",
};

class ComplexOperationTest : public MyTestSuite {
  Q_OBJECT
 public:
  ComplexOperationTest() : MyTestSuite{false} {}
 private slots:
  // a{a1{a11.txt}, a1.txt},
  // b{b.md}
  // c
  // d
  void initTestCase() {
    QVERIFY(QFile::exists(rootpath + "/a/a1.txt"));
    QVERIFY(QFile::exists(rootpath + "/b/b.md"));
    QVERIFY(QFile::exists(rootpath + "/a/a1/a11.txt"));
    QVERIFY(QFile::exists(rootpath + "/c"));
    QVERIFY(QFile::exists(rootpath + "/d"));
  }

  void test_cut_mimedata_flatten() {  //
    QList<QUrl> urlsLst{
        QUrl{PathTool::linkPath(rootpath + "/a/a1.txt")},
        QUrl{PathTool::linkPath(rootpath + "/a/a1")},
    };

    QMimeData mimeData;
    mimeData.setUrls(urlsLst);

    QStringList absPaths = ComplexOperationBase::MimeData2FileAbsPaths(mimeData);
    QStringList selectionAbsPaths{rootpath + "/a/a1.txt",  //
                                  rootpath + "/a/a1"};
    QCOMPARE(absPaths, selectionAbsPaths);

    ComplexMove cm;
    BATCH_COMMAND_LIST_TYPE actualCmds = cm.To(selectionAbsPaths, dest, FILE_STRUCTURE_MODE::FLATTEN);
    BATCH_COMMAND_LIST_TYPE expectCmds{
        //
        ACMD::GetInstMV(rootpath + "/" + "a", "a1.txt", dest),  //
        ACMD::GetInstMV(rootpath + "/" + "a", "a1", dest),          //
    };
    QCOMPARE(actualCmds.size(), expectCmds.size());
    QCOMPARE(actualCmds, expectCmds);
  }

  void test_cut_flatten() {  //
    ComplexMove cm;
    BATCH_COMMAND_LIST_TYPE actualCmds = cm.To(absPaths, dest, FILE_STRUCTURE_MODE::FLATTEN);
    BATCH_COMMAND_LIST_TYPE expectCmds{
        //
        ACMD::GetInstMV(rootpath + '/' + "a/a1", "a11.txt", dest),  //
        ACMD::GetInstMV(rootpath + '/' + "a", "a1.txt", dest),       //
        ACMD::GetInstMV(rootpath + '/' + "b", "b.md", dest),           //
        ACMD::GetInstMV(rootpath, "c", dest),                             //
    };
    QCOMPARE(actualCmds.size(), expectCmds.size());
    QCOMPARE(actualCmds, expectCmds);
  }

  void test_cut_keep() {  //
    ComplexMove cm;
    BATCH_COMMAND_LIST_TYPE actualCmds = cm.To(absPaths, dest, FILE_STRUCTURE_MODE::KEEP);
    BATCH_COMMAND_LIST_TYPE expectCmds{
        //
        ACMD::GetInstMV(rootpath, "a/a1/a11.txt", dest),  //
        ACMD::GetInstMV(rootpath, "a/a1.txt", dest),          //
        ACMD::GetInstMV(rootpath, "b/b.md", dest),              //
        ACMD::GetInstMV(rootpath, "c", dest),                        //
    };
    QCOMPARE(actualCmds.size(), expectCmds.size());
    QCOMPARE(actualCmds, expectCmds);
  }

  void test_copy_mimedata_flatten() {  //
    QList<QUrl> urlsLst{
        QUrl{PathTool::linkPath(rootpath + "/a/a1.txt")},
        QUrl{PathTool::linkPath(rootpath + "/a/a1")},
    };

    QMimeData mimeData;
    mimeData.setUrls(urlsLst);

    QStringList absPaths = ComplexOperationBase::MimeData2FileAbsPaths(mimeData);
    QStringList selectionAbsPaths{rootpath + "/a/a1.txt",  //
                                  rootpath + "/a/a1"};
    QCOMPARE(absPaths, selectionAbsPaths);

    ComplexCopy cm;
    BATCH_COMMAND_LIST_TYPE actualCmds = cm.To(selectionAbsPaths, dest, FILE_STRUCTURE_MODE::FLATTEN);
    BATCH_COMMAND_LIST_TYPE expectCmds{
        //
        ACMD::GetInstCPFILE(rootpath + "/" + "a", "a1.txt", dest),  //
        ACMD::GetInstCPDIR(rootpath + "/" + "a", "a1", dest),       //
    };
    QCOMPARE(actualCmds.size(), expectCmds.size());
    QCOMPARE(actualCmds, expectCmds);
  }

  void test_copy_flatten() {  //
    ComplexCopy cm;
    BATCH_COMMAND_LIST_TYPE actualCmds = cm.To(absPaths, dest, FILE_STRUCTURE_MODE::FLATTEN);
    BATCH_COMMAND_LIST_TYPE expectCmds{
        //
        ACMD::GetInstCPFILE(rootpath + "/" + "a/a1", "a11.txt", dest),  //
        ACMD::GetInstCPFILE(rootpath + "/" + "a", "a1.txt", dest),      //
        ACMD::GetInstCPFILE(rootpath + "/" + "b", "b.md", dest),        //
        ACMD::GetInstCPDIR(rootpath, "c", dest),                        //
    };
    QCOMPARE(actualCmds.size(), expectCmds.size());
    QCOMPARE(actualCmds, expectCmds);
  }
  void test_copy_keep() {  //
    ComplexCopy cm;
    BATCH_COMMAND_LIST_TYPE actualCmds = cm.To(absPaths, dest, FILE_STRUCTURE_MODE::KEEP);
    BATCH_COMMAND_LIST_TYPE expectCmds{
        //
        ACMD::GetInstCPFILE(rootpath, "a/a1/a11.txt", dest),  //
        ACMD::GetInstCPFILE(rootpath, "a/a1.txt", dest),      //
        ACMD::GetInstCPFILE(rootpath, "b/b.md", dest),        //
        ACMD::GetInstCPDIR(rootpath, "c", dest),              //
    };
    QCOMPARE(actualCmds.size(), expectCmds.size());
    QCOMPARE(actualCmds, expectCmds);
  }

  // a{a1{a11.txt}, a1.txt},
  // b{b.md}
  // c
  // d
  void test_merge_to_empty_dst_path() {  //
    QString path1{rootpath + '/' + "a"};
    QString emptyPath{rootpath + '/' + "c"};

    ComplexMerge cm;
    BATCH_COMMAND_LIST_TYPE actualCmds = cm.Merge(path1, emptyPath);
    BATCH_COMMAND_LIST_TYPE expectCmds{
        //
        ACMD::GetInstMV(rootpath + "/" + "a", "a1.txt", emptyPath),  //
        ACMD::GetInstMV(rootpath + "/" + "a", "a1", emptyPath),          //
        ACMD::GetInstRMDIR(rootpath, "a"),                                         //
    };
    QCOMPARE(actualCmds.size(), expectCmds.size());
    QCOMPARE(actualCmds, expectCmds);
  }

  void test_merge_empty_path_to_dst() {  //
    QString path1{rootpath + '/' + "a"};
    QString emptyPath{rootpath + '/' + "c"};

    ComplexMerge cm;
    BATCH_COMMAND_LIST_TYPE actualCmds = cm.Merge(emptyPath, path1);
    BATCH_COMMAND_LIST_TYPE expectCmds{
        //
        ACMD::GetInstRMDIR(rootpath, "c"),  //
    };
    QCOMPARE(actualCmds.size(), expectCmds.size());
    QCOMPARE(actualCmds, expectCmds);
  }

  void test_merge_two_empty_path() {  //
    QString emptyPath1{rootpath + '/' + "c"};
    QString emptyPath2{rootpath + '/' + "d"};

    ComplexMerge cm;
    BATCH_COMMAND_LIST_TYPE actualCmds = cm.Merge(emptyPath1, emptyPath2);
    BATCH_COMMAND_LIST_TYPE expectCmds{
        //
        ACMD::GetInstRMDIR(rootpath, "c"),  //
    };
    QCOMPARE(actualCmds.size(), expectCmds.size());
    QCOMPARE(actualCmds, expectCmds);
  }
};

#include "ComplexOperationTest.moc"
ComplexOperationTest gComplexOperationTest;
