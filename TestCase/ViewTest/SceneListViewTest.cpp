#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "SceneListView.h"
#include "EndToExposePrivateMember.h"

#include "ClipboardGuard.h"
#include "TDir.h"
#include "JsonKey.h"
#include "SceneInPageActions.h"

class SceneListViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
 private slots:
  void initTestCase() {
    // recondition here
    QVERIFY(QMetaType::type("QString") != 0);
    QVERIFY(QMetaType::type("QStringList") != 0);
    QVERIFY(tDir.IsValid());

    QList<FsNodeEntry> nodes{
        {"Chris Evans.json", false, JsonKey::ConstructJsonByteArray("Chris Evans")},    // batch 1
        {"Chris Evans.jpg", false, ""},                                                 //
        {"Chris Evans.mp4", false, ""},                                                 //
        {"Henry Cavill.json", false, JsonKey::ConstructJsonByteArray("Henry Cavill")},  // batch 1
        {"Henry Cavill.png", false, ""},                                                //
        {"Henry Cavill.mp4", false, ""},                                                //
    };
    QCOMPARE(tDir.createEntries(nodes), 6);

    Configuration().clear();
  }

  void cleanupTestCase() { SceneListViewMocker::MockSetRootPathQuery() = true; }

  void default_constructor_ok() {
    SceneListView sceneView{nullptr, nullptr, nullptr, nullptr};
    // all memebers should be nullptr.
    QVERIFY(sceneView._sceneModel == nullptr);
    QVERIFY(sceneView._sceneSortProxyModel == nullptr);
    QVERIFY(sceneView._scenePageControl == nullptr);
    QVERIFY(sceneView.COPY_BASENAME_FROM_SCENE == nullptr);
    QVERIFY(sceneView.OPEN_CORRESPONDING_FOLDER == nullptr);
    QVERIFY(sceneView.mAlignDelegate == nullptr);
    // call onClickEvent with invalid index should not crash down
    sceneView.onClickEvent(QModelIndex(), QModelIndex());
  }

  void IsPathAtShallowDepth_ok() {
    // test including /home/path/ and "C:/home/to" 2 platform should both ok
    QVERIFY(!SceneListView::IsPathAtShallowDepth(tDir.path()));

    QVERIFY(SceneListView::IsPathAtShallowDepth("/"));
    QVERIFY(SceneListView::IsPathAtShallowDepth("C:/"));
    QVERIFY(!SceneListView::IsPathAtShallowDepth("/home/user"));
    QVERIFY(!SceneListView::IsPathAtShallowDepth("C:/Users/Public/Documents"));
  }

  void subscribe_signal_slot_ok() {
    ScenesListModel sceneModel;
    SceneSortProxyModel sceneProxyModel;
    ScenePageControl pageControlToolbar;
    SceneListView sceneView{&sceneModel, &sceneProxyModel, &pageControlToolbar};
    QCOMPARE(sceneModel.rowCount(), 0);
    {
      // 1. setRootPath on an rootPath "/", _sceneModel->rootPath() should not change
      SceneListViewMocker::MockSetRootPathQuery() = false;
      sceneView.setRootPath("/");
      QVERIFY(sceneModel.rootPath() != "/");
      sceneView.setRootPath("C:/home");
      QVERIFY(sceneModel.rootPath() != "C:/home");
    }

    {  // 2. setRootPath on a test directory ok
      SceneListViewMocker::MockSetRootPathQuery() = true;
      sceneView.setRootPath(tDir.path());
      QCOMPARE(sceneModel.rootPath(), tDir.path());
      QCOMPARE(sceneModel.rowCount(), 0);
    }

    // no scn files now
    const QDir mDir = QDir(tDir);
    const QStringList emptyScnList = mDir.entryList({"*.scn"}, QDir::Filter::Files);
    QVERIFY(emptyScnList.isEmpty());
    {
      // generate scn file ok
      // update on a shallow depth get rejected
      sceneModel.mRootPath = "/";  // force set this path to be root
      QCOMPARE(sceneView.onUpdateJsonFiles(), -1);
      sceneModel.mRootPath = "C:/";  // force set this path to be root
      QCOMPARE(sceneView.onUpdateJsonFiles(), -1);

      sceneModel.setRootPath(tDir.path());
      {
        SceneInPageActions& sceneActInst = g_SceneInPageActions();
        emit sceneActInst._UPDATE_JSON->triggered();
        emit sceneActInst._UPDATE_SCN->triggered();
        QStringList scn2FileList = mDir.entryList({"*.scn"}, QDir::Filter::Files);
        QCOMPARE(scn2FileList.size(), 1);
      }
      QCOMPARE(sceneModel.rowCount(), 2);  // 2 json one for chris evans, another for henry cavill

      QCOMPARE(sceneView.onUpdateJsonFiles(), 0);  // already updated. no need again
      QCOMPARE(sceneView.onUpdateScnFiles(), 1);   // ignore whether json changed. update scn using json updated
      {
        QVERIFY(sceneView.onClearScnFiles() > 0);  // clear scn files
        QStringList scn2FileList = mDir.entryList({"*.scn"}, QDir::Filter::Files);
        QCOMPARE(scn2FileList.size(), 0);
      }
      {
        QVERIFY(sceneView.onUpdateScnFiles() > 0);  // update/generated scn files
        QStringList scn2FileList = mDir.entryList({"*.scn"}, QDir::Filter::Files);
        QVERIFY(scn2FileList.size() > 0);
      }
    }

    // check signal currentSceneChanged emit ok
    // 测试点击事件发射信号
    {
      QSignalSpy spy(&sceneView, &SceneListView::currentSceneChanged);
      QModelIndex firstIndex = sceneProxyModel.index(0, 0);
      sceneView.setCurrentIndex(firstIndex);
      QCOMPARE(spy.count(), 1);
      sceneView.onClickEvent(firstIndex, QModelIndex());
      QCOMPARE(spy.count(), 2);

      QList<QVariant> arguments = spy.back();
      QCOMPARE(arguments.size(), 3);
      QString name = arguments[0].toString();
      QStringList imgList = arguments[1].toStringList();
      QStringList vidList = arguments[2].toStringList();
      QVERIFY(name == "Chris Evans");
      QCOMPARE(imgList.size(), 1);
      QVERIFY(imgList[0].contains(".jpg"));
      QCOMPARE(vidList.size(), 1);
      QVERIFY(vidList[0].contains(".mp4"));
    }

    {
      // copy basename ok
      ClipboardGuard clipGuard;
      QClipboard* clipboard = clipGuard.clipBoard();
      QVERIFY(clipboard != nullptr);
      sceneView.onCopyBaseName();
      QString copiedText = clipboard->text();
      QVERIFY(copiedText == "Chris Evans");
      // auto recover
    }

    {
      // 测试打开对应文件夹（模拟行为）
      sceneView.onOpenCorrespondingFolder();
    }

    {
      // 测试委托功能 - 使用视图中的实际委托对象
      AlignDelegate* delegate = dynamic_cast<AlignDelegate*>(sceneView.itemDelegate());
      QVERIFY(delegate != nullptr);

      QStyleOptionViewItem option;
      QModelIndex index = sceneProxyModel.index(0, 0);
      delegate->initStyleOption(&option, index);
      QCOMPARE(option.decorationPosition, QStyleOptionViewItem::Position::Top);
      QCOMPARE(option.decorationAlignment, Qt::AlignmentFlag::AlignHCenter);
      QCOMPARE(option.textElideMode, Qt::TextElideMode::ElideLeft);
      QCOMPARE(option.displayAlignment, Qt::AlignmentFlag::AlignVCenter);

      // 测试显示文本处理
      QString shortText = "Short text";
      QCOMPARE(delegate->displayText(shortText, QLocale()), shortText);

      QString longText(50, 'a');  // 50个'a'
      QString expected = QString(20, 'a') + "\n" + QString(20, 'a');
      QCOMPARE(delegate->displayText(longText, QLocale()), expected);
    }
  }
};

#include "SceneListViewTest.moc"
REGISTER_TEST(SceneListViewTest, false)
