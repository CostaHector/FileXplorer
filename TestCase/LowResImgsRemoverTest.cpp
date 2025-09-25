#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
// add necessary includes here
#include "LowResImgsRemover.h"
#include "UndoRedo.h"
#include "TDir.h"

class LowResImgsRemoverTest : public PlainTestSuite {
  Q_OBJECT
public:
  LowResImgsRemoverTest()
    : PlainTestSuite{} {}
  LowResImgsRemover m_duplicateImagsRemover;
private slots:
  void GetLowResImgsToDel_ok() {
    const auto& toBeDelete = m_duplicateImagsRemover.GetLowResImgsToDel({"A.jpg", "B.png"});
    QVERIFY2(toBeDelete.isEmpty(), "should contains nothing");

    const auto& onlyOneImageRmvNothing = m_duplicateImagsRemover.GetLowResImgsToDel({"A.jpg"});
    QVERIFY2(onlyOneImageRmvNothing.isEmpty(), "should contains nothing");

    const auto& rmvExcept480p = m_duplicateImagsRemover.GetLowResImgsToDel({"A 360p.jpg", "A 480p.jpg"});
    QCOMPARE(rmvExcept480p, (QStringList{"A 360p.jpg"}));

    const auto& rmvExcept720p = m_duplicateImagsRemover.GetLowResImgsToDel({"A 360p.jpg", "A 480p.jpg", "A 720p.jpg"});
    QCOMPARE(rmvExcept720p, (QStringList{"A 360p.jpg", "A 480p.jpg"}));

    const auto& rmvExcept1080p = m_duplicateImagsRemover.GetLowResImgsToDel({"A 360p.jpg", "A 480p.jpg", "A 720p.jpg", "A 1080p.jpg"});
    QCOMPARE(rmvExcept1080p, (QStringList{"A 360p.jpg", "A 480p.jpg", "A 720p.jpg"}));

    const auto& rmvExcept2160p = m_duplicateImagsRemover.GetLowResImgsToDel(
        {"A 360p.jpg", "A 480p.jpg", "A 720p.jpg", "A 1080p.jpg", "A 2160p.jpg"});
    QCOMPARE(rmvExcept2160p, (QStringList{"A 360p.jpg", "A 480p.jpg", "A 720p.jpg", "A 1080p.jpg"}));
  }

  void remove_low_resolution_images_ok() {
    TDir tDir;
    QCOMPARE(tDir.IsValid(), true);
    QList<FsNodeEntry> nodes{
        {"Chris Evans 720p.jpg", false, ""},
        {"Chris Evans 2160p.mp4", false, ""},
        {"Chris Evans 1080p.jpg", false, ""},
    };
    QCOMPARE(tDir.createEntries(nodes), nodes.size());
    const QString workpath = tDir.path();
    const QStringList& initalLst{"Chris Evans 1080p.jpg", "Chris Evans 2160p.mp4", "Chris Evans 720p.jpg"};

    // only low resolution same image type files will be deleted
    QCOMPARE(m_duplicateImagsRemover(workpath), 1);
    const QStringList& afterRemoveLowResList = tDir.entryList(QDir::Filter::Files, QDir::SortFlag::Name);
    QCOMPARE(afterRemoveLowResList, (QStringList{"Chris Evans 1080p.jpg", "Chris Evans 2160p.mp4"}));

    auto& undoRedoInst = UndoRedo::GetInst();
    QCOMPARE(undoRedoInst.on_Undo(), true);
    const QStringList& afterUndoList = tDir.entryList(QDir::Filter::Files, QDir::SortFlag::Name);
    QCOMPARE(afterUndoList, initalLst);

    QCOMPARE(undoRedoInst.on_Redo(), true);
    const QStringList& afterRedoList = tDir.entryList(QDir::Filter::Files, QDir::SortFlag::Name);
    QCOMPARE(afterRedoList, afterRemoveLowResList);

    // avoid remains in recycle bin
    QCOMPARE(undoRedoInst.on_Undo(), true);
    const QStringList& afterUndo2List = tDir.entryList(QDir::Filter::Files, QDir::SortFlag::Name);
    QCOMPARE(afterUndo2List, initalLst);
  }
};

#include "LowResImgsRemoverTest.moc"
REGISTER_TEST(LowResImgsRemoverTest, false)
