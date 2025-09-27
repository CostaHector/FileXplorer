#include <QtTest>
#include <QCoreApplication>
#include "PlainTestSuite.h"
#include "TDir.h"
#include "BeginToExposePrivateMember.h"
#include "ComplexOperation.h"
#include "FileOpActs.h"
#include "EndToExposePrivateMember.h"
#include "PathTool.h"
#include "MemoryKey.h"

using namespace ComplexOperation;

class ComplexOperationTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir mDir;
  const QString rootpath{mDir.path()};
  const QStringList absPaths{
      rootpath + "/a/a1/a11.txt",
      rootpath + "/a/a1.txt",
      rootpath + "/b/b.md",
      rootpath + "/c",
  };
  const QString dest{
      rootpath + "/d",
  };
  const QList<FsNodeEntry> gNodeEntries{
      FsNodeEntry{"a/a1.txt", false, {}},      // . 46
      FsNodeEntry{"a/a1/a11.txt", false, {}},  // / 47
      FsNodeEntry{"b/b.md", false, {}},        //
      FsNodeEntry{"c", true, {}},              //
      FsNodeEntry{"d", true, {}},              //
  };

  // below only copy/cut/link file. not including directory
  const QStringList mSrcAbsPaths{
      rootpath + "/a/a1/a11.txt",
      rootpath + "/a/a1.txt",
      rootpath + "/b/b.md",
  };
  const QList<QUrl> mSrcUrls{
      QUrl{PathTool::linkPath(rootpath + "/a/a1/a11.txt")},
      QUrl{PathTool::linkPath(rootpath + "/a/a1.txt")},
      QUrl{PathTool::linkPath(rootpath + "/b/b.md")},
  };

  const QSet<QString> mExpectCutFlattenPath{
      "d/a1.txt",   // cut generated
      "d/a11.txt",  //
      "d/b.md",     //
  };
  const QSet<QString> mExpectCutKeepPath{
      "d/b/b.md",        // cut generated
      "d/a/a1.txt",      //
      "d/a/a1/a11.txt",  //
  };
  const QSet<QString> mExpectCopyFlattenPath{
      "a/a1/a11.txt",  // the origin
      "a/a1.txt",      //
      "b/b.md",        //
      "d/a1.txt",      // copy genenrated
      "d/a11.txt",     //
      "d/b.md",        //
  };
  const QSet<QString> mExpectCopyKeepPath{
      "a/a1/a11.txt",    // the origin
      "a/a1.txt",        //
      "b/b.md",          //
      "d/b/b.md",        // copy generated
      "d/a/a1.txt",      //
      "d/a/a1/a11.txt",  //
  };
  const QSet<QString> mExpectLinkFlattenPath{
      "a/a1/a11.txt",   // the origin
      "a/a1.txt",       //
      "b/b.md",         //
      "d/a1.txt.lnk",   // link genenrated
      "d/a11.txt.lnk",  //
      "d/b.md.lnk",     //
  };
  const QSet<QString> mExpectLinkKeepPath{
      "a/a1/a11.txt",        // the origin
      "a/a1.txt",            //
      "b/b.md",              //
      "d/b/b.md.lnk",        // link generated
      "d/a/a1.txt.lnk",      //
      "d/a/a1/a11.txt.lnk",  //
  };
  // above only copy/cut/link file. not including directory

  void ResetWorkPath() {
    QVERIFY(mDir.ClearAll());
    QCOMPARE(mDir.createEntries(gNodeEntries), gNodeEntries.size());
  }

 private slots:
  // a{a1{a11.txt}, a1.txt},
  // b{b.md}
  // c
  // d
  void initTestCase() {
    QVERIFY(mDir.IsValid());
    ResetWorkPath();
  }

  void test_cut_mimedata_flatten() {  //
    QList<QUrl> urlsLst{
        QUrl{PathTool::linkPath(rootpath + "/a/a1.txt")},
        QUrl{PathTool::linkPath(rootpath + "/a/a1")},
    };

    QMimeData mimeData;
    QCOMPARE(ComplexOperationBase::MimeData2FileAbsPaths(mimeData), (QStringList{}));// empty in empty out
    mimeData.setUrls(urlsLst);

    QStringList absPaths = ComplexOperationBase::MimeData2FileAbsPaths(mimeData);
    QStringList selectionAbsPaths{rootpath + "/a/a1.txt",  //
                                  rootpath + "/a/a1"};
    QCOMPARE(absPaths, selectionAbsPaths);

    ComplexMove cm;
    BATCH_COMMAND_LIST_TYPE actualCmds = cm.To(selectionAbsPaths, dest, FileStuctureModeE::FLATTEN);
    BATCH_COMMAND_LIST_TYPE expectCmds{
        //
        ACMD::GetInstMV(rootpath + "/" + "a", "a1.txt", dest),  //
        ACMD::GetInstMV(rootpath + "/" + "a", "a1", dest),      //
    };
    QCOMPARE(actualCmds.size(), expectCmds.size());
    QCOMPARE(actualCmds, expectCmds);
  }

  void test_cut_flatten() {  //
    ComplexMove cm;
    BATCH_COMMAND_LIST_TYPE actualCmds = cm.To(absPaths, dest, FileStuctureModeE::FLATTEN);
    BATCH_COMMAND_LIST_TYPE expectCmds{
        //
        ACMD::GetInstMV(rootpath + '/' + "a/a1", "a11.txt", dest),  //
        ACMD::GetInstMV(rootpath + '/' + "a", "a1.txt", dest),      //
        ACMD::GetInstMV(rootpath + '/' + "b", "b.md", dest),        //
        ACMD::GetInstMV(rootpath, "c", dest),                       //
    };
    QCOMPARE(actualCmds.size(), expectCmds.size());
    QCOMPARE(actualCmds, expectCmds);
  }

  void test_cut_keep() {  //
    ComplexMove cm;
    BATCH_COMMAND_LIST_TYPE actualCmds = cm.To(absPaths, dest, FileStuctureModeE::PRESERVE);
    BATCH_COMMAND_LIST_TYPE expectCmds{
        //
        ACMD::GetInstMV(rootpath, "a/a1/a11.txt", dest),  //
        ACMD::GetInstMV(rootpath, "a/a1.txt", dest),      //
        ACMD::GetInstMV(rootpath, "b/b.md", dest),        //
        ACMD::GetInstMV(rootpath, "c", dest),             //
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
    BATCH_COMMAND_LIST_TYPE actualCmds = cm.To(selectionAbsPaths, dest, FileStuctureModeE::FLATTEN);
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
    BATCH_COMMAND_LIST_TYPE actualCmds = cm.To(absPaths, dest, FileStuctureModeE::FLATTEN);
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
    BATCH_COMMAND_LIST_TYPE actualCmds = cm.To(absPaths, dest, FileStuctureModeE::PRESERVE);
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

  void test_merge_to_empty_dst_path() {  //
    QString path1{rootpath + '/' + "a"};
    QString emptyPath{rootpath + '/' + "c"};

    ComplexMerge cm;
    BATCH_COMMAND_LIST_TYPE actualCmds = cm.Merge(path1, emptyPath);
    BATCH_COMMAND_LIST_TYPE expectCmds{
        //
        ACMD::GetInstMV(rootpath + "/" + "a", "a1.txt", emptyPath),  //
        ACMD::GetInstMV(rootpath + "/" + "a", "a1", emptyPath),      //
        ACMD::GetInstRMDIR(rootpath, "a"),                           //
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

  void testWindowsPlatform() {
#ifdef Q_OS_WIN
    QMimeData moveData;
    QByteArray moveEffect(1, '\x02');  // 0x2 for move
    moveData.setData("Preferred DropEffect", moveEffect);
    QCOMPARE(GetCutCopyModeFromNativeMimeData(moveData), Qt::MoveAction);

    QMimeData copyData;
    QByteArray copyEffect(1, '\x01');  // 0x1 for copy
    copyData.setData("Preferred DropEffect", copyEffect);
    QCOMPARE(GetCutCopyModeFromNativeMimeData(copyData), Qt::CopyAction);

    QMimeData linkData;
    QByteArray linkEffect(1, '\x04');  // 0x4 for link
    linkData.setData("Preferred DropEffect", linkEffect);
    QCOMPARE(GetCutCopyModeFromNativeMimeData(linkData), Qt::LinkAction);

    QMimeData invalidData;
    QByteArray invalidEffect(1, '\x99');  // 无效值
    invalidData.setData("Preferred DropEffect", invalidEffect);
    QCOMPARE(GetCutCopyModeFromNativeMimeData(invalidData), Qt::IgnoreAction);

    QMimeData noFormatData;
    QCOMPARE(GetCutCopyModeFromNativeMimeData(noFormatData), Qt::IgnoreAction);
#else
    QSKIP("This test is Windows-only");
#endif
  }

  void testLinuxPlatform() {
#ifdef Q_OS_LINUX
    {
      QMimeData gnomeCutData;
      gnomeCutData.setData("x-special/gnome-copied-files", "cut\n/path/to/file");
      QCOMPARE(GetCutCopyModeFromNativeMimeData(gnomeCutData), Qt::MoveAction);

      QMimeData gnomeCopyData;
      gnomeCopyData.setData("x-special/gnome-copied-files", "copy\n/path/to/file");
      QCOMPARE(GetCutCopyModeFromNativeMimeData(gnomeCopyData), Qt::CopyAction);

      QMimeData gnomeLinkData;
      gnomeLinkData.setData("x-special/gnome-copied-files", "link\n/path/to/file");
      QCOMPARE(GetCutCopyModeFromNativeMimeData(gnomeLinkData), Qt::LinkAction);

      QMimeData gnomeInvalidData;
      gnomeInvalidData.setData("x-special/gnome-copied-files", "invalid\n/path/to/file");
      QCOMPARE(GetCutCopyModeFromNativeMimeData(gnomeInvalidData), Qt::IgnoreAction);
    }
    {
      QMimeData xdndMoveData;
      xdndMoveData.setData("XdndAction", "XdndActionMove");
      QCOMPARE(GetCutCopyModeFromNativeMimeData(xdndMoveData), Qt::MoveAction);

      QMimeData xdndCopyData;
      xdndCopyData.setData("XdndAction", "XdndActionCopy");
      QCOMPARE(GetCutCopyModeFromNativeMimeData(xdndCopyData), Qt::CopyAction);

      QMimeData xdndLinkData;
      xdndLinkData.setData("XdndAction", "XdndActionLink");
      QCOMPARE(GetCutCopyModeFromNativeMimeData(xdndLinkData), Qt::LinkAction);

      QMimeData xdndInvalidData;
      xdndInvalidData.setData("XdndAction", "XdndActionInvalid");
      QCOMPARE(GetCutCopyModeFromNativeMimeData(xdndInvalidData), Qt::IgnoreAction);
    }

    QMimeData noFormatData;
    QCOMPARE(GetCutCopyModeFromNativeMimeData(noFormatData), Qt::IgnoreAction);
#else
    QSKIP("This test is Linux-only");
#endif
  }

  void DoDropAction_QStringList_ok() {
    using namespace FileStructurePolicy;

    QStringList emptySrcPaths;
    DoDropAction(Qt::DropAction::IgnoreAction, emptySrcPaths, QString{}, FileStuctureModeE::BUTT);  // will not crash down

    {
      ResetWorkPath();
      DoDropAction(Qt::DropAction::CopyAction, mSrcAbsPaths, dest, FileStuctureModeE::FLATTEN);
      QCOMPARE(mDir.Snapshot(QDir::Filter::Files), mExpectCopyFlattenPath);
      QCOMPARE(DoDropAction(Qt::DropAction::CopyAction, emptySrcPaths, {}, FileStuctureModeE::BUTT), 0);  // will not crash down

      ResetWorkPath();
      DoDropAction(Qt::DropAction::MoveAction, mSrcAbsPaths, dest, FileStuctureModeE::FLATTEN);
      QCOMPARE(mDir.Snapshot(QDir::Filter::Files), mExpectCutFlattenPath);
      QCOMPARE(DoDropAction(Qt::DropAction::MoveAction, emptySrcPaths, {}, FileStuctureModeE::BUTT), 0);  // will not crash down

      ResetWorkPath();
      DoDropAction(Qt::DropAction::LinkAction, mSrcAbsPaths, dest, FileStuctureModeE::FLATTEN);
      QCOMPARE(mDir.Snapshot(QDir::Filter::Files), mExpectLinkFlattenPath);
      QCOMPARE(DoDropAction(Qt::DropAction::LinkAction, emptySrcPaths, {}, FileStuctureModeE::BUTT), 0);  // will not crash down
    }
    {
      ResetWorkPath();
      DoDropAction(Qt::DropAction::CopyAction, mSrcAbsPaths, dest, FileStuctureModeE::PRESERVE);
      QCOMPARE(mDir.Snapshot(QDir::Filter::Files), mExpectCopyKeepPath);

      ResetWorkPath();
      DoDropAction(Qt::DropAction::MoveAction, mSrcAbsPaths, dest, FileStuctureModeE::PRESERVE);
      QCOMPARE(mDir.Snapshot(QDir::Filter::Files), mExpectCutKeepPath);

      ResetWorkPath();
      DoDropAction(Qt::DropAction::LinkAction, mSrcAbsPaths, dest, FileStuctureModeE::PRESERVE);
      QCOMPARE(mDir.Snapshot(QDir::Filter::Files), mExpectLinkKeepPath);
    }
  }

  void DoDropAction_QUrlList_ok() {
    using namespace FileStructurePolicy;
    QList<QUrl> emptySrcUrls;
    DoDropAction(Qt::DropAction::IgnoreAction, emptySrcUrls, QString{}, FileStuctureModeE::BUTT);  // will not crash down

    {
      ResetWorkPath();
      DoDropAction(Qt::DropAction::CopyAction, mSrcUrls, dest, FileStuctureModeE::FLATTEN);
      QCOMPARE(mDir.Snapshot(QDir::Filter::Files), mExpectCopyFlattenPath);
      QCOMPARE(DoDropAction(Qt::DropAction::CopyAction, emptySrcUrls, {}, FileStuctureModeE::BUTT), 0);  // will not crash down

      ResetWorkPath();
      DoDropAction(Qt::DropAction::MoveAction, mSrcUrls, dest, FileStuctureModeE::FLATTEN);
      QCOMPARE(mDir.Snapshot(QDir::Filter::Files), mExpectCutFlattenPath);
      QCOMPARE(DoDropAction(Qt::DropAction::MoveAction, emptySrcUrls, {}, FileStuctureModeE::BUTT), 0);  // will not crash down

      ResetWorkPath();
      DoDropAction(Qt::DropAction::LinkAction, mSrcUrls, dest, FileStuctureModeE::FLATTEN);
      QCOMPARE(mDir.Snapshot(QDir::Filter::Files), mExpectLinkFlattenPath);
      QCOMPARE(DoDropAction(Qt::DropAction::LinkAction, emptySrcUrls, {}, FileStuctureModeE::BUTT), 0);  // will not crash down
    }
    {
      ResetWorkPath();
      DoDropAction(Qt::DropAction::CopyAction, mSrcUrls, dest, FileStuctureModeE::PRESERVE);
      QCOMPARE(mDir.Snapshot(QDir::Filter::Files), mExpectCopyKeepPath);

      ResetWorkPath();
      DoDropAction(Qt::DropAction::MoveAction, mSrcUrls, dest, FileStuctureModeE::PRESERVE);
      QCOMPARE(mDir.Snapshot(QDir::Filter::Files), mExpectCutKeepPath);

      ResetWorkPath();
      DoDropAction(Qt::DropAction::LinkAction, mSrcUrls, dest, FileStuctureModeE::PRESERVE);
      QCOMPARE(mDir.Snapshot(QDir::Filter::Files), mExpectLinkKeepPath);
    }
  }
};

#include "ComplexOperationTest.moc"
REGISTER_TEST(ComplexOperationTest, false)
