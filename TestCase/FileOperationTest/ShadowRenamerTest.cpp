#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "ShadowRenamer.h"
#include "TDir.h"
#include "UndoRedo.h"

using namespace ShadowRenamer;
class ShadowRenamerTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir mTDir;
private slots:
  void initTestCase() {
    QVERIFY(mTDir.IsValid());
    QList<FsNodeEntry> nodes {
      // CheckPathValid
      {"CheckPathValid/HelloWorld", true, ""}, // source
      {"CheckPathValid/Hello", true, ""}, // can be shadow
      {"CheckPathValid/HelloWorldOther", true, ""},  // can be shadow
      {"CheckPathValid/HelloWorld/sub", true, ""},  // can not be shadow
      // videos under folder directly shadow rename
      {"directly/captain american.mp4", false, "any contents1"},
      {"directly/x-men.mp4", false, "any contents2"},
      {"directly/noCorrespondingVideo.mp4", false, "any contents no corresponding video1"},
      {"directlyShadow", true, ""},
      // videos under folder indirectly shadow rename
      {"indirectly/Marvel/captain american.mp4", false, "any contents3"},
      {"indirectly/forbes/x-men/wolverine.mp4", false, "any contents4"},
      {"indirectly/no/noCorrespondingVideo.mp4", false, "any contents no corresponding video2"},
      {"indirectlyShadow", true, ""},
    };
    QCOMPARE(mTDir.createEntries(nodes), nodes.size());

    const QString folderFullPathContainsVideosNeedRename{mTDir.itemPath("CheckPathValid/HelloWorld")};
    QVERIFY(ShadowRenamer::CheckPathValid(folderFullPathContainsVideosNeedRename, mTDir.itemPath("CheckPathValid/Hello")));
    QVERIFY(ShadowRenamer::CheckPathValid(folderFullPathContainsVideosNeedRename, mTDir.itemPath("CheckPathValid/HelloWorldOther")));
    QVERIFY(!ShadowRenamer::CheckPathValid(folderFullPathContainsVideosNeedRename, mTDir.itemPath("CheckPathValid/HelloWorld")));
    QVERIFY(!ShadowRenamer::CheckPathValid(folderFullPathContainsVideosNeedRename, mTDir.itemPath("CheckPathValid/HelloWorld/sub")));

    const int whenCreateShadowFolderFailedRet = -1;
    QCOMPARE(onCreateStagingFile("ZX2:/inexist/folder"), whenCreateShadowFolderFailedRet);
    const std::pair<bool, SyncDetails> whenNoShadowPathRet{true, {}};
    QCOMPARE(ShadowRenamer::SyncSourceFileByShadowFile("ZX2:/inexist/folderShadow", "ZX2:/inexist/folder"), whenNoShadowPathRet);
  }

  void directlyShadow_ok() {
    const QString sourceFolder{mTDir.itemPath("directly")};
    const QString shadowFolder{mTDir.itemPath("directlyShadow")};
    QVERIFY(ShadowRenamer::CheckPathValid(sourceFolder, shadowFolder));
    QCOMPARE(ShadowRenamer::CreateShadowFileUsingNameAsContent(sourceFolder, shadowFolder), 3);
    QCOMPARE(GetStagingFileFolder(sourceFolder), shadowFolder);
    QCOMPARE(ShadowRenamer::onCreateStagingFile(sourceFolder), -1); // shadowFolder not empty, reject it

    QCOMPARE(onShowStagingStatistics(mTDir.itemPath("directlyShadow")), (SyncDetails{3, 3, 0, 0, 0, 0}));

    // video文件丢失 修改影子文件 同步后需要记录
    QVERIFY(QFile::exists(mTDir.itemPath("directly/noCorrespondingVideo.mp4")));
    QVERIFY(QFile::remove(mTDir.itemPath("directly/noCorrespondingVideo.mp4")));
    QVERIFY(!QFile::exists(mTDir.itemPath("directly/noCorrespondingVideo.mp4")));

    QVERIFY(mTDir.checkFileContents("directlyShadow/captain american.mp4", QSet<QString>{JoinShadowFileContent("captain american.mp4", SHADOW_STATUS::PENDING_SYNC)}));
    QVERIFY(mTDir.checkFileContents("directlyShadow/x-men.mp4", QSet<QString>{JoinShadowFileContent("x-men.mp4", SHADOW_STATUS::PENDING_SYNC)}));

    bool bProcedureOk{false};
    ShadowRenamer::SyncDetails detail;
    QVERIFY(detail.isFinished());

    {
      // 对影子文件改名
      QVERIFY(QFile::rename(mTDir.itemPath("directlyShadow/captain american.mp4"), mTDir.itemPath("directlyShadow/Captain American - Steve.mp4")));
      QVERIFY(QFile::rename(mTDir.itemPath("directlyShadow/noCorrespondingVideo.mp4"), mTDir.itemPath("directlyShadow/After Renamed noCorrespondingVideo.mp4")));

      // 反向同步时, 1.更新video文件名, 2.更新shadow文件内容中的source和status两个部分
      std::tie(bProcedureOk, detail) = ShadowRenamer::SyncSourceFileByShadowFile(shadowFolder, sourceFolder);
      QVERIFY(bProcedureOk);
      QCOMPARE(detail.shadowFilesCnt, 3);
      QCOMPARE(detail.pendingSyncCnt, 0);
      QCOMPARE(detail.alreadySyncedCnt, 0);
      QCOMPARE(detail.noNeedSyncCnt, 1);
      QCOMPARE(detail.noCorrespondingCnt, 1);
      QCOMPARE(detail.newSyncedCnt, 1);
      QVERIFY(!detail.isFinished());
      const SyncDetails expectRet{3, 0, 1, 1, 1, 0};
      QCOMPARE(onShowStagingStatistics(mTDir.itemPath("directlyShadow")), expectRet);
      std::tie(bProcedureOk, detail) = ShadowRenamer::SyncSourceFileByShadowFile(shadowFolder, sourceFolder); // do it again
      QVERIFY(bProcedureOk);
      QCOMPARE(detail, expectRet);

      // 1. 更新video文件名
      QVERIFY(!mTDir.exists("directly/captain american.mp4"));
      QVERIFY(mTDir.exists("directly/Captain American - Steve.mp4"));
      // 2. 更新shadow文件内容中的source和status两个部分
      QVERIFY(mTDir.checkFileContents("directlyShadow/Captain American - Steve.mp4", QSet<QString>{JoinShadowFileContent("Captain American - Steve.mp4", SHADOW_STATUS::ALREADY_SYNCED)}));
      QVERIFY(mTDir.checkFileContents("directlyShadow/x-men.mp4", QSet<QString>{JoinShadowFileContent("x-men.mp4", SHADOW_STATUS::NO_NEED_SYNC)}));
    }

    // 移除无效的影子文件
    QVERIFY(QFile::exists(mTDir.itemPath("directlyShadow/After Renamed noCorrespondingVideo.mp4")));
    QVERIFY(QFile::remove(mTDir.itemPath("directlyShadow/After Renamed noCorrespondingVideo.mp4")));
    QVERIFY(!QFile::exists(mTDir.itemPath("directlyShadow/After Renamed noCorrespondingVideo.mp4")));
    QCOMPARE(onShowStagingStatistics(mTDir.itemPath("directlyShadow")), (SyncDetails{2, 0, 1, 1, 0, 0}));
    {
      // 将已经同步过了的影子文件改到另一个新名字, sync时不会生效
      QVERIFY(QFile::rename(mTDir.itemPath("directlyShadow/Captain American - Steve.mp4"), mTDir.itemPath("directlyShadow/Captain American - Chris Evans.mp4")));
      QVERIFY(QFile::rename(mTDir.itemPath("directlyShadow/x-men.mp4"), mTDir.itemPath("directlyShadow/X - MEN - Michael Fassbender.mp4")));

      std::tie(bProcedureOk, detail) = ShadowRenamer::SyncSourceFileByShadowFile(shadowFolder, sourceFolder);
      QVERIFY(bProcedureOk);
      QCOMPARE(detail.shadowFilesCnt, 2);
      QCOMPARE(detail.pendingSyncCnt, 0);
      QCOMPARE(detail.alreadySyncedCnt, 1);
      QCOMPARE(detail.noNeedSyncCnt, 0);
      QCOMPARE(detail.noCorrespondingCnt, 0);
      QCOMPARE(detail.newSyncedCnt, 1);
      QVERIFY(detail.isFinished());

      QVERIFY(mTDir.exists("directly/Captain American - Steve.mp4"));
      QVERIFY(mTDir.exists("directly/X - MEN - Michael Fassbender.mp4"));
    }
    QCOMPARE(onShowStagingStatistics(mTDir.itemPath("directlyShadow")), (SyncDetails{2, 0, 2, 0, 0, 0}));
  }

  void indirectlyShadow_ok() {
    const QString sourceFolder{mTDir.itemPath("indirectly")};
    const QString shadowFolder{mTDir.itemPath("indirectlyShadow")};
    QVERIFY(ShadowRenamer::CheckPathValid(sourceFolder, shadowFolder));
    QCOMPARE(ShadowRenamer::CreateShadowFileUsingNameAsContent(sourceFolder, shadowFolder), 3);
    QCOMPARE(GetStagingFileFolder(sourceFolder), shadowFolder);
    QCOMPARE(ShadowRenamer::onCreateStagingFile(sourceFolder), -1); // shadowFolder not empty, reject it

    QCOMPARE(onShowStagingStatistics(mTDir.itemPath("indirectlyShadow")), (SyncDetails{3, 3, 0, 0, 0, 0}));

    // video文件丢失 修改影子文件 同步后需要记录
    QVERIFY(QFile::exists(mTDir.itemPath("indirectly/no/noCorrespondingVideo.mp4")));
    QVERIFY(QFile::remove(mTDir.itemPath("indirectly/no/noCorrespondingVideo.mp4")));
    QVERIFY(!QFile::exists(mTDir.itemPath("indirectly/no/noCorrespondingVideo.mp4")));

    QVERIFY(mTDir.checkFileContents("indirectlyShadow/Marvel/captain american.mp4", QSet<QString>{JoinShadowFileContent("Marvel/captain american.mp4", SHADOW_STATUS::PENDING_SYNC)}));
    QVERIFY(mTDir.checkFileContents("indirectlyShadow/forbes/x-men/wolverine.mp4", QSet<QString>{JoinShadowFileContent("forbes/x-men/wolverine.mp4", SHADOW_STATUS::PENDING_SYNC)}));

    bool bProcedureOk{false};
    ShadowRenamer::SyncDetails detail;
    QVERIFY(detail.isFinished());

    {
      // 对影子文件改名
      QVERIFY(QFile::rename(mTDir.itemPath("indirectlyShadow/Marvel/captain american.mp4"), mTDir.itemPath("indirectlyShadow/Marvel/Captain American - Steve.mp4")));
      QVERIFY(QFile::rename(mTDir.itemPath("indirectlyShadow/no/noCorrespondingVideo.mp4"), mTDir.itemPath("indirectlyShadow/no/After Renamed noCorrespondingVideo.mp4")));

      // 反向同步时, 1.更新video文件名, 2.更新shadow文件内容中的source和status两个部分
      std::tie(bProcedureOk, detail) = ShadowRenamer::SyncSourceFileByShadowFile(shadowFolder, sourceFolder);
      QVERIFY(bProcedureOk);
      QCOMPARE(detail.shadowFilesCnt, 3);
      QCOMPARE(detail.pendingSyncCnt, 0);
      QCOMPARE(detail.alreadySyncedCnt, 0);
      QCOMPARE(detail.noNeedSyncCnt, 1);
      QCOMPARE(detail.noCorrespondingCnt, 1);
      QCOMPARE(detail.newSyncedCnt, 1);
      QVERIFY(!detail.isFinished());
      const SyncDetails expectRet{3, 0, 1, 1, 1, 0};
      QCOMPARE(onShowStagingStatistics(mTDir.itemPath("indirectlyShadow")), expectRet);
      std::tie(bProcedureOk, detail) = ShadowRenamer::SyncSourceFileByShadowFile(shadowFolder, sourceFolder); // do it again
      QVERIFY(bProcedureOk);
      QCOMPARE(detail, expectRet);

      // 1. 更新video文件名
      QVERIFY(!mTDir.exists("indirectly/Marvel/captain american.mp4"));
      QVERIFY(mTDir.exists("indirectly/Marvel/Captain American - Steve.mp4"));
      // 2. 更新shadow文件内容中的source和status两个部分
      QVERIFY(mTDir.checkFileContents("indirectlyShadow/Marvel/Captain American - Steve.mp4", QSet<QString>{JoinShadowFileContent("Marvel/Captain American - Steve.mp4", SHADOW_STATUS::ALREADY_SYNCED)}));
      QVERIFY(mTDir.checkFileContents("indirectlyShadow/forbes/x-men/wolverine.mp4", QSet<QString>{JoinShadowFileContent("forbes/x-men/wolverine.mp4", SHADOW_STATUS::NO_NEED_SYNC)}));
    }

    // 移除无效的影子文件
    QVERIFY(QFile::exists(mTDir.itemPath("indirectlyShadow/no/After Renamed noCorrespondingVideo.mp4")));
    QVERIFY(QFile::remove(mTDir.itemPath("indirectlyShadow/no/After Renamed noCorrespondingVideo.mp4")));
    QVERIFY(!QFile::exists(mTDir.itemPath("indirectlyShadow/no/After Renamed noCorrespondingVideo.mp4")));
    QCOMPARE(onShowStagingStatistics(mTDir.itemPath("indirectlyShadow")), (SyncDetails{2, 0, 1, 1, 0, 0}));
    {
      // 将已经同步过了的影子文件改到另一个新名字, sync时不会生效
      QVERIFY(QFile::rename(mTDir.itemPath("indirectlyShadow/Marvel/Captain American - Steve.mp4"), mTDir.itemPath("indirectlyShadow/Marvel/Captain American - Chris Evans.mp4")));

      QVERIFY(QFile::rename(mTDir.itemPath("indirectlyShadow/forbes/x-men/wolverine.mp4"), mTDir.itemPath("indirectlyShadow/forbes/x-men/Wolverine - Hugh Jackman.mp4")));
      QVERIFY(QFile::rename(mTDir.itemPath("indirectlyShadow/forbes/x-men"), mTDir.itemPath("indirectlyShadow/forbes/X - MEN")));
      QVERIFY(QFile::rename(mTDir.itemPath("indirectlyShadow/forbes"), mTDir.itemPath("indirectlyShadow/Forbes")));

      std::tie(bProcedureOk, detail) = ShadowRenamer::SyncSourceFileByShadowFile(shadowFolder, sourceFolder);
      QVERIFY(bProcedureOk);
      QCOMPARE(detail.shadowFilesCnt, 2);
      QCOMPARE(detail.pendingSyncCnt, 0);
      QCOMPARE(detail.alreadySyncedCnt, 1);
      QCOMPARE(detail.noNeedSyncCnt, 0);
      QCOMPARE(detail.noCorrespondingCnt, 0);
      QCOMPARE(detail.newSyncedCnt, 1);
      QVERIFY(detail.isFinished());

      QVERIFY(mTDir.exists("indirectly/Marvel/Captain American - Steve.mp4"));
      QVERIFY(mTDir.exists("indirectly/Forbes/X - MEN/Wolverine - Hugh Jackman.mp4"));
    }
    QCOMPARE(onShowStagingStatistics(mTDir.itemPath("indirectlyShadow")), (SyncDetails{2, 0, 2, 0, 0, 0}));
  }

  void onShowStagingStatistics_ok() {
    const SyncDetails defaultDetail;
    QCOMPARE(onShowStagingStatistics(mTDir.itemPath("directly")), defaultDetail); // not a shadow path
    // others tests see in:
    // directlyShadow_ok
    // indirectlyShadow_ok
  }

  void onRecycleNoNeedSyncStagingFile_ok() {
    const std::pair<bool, int> notShadowPath{false, -1};
    QCOMPARE(onRecycleNoNeedSyncStagingFile(mTDir.itemPath("directly")), notShadowPath);
    QCOMPARE(onRecycleNoNeedSyncStagingFile(mTDir.itemPath("indirectly")), notShadowPath);
    QCOMPARE(onRecycleAlreadySyncedStagingFile(mTDir.itemPath("directly")), notShadowPath);
    QCOMPARE(onRecycleAlreadySyncedStagingFile(mTDir.itemPath("indirectly")), notShadowPath);

    const std::pair<bool, int> alreadySyncReturn{true, 2};
    const std::pair<bool, int> noNeedSyncReturn{true, 0};
    QVERIFY(mTDir.exists("directlyShadow/Captain American - Chris Evans.mp4"));
    QVERIFY(mTDir.exists("directlyShadow/X - MEN - Michael Fassbender.mp4"));
    QCOMPARE(onRecycleAlreadySyncedStagingFile(mTDir.itemPath("directlyShadow")), alreadySyncReturn);
    QVERIFY(!mTDir.exists("directlyShadow/Captain American - Chris Evans.mp4"));
    QVERIFY(!mTDir.exists("directlyShadow/X - MEN - Michael Fassbender.mp4"));
    QCOMPARE(onRecycleNoNeedSyncStagingFile(mTDir.itemPath("directlyShadow")), noNeedSyncReturn);
    QVERIFY(UndoRedo::GetInst().on_Undo());
    QVERIFY(mTDir.exists("directlyShadow/Captain American - Chris Evans.mp4"));
    QVERIFY(mTDir.exists("directlyShadow/X - MEN - Michael Fassbender.mp4"));

    QVERIFY(mTDir.exists("indirectlyShadow/Marvel/Captain American - Chris Evans.mp4"));
    QVERIFY(mTDir.exists("indirectlyShadow/Forbes/X - MEN/Wolverine - Hugh Jackman.mp4"));
    QCOMPARE(onRecycleAlreadySyncedStagingFile(mTDir.itemPath("indirectlyShadow")), alreadySyncReturn);
    QVERIFY(!mTDir.exists("indirectlyShadow/Marvel/Captain American - Chris Evans.mp4"));
    QVERIFY(!mTDir.exists("indirectlyShadow/Forbes/X - MEN/Wolverine - Hugh Jackman.mp4"));
    QCOMPARE(onRecycleNoNeedSyncStagingFile(mTDir.itemPath("indirectlyShadow")), noNeedSyncReturn);
    QVERIFY(UndoRedo::GetInst().on_Undo());
    QVERIFY(mTDir.exists("indirectlyShadow/Marvel/Captain American - Chris Evans.mp4"));
    QVERIFY(mTDir.exists("indirectlyShadow/Forbes/X - MEN/Wolverine - Hugh Jackman.mp4"));
  }
};

#include "ShadowRenamerTest.moc"
REGISTER_TEST(ShadowRenamerTest, false)