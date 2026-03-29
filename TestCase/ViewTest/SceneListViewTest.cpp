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

#include "TDir.h"
#include "JsonKey.h"
#include "SceneInPageActions.h"
#include "ImageTestPrecoditionTools.h"
#include "JsonHelper.h"
#include "UndoRedo.h"
#include "BatchRenameBy.h"
#include "RecycleCfmDlg.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

using namespace ImageTestPrecoditionTools;

class SceneListViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
  const QString scnAbsPath = SceneInfoManager::ScnMgr::GetScnAbsFilePath(tDir.path());
  QVariantList expectCurrentSceneChangedArgs_ChrisEvans;
  QVariantList expectCurrentSceneChangedArgs_HenryCavill;
  const QSize listViewImageSize{RateHelper::RATING_BAR_HEIGHT * RateHelper::RATING_BAR_HEIGHT, RateHelper::RATING_BAR_HEIGHT * 2};
  static constexpr int rateChrisEvans = 3;
  static constexpr int rateHenryCavill = 8;

 private slots:
  void initTestCase() {
    // precondition here
    QVERIFY(tDir.IsValid());
    QCOMPARE(scnAbsPath, tDir.itemPath(tDir.baseName() + ".scn"));

    QVERIFY(QMetaType::type("QString") != 0);
    QVERIFY(QMetaType::type("QStringList") != 0);

    QStringList casts;
    QString studios;
    QStringList tags;
    QString details;
    QString md5;
    QString uploaded;

    QByteArray chrisEvansJsonBA = JsonKey::ConstructJsonByteArray("Chris Evans", casts, studios, tags, uploaded, rateChrisEvans,  //
                                                                  0, "", "", {}, 0, {}, "",                                       //
                                                                  false, md5, details);
    QByteArray henryCavillJsonBA = JsonKey::ConstructJsonByteArray("Henry Cavill", casts, studios, tags, uploaded, rateHenryCavill,  //
                                                                   0, "", "", {}, 0, {}, "",                                         //
                                                                   false, md5, details);

    QList<FsNodeEntry> nodes{
        {"Chris Evans.json", false, chrisEvansJsonBA},             // batch 1
        {"Chris Evans.jpg", false, GetPNGImage(100, 50, "jpg")},   //
        {"Chris Evans.mp4", false, ""},                            //
        {"Henry Cavill.json", false, henryCavillJsonBA},           // batch 2
        {"Henry Cavill.png", false, GetPNGImage(50, 100, "png")},  //
        {"Henry Cavill.mp4", false, ""},                           //
    };
    QCOMPARE(tDir.createEntries(nodes), 6);

    expectCurrentSceneChangedArgs_ChrisEvans = QVariantList{
        "Chris Evans",                                  //
        tDir.itemPath("Chris Evans.json"),              //
        QStringList{tDir.itemPath("Chris Evans.jpg")},  //
        QStringList{tDir.itemPath("Chris Evans.mp4")},
    };
    expectCurrentSceneChangedArgs_HenryCavill = QVariantList{
        "Henry Cavill",                                  //
        tDir.itemPath("Henry Cavill.json"),              //
        QStringList{tDir.itemPath("Henry Cavill.png")},  //
        QStringList{tDir.itemPath("Henry Cavill.mp4")},
    };
  }

  void init() {
    SceneInPageActions& sceneAct = g_SceneInPageActions();
    sceneAct._BY_MOVIE_PATH->setChecked(true);
    sceneAct._REVERSE_RESULT->setChecked(false);  // by name ascending chris -> henry
    GlobalMockObject::reset();
  }

  void cleanup() {  //
    GlobalMockObject::verify();
  }

  void cleanupTestCase() {  //
    SceneListViewMocker::MockSetRootPathQuery() = true;
    Configuration().clear();
  }

  void default_constructor_ok() {
    Configuration().clear();
    SceneListView sceneView{nullptr, nullptr, nullptr, nullptr};
    sceneView.InitListView();
    QCOMPARE(sceneView.viewMode(), QListView::ViewMode::IconMode);  // default ok
    QCOMPARE(sceneView.flow(), QListView::Flow::LeftToRight);
    QCOMPARE(sceneView.isWrapping(), true);

    // all memebers should be nullptr.
    QVERIFY(sceneView._sceneModel == nullptr);
    QVERIFY(sceneView._sceneSortProxyModel == nullptr);
    QVERIFY(sceneView._scenePageControl == nullptr);
    QVERIFY(sceneView._OPEN_CORRESPONDING_FOLDER == nullptr);
    QVERIFY(sceneView.mAlignDelegate == nullptr);
    // call onClickEvent with invalid index should not crash down
    sceneView.onClickEvent(QModelIndex());
    sceneView.onCellVisualUpdateRequested({});
  }

  void IsPathAtShallowDepth_ok() {
    // test including /home/path/ and "C:/home/to" 2 platform should both ok
    QVERIFY(!SceneListView::IsPathAtShallowDepth(tDir.path()));

    QVERIFY(SceneListView::IsPathAtShallowDepth("/"));
    QVERIFY(SceneListView::IsPathAtShallowDepth("C:/"));
    QVERIFY(!SceneListView::IsPathAtShallowDepth("/home/user"));
    QVERIFY(!SceneListView::IsPathAtShallowDepth("C:/Users/Public/Documents"));
  }

  void update_json_update_scene_slot_ok() {
    SceneInPageActions& sceneAct = g_SceneInPageActions();

    ScenesListModel sceneModel{"ScenesListView"};
    SceneSortProxyModel sceneProxyModel;
    ScenePageControl pageControlToolbar;
    SceneListView sceneView{&sceneModel, &sceneProxyModel, &pageControlToolbar};
    sceneView.InitListView();
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

    // precondition: no scn files now
    QVERIFY(!QFile::exists(scnAbsPath));
    {
      // 3.1 update json/generate scn file rejected (update on a shallow depth)
      sceneModel.mRootPath = "/";  // force set this path to be root in linux
      QCOMPARE(sceneView.onUpdateJsonFiles(), -1);
      QCOMPARE(sceneView.onUpdateScnFiles(), -1);

      sceneModel.mRootPath = "C:/";  // force set this path to be root in windows
      QCOMPARE(sceneView.onUpdateJsonFiles(), -1);
      QCOMPARE(sceneView.onUpdateScnFiles(), -1);
    }

    {
      // 3.2 update json, update scn, scn file generated ok
      sceneModel.setRootPath(tDir.path());
      SceneInPageActions& sceneActInst = g_SceneInPageActions();
      emit sceneActInst._UPDATE_JSON->triggered();
      emit sceneActInst._UPDATE_SCN->triggered();
      QVERIFY(QFile::exists(scnAbsPath));
      QCOMPARE(sceneModel.rowCount(), 2);  // 2 json one for chris evans, another for henry cavill
    }

    {
      // 3.3 update Json again, update Scn again, clear scn, update Scn
      QCOMPARE(sceneView.onUpdateJsonFiles(), 0);  // already updated. no need again
      QCOMPARE(sceneView.onUpdateScnFiles(), 1);   // ignore whether json changed. update scn using json updated

      QVERIFY(sceneView.onClearScnFiles() > 0);  // clear scn files
      QVERIFY(!QFile::exists(scnAbsPath));

      QVERIFY(sceneView.onUpdateScnFiles() > 0);
      QVERIFY(QFile::exists(scnAbsPath));
    }
  }

  void select_scene_slot_interact_with_sort_ok() {
    SceneInPageActions& sceneAct = g_SceneInPageActions();

    ScenesListModel sceneModel{"ScenesListView"};
    SceneSortProxyModel sceneProxyModel;
    ScenePageControl pageControlToolbar;
    SceneListView sceneView{&sceneModel, &sceneProxyModel, &pageControlToolbar};
    sceneView.InitListView();
    QCOMPARE(sceneModel.rowCount(), 0);

    QVERIFY(QFile::exists(scnAbsPath));
    sceneView.setRootPath(tDir.path());
    QCOMPARE(sceneModel.rowCount(), 2);

    QModelIndex firstIndex, secondIndex;

    firstIndex = sceneProxyModel.index(0, 0);
    secondIndex = sceneProxyModel.index(1, 0);
    QCOMPARE(firstIndex.data(Qt::DisplayRole).toString(), "Chris Evans");
    QCOMPARE(secondIndex.data(Qt::DisplayRole).toString(), "Henry Cavill");

    // check currentSceneChanged signal
    QSignalSpy spy(&sceneView, &SceneListView::currentSceneChanged);
    {
      // invalid index
      QVERIFY(!sceneView.onClickEvent(QModelIndex{}));
      QCOMPARE(spy.count(), 1);

      // currentSceneChanged emit ok
      QVERIFY(sceneView.onClickEvent(firstIndex));
      QCOMPARE(spy.count(), 2);
      QCOMPARE(spy.back(), expectCurrentSceneChangedArgs_ChrisEvans);

      QVERIFY(sceneView.onClickEvent(secondIndex));
      QCOMPARE(spy.count(), 3);
      QCOMPARE(spy.back(), expectCurrentSceneChangedArgs_HenryCavill);
    }

    sceneAct._BY_RATE->setChecked(true);
    sceneAct._REVERSE_RESULT->setChecked(true);  // by rate descending, henry 8, chris 3
    firstIndex = sceneProxyModel.index(0, 0);
    secondIndex = sceneProxyModel.index(1, 0);
    QCOMPARE(firstIndex.data(Qt::DisplayRole).toString(), "Henry Cavill");
    QCOMPARE(secondIndex.data(Qt::DisplayRole).toString(), "Chris Evans");
    QVERIFY(sceneView.onClickEvent(firstIndex));
    QCOMPARE(spy.count(), 4);
    QCOMPARE(spy.back(), expectCurrentSceneChangedArgs_HenryCavill);

    sceneAct._REVERSE_RESULT->setChecked(false);  // by rate descending, henry 8, chris 3
    firstIndex = sceneProxyModel.index(0, 0);
    secondIndex = sceneProxyModel.index(1, 0);
    QCOMPARE(firstIndex.data(Qt::DisplayRole).toString(), "Chris Evans");
    QCOMPARE(secondIndex.data(Qt::DisplayRole).toString(), "Henry Cavill");
    QVERIFY(sceneView.onClickEvent(secondIndex));
    QCOMPARE(spy.count(), 5);
    QCOMPARE(spy.back(), expectCurrentSceneChangedArgs_HenryCavill);
  }

  void open_folder_ok() {
    ScenesListModel sceneModel{"ScenesListView"};
    SceneSortProxyModel sceneProxyModel;
    ScenePageControl pageControlToolbar;
    SceneListView sceneView{&sceneModel, &sceneProxyModel, &pageControlToolbar};
    sceneView.InitListView();
    QCOMPARE(sceneModel.rowCount(), 0);
    QVERIFY(QFile::exists(scnAbsPath));
    SceneListViewMocker::MockSetRootPathQuery() = true;
    sceneView.setRootPath(tDir.path());
    QCOMPARE(sceneModel.rowCount(), 2);

    sceneView.setCurrentIndex(QModelIndex{});
    QVERIFY(!sceneView.currentIndex().isValid());

    {  // open folder
      sceneView.setCurrentIndex(QModelIndex{});
      QVERIFY(!sceneView.onOpenCorrespondingFolder());

      QModelIndex firstIndex = sceneProxyModel.index(0, 0);
      sceneView.setCurrentIndex(firstIndex);
      QVERIFY(sceneView.onOpenCorrespondingFolder());
    }
  }

  void onRenameSceneAndRelated_ok() {
    MOCKER(RecycleCfmDlg::recycleQuestion).expects(exactly(2)).will(returnValue(false)).then(returnValue(true));
    ScenesListModel sceneModel{"ScenesListView"};
    SceneSortProxyModel sceneProxyModel;
    ScenePageControl pageControlToolbar;
    SceneListView sceneView{&sceneModel, &sceneProxyModel, &pageControlToolbar};
    sceneView.InitListView();
    QCOMPARE(sceneModel.rowCount(), 0);
    QVERIFY(QFile::exists(scnAbsPath));
    SceneListViewMocker::MockSetRootPathQuery() = true;
    sceneView.setRootPath(tDir.path());
    QCOMPARE(sceneModel.rowCount(), 2);

    MOCKER(BatchRenameBy::ReplaceBySpecifiedJson).expects(exactly(1)).will(returnValue(1));
    MOCKER(BatchRenameBy::InsertBySpecifiedJson).expects(exactly(1)).will(returnValue(1));
    MOCKER(BatchRenameBy::NumerizerBySpecifiedJson).expects(exactly(1)).will(returnValue(1));

    sceneView.clearSelection();
    QCOMPARE(sceneView.onRenameSceneAndRelated(), 0);          // no selection
    QCOMPARE(sceneView.onRenameSceneAndRelatedInsert(), 0);    // no selection
    QCOMPARE(sceneView.onRenameSceneAndRelatedNumerize(), 0);  // no selection
    QCOMPARE(sceneView.onRecycleSceneAndRelated(), 0);         // no selection

    QCOMPARE(sceneModel.rowCount(), 2);
    sceneView.selectionModel()->select(sceneProxyModel.index(0, 0), QItemSelectionModel::SelectionFlag::SelectCurrent);
    QCOMPARE(sceneView.onRenameSceneAndRelated(), 1);

    sceneModel.setRootPath(tDir.path(), true);
    QCOMPARE(sceneModel.rowCount(), 2);
    sceneView.selectionModel()->select(sceneProxyModel.index(0, 0), QItemSelectionModel::SelectionFlag::SelectCurrent);
    QCOMPARE(sceneView.onRenameSceneAndRelatedInsert(), 1);
    QCOMPARE(sceneModel.rowCount(), 1);

    sceneModel.setRootPath(tDir.path(), true);
    QCOMPARE(sceneModel.rowCount(), 2);
    sceneView.selectionModel()->select(sceneProxyModel.index(0, 0), QItemSelectionModel::SelectionFlag::SelectCurrent);
    QCOMPARE(sceneView.onRenameSceneAndRelatedNumerize(), 1);
    QCOMPARE(sceneModel.rowCount(), 1);

    sceneView.clearSelection();
    sceneView.selectionModel()->select(sceneProxyModel.index(0, 0), QItemSelectionModel::SelectionFlag::SelectCurrent);
    QCOMPARE(sceneView.onRecycleSceneAndRelated(), 0);  // 1st. user click cancel
    QCOMPARE(sceneView.onRecycleSceneAndRelated(), 3);  // 2nd. user click yes 3 files related to json(include json self)
    QCOMPARE(sceneModel.rowCount(), 0);

    QVERIFY(UndoRedo::GetInst().on_Undo());
  }

  void delegate_ok() {
    SceneInPageActions& sceneAct = g_SceneInPageActions();
    sceneAct._BY_RATE->setChecked(true);
    sceneAct._REVERSE_RESULT->setChecked(false);  // by rate ascending

    ScenesListModel sceneModel{"ScenesListView"};
    SceneSortProxyModel sceneProxyModel;
    ScenePageControl pageControlToolbar;
    SceneListView sceneView{&sceneModel, &sceneProxyModel, &pageControlToolbar};
    sceneView.InitListView();
    sceneView.setFlow(QListView::Flow::LeftToRight);
    sceneView.setFixedSize(RateHelper::RATING_BAR_HEIGHT * 9, RateHelper::RATING_BAR_HEIGHT * 3);  // show two item in one row
    sceneModel.onIconSizeChange(listViewImageSize);

    QCOMPARE(sceneModel.rowCount(), 0);

    QVERIFY(QFile::exists(scnAbsPath));
    sceneView.setRootPath(tDir.path());
    QCOMPARE(sceneModel.rowCount(), 2);

    QVERIFY(sceneView.mAlignDelegate != nullptr);

    QModelIndex firstIndex, secondIndex;
    QRect imgRect0, imgRect1;
    QPoint pnt0In, pnt0Out;
    QPoint pnt1In, pnt1Out;
    auto InitPntAndRect = [&]() {
      firstIndex = sceneProxyModel.index(0, 0);
      secondIndex = sceneProxyModel.index(1, 0);

      const QRect vRect0 = sceneView.visualRect(firstIndex);
      imgRect0 = sceneView.mAlignDelegate->GetRealImageVisualRect(firstIndex, vRect0);
      pnt0Out = QPoint{imgRect0.left() + 2, imgRect0.top() + 5};  // rate 1 postion
      pnt0In = QPoint{imgRect0.left() + 2, imgRect0.bottom() - 5};

      const QRect vRect1 = sceneView.visualRect(secondIndex);
      imgRect1 = sceneView.mAlignDelegate->GetRealImageVisualRect(secondIndex, vRect1);
      pnt1Out = QPoint{imgRect1.right() - 2, imgRect1.top() + 5};  // rate 10 postion
      pnt1In = QPoint{imgRect1.right() - 2, imgRect1.bottom() - 5};
    };

    InitPntAndRect();
    QVERIFY(imgRect0.height() > RateHelper::RATING_BAR_HEIGHT);
    QVERIFY(imgRect1.height() > RateHelper::RATING_BAR_HEIGHT);
    QCOMPARE(imgRect0.y(), imgRect1.y());  // in a same row
    QVERIFY(RateHelper::isClickPointInsideRatingBar(pnt0In, imgRect0));
    QVERIFY(RateHelper::isClickPointInsideRatingBar(pnt1In, imgRect1));
    QVERIFY(!RateHelper::isClickPointInsideRatingBar(pnt0Out, imgRect0));
    QVERIFY(!RateHelper::isClickPointInsideRatingBar(pnt1Out, imgRect1));

    const RatingStateMachine& machine = sceneView.mAlignDelegate->mRateMachine;
    QCOMPARE(machine.status(), RatingState::IDLE);
    QVERIFY(machine.mRateData.isInit());

    auto checkNameAndRate = [&](const QString& expect0Name, int expect0Rate, const QString& expect1Name, int expect1Rate) -> void {
      QCOMPARE(firstIndex.data(Qt::DisplayRole).toString(), expect0Name);
      QCOMPARE(firstIndex.data(ScenesListModel::CustomRole::RatingRole).toInt(), expect0Rate);
      QCOMPARE(secondIndex.data(Qt::DisplayRole).toString(), expect1Name);
      QCOMPARE(secondIndex.data(ScenesListModel::CustomRole::RatingRole).toInt(), expect1Rate);
    };

    auto checkJsonScnRate = [&](const int expectChrisRate, const int expectHenryRate) {
      const QVariantHash json1 = JsonHelper::MovieJsonLoader(tDir.itemPath("Chris Evans.json"));
      QCOMPARE(json1.value("Rate", -1).toInt(), expectChrisRate);
      const QVariantHash json2 = JsonHelper::MovieJsonLoader(tDir.itemPath("Henry Cavill.json"));
      QCOMPARE(json2.value("Rate", -1).toInt(), expectHenryRate);

      SceneInfoList parsedScenes = SceneHelper::ParseAScnFile(scnAbsPath, "/");
      QCOMPARE(parsedScenes.size(), 2);
      std::sort(parsedScenes.begin(), parsedScenes.end(), SceneInfo::lessThanName);
      QCOMPARE(parsedScenes[0].rate, expectChrisRate);
      QCOMPARE(parsedScenes[1].rate, expectHenryRate);
    };

    QMouseEvent in0Event(QEvent::MouseButtonPress,
                         pnt0In,  //
                         Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
    QMouseEvent out0Event(QEvent::MouseButtonPress,
                          pnt0Out,  //
                          Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);

    QMouseEvent in1Event(QEvent::MouseButtonPress,
                         pnt1In,  //
                         Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
    QMouseEvent out1Event(QEvent::MouseButtonPress,
                          pnt1Out,  //
                          Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);

    // mouse click ok
    {
      QCOMPARE(machine.status(), RatingState::IDLE);
      checkJsonScnRate(rateChrisEvans, rateHenryCavill);

      // from idle to rate bar <=> ClickOtherRateBarWhenIdle
      sceneView.mousePressEvent(&in0Event);
      QCOMPARE(machine.mRateData, (RateData{firstIndex, RatingState::SELECTED_SHOW, rateChrisEvans, rateChrisEvans}));
      checkNameAndRate("Chris Evans", rateChrisEvans, "Henry Cavill", rateHenryCavill);

      // click same not rate bar when show <=> ClickSameOneNotRateBarWhenShow
      sceneView.mousePressEvent(&out0Event);
      QCOMPARE(machine.mRateData, (RateData{firstIndex, RatingState::SELECTED_HIDE, rateChrisEvans, rateChrisEvans}));
      checkNameAndRate("Chris Evans", rateChrisEvans, "Henry Cavill", rateHenryCavill);

      // click other not rate when hide <=> ClickOtherOneNotRateBarWhenHide
      sceneView.mousePressEvent(&out1Event);
      QCOMPARE(machine.mRateData, (RateData{secondIndex, RatingState::SELECTED_HIDE, rateHenryCavill, rateHenryCavill}));
      checkNameAndRate("Chris Evans", rateChrisEvans, "Henry Cavill", rateHenryCavill);

      // click same in rate when hide <=> ClickSameOneRateBarWhenHide
      sceneView.mousePressEvent(&in1Event);
      QCOMPARE(machine.mRateData, (RateData{secondIndex, RatingState::SELECTED_SHOW, rateHenryCavill, rateHenryCavill}));
      checkNameAndRate("Chris Evans", rateChrisEvans, "Henry Cavill", rateHenryCavill);

      // click same in rate when show <=> ClickSameOneRateBarWhenShow
      sceneView.mousePressEvent(&in1Event);  // in the middle
      QCOMPARE(machine.mRateData, (RateData{secondIndex, RatingState::SELECTED_SHOW, rateHenryCavill, 10}));
      checkNameAndRate("Chris Evans", rateChrisEvans, "Henry Cavill", 10);
      // check json, check scn now
      checkJsonScnRate(rateChrisEvans, 10);

      // click same out rate when show <=> ClickSameOneNotRateBarWhenShow
      sceneView.mousePressEvent(&out1Event);
      QCOMPARE(machine.mRateData, (RateData{secondIndex, RatingState::SELECTED_HIDE, 10, 10}));
      checkNameAndRate("Chris Evans", rateChrisEvans, "Henry Cavill", 10);

      // click same out rate when hide <=> ClickSameOneNotRateBarWhenHide
      sceneView.mousePressEvent(&out1Event);
      QCOMPARE(machine.mRateData, (RateData{secondIndex, RatingState::SELECTED_HIDE, 10, 10}));
      checkNameAndRate("Chris Evans", rateChrisEvans, "Henry Cavill", 10);

      // ClickOtherOneRateBarWhenHide
      sceneView.mousePressEvent(&in0Event);
      QCOMPARE(machine.mRateData, (RateData{firstIndex, RatingState::SELECTED_SHOW, rateChrisEvans, rateChrisEvans}));
      checkNameAndRate("Chris Evans", rateChrisEvans, "Henry Cavill", 10);

      // ClickOtherOneNotRateBarWhenShow
      sceneView.mousePressEvent(&out1Event);
      QCOMPARE(machine.mRateData, (RateData{secondIndex, RatingState::SELECTED_HIDE, 10, 10}));
      checkNameAndRate("Chris Evans", rateChrisEvans, "Henry Cavill", 10);

      // ClickSameOneNotRateBarWhenHide
      sceneView.mousePressEvent(&in1Event);
      QCOMPARE(machine.mRateData, (RateData{secondIndex, RatingState::SELECTED_SHOW, 10, 10}));
      checkNameAndRate("Chris Evans", rateChrisEvans, "Henry Cavill", 10);

      // ClickOtherOneRateBarWhenShow
      sceneView.mousePressEvent(&in0Event);
      QCOMPARE(machine.mRateData, (RateData{firstIndex, RatingState::SELECTED_SHOW, rateChrisEvans, rateChrisEvans}));
      checkNameAndRate("Chris Evans", rateChrisEvans, "Henry Cavill", 10);

      // ClickSameOneRateBarWhenShow
      sceneView.mousePressEvent(&in0Event);
      QCOMPARE(machine.mRateData, (RateData{firstIndex, RatingState::SELECTED_SHOW, rateChrisEvans, 1}));
      checkNameAndRate("Chris Evans", 1, "Henry Cavill", 10);

      // ClickSameOneNotRateBarWhenShow
      sceneView.mousePressEvent(&out0Event);
      QCOMPARE(machine.mRateData, (RateData{firstIndex, RatingState::SELECTED_HIDE, 1, 1}));
      checkNameAndRate("Chris Evans", 1, "Henry Cavill", 10);
      checkJsonScnRate(1, 10);
    }

    {
      sceneView.mAlignDelegate->mRateMachine.mRateData.invalidate();
      QVERIFY(machine.mRateData.isInit());
      QCOMPARE(machine.status(), RatingState::IDLE);

      // from idle to not rate bar <=> ClickOtherNotRateBarWhenIdle
      sceneView.mousePressEvent(&out1Event);
      QCOMPARE(machine.mRateData, (RateData{secondIndex, RatingState::SELECTED_HIDE, 10, 10}));
      checkNameAndRate("Chris Evans", 1, "Henry Cavill", 10);
    }

    {
      // 测试委托功能 - 使用视图中的实际委托对象
      SceneStyleDelegate* delegate = dynamic_cast<SceneStyleDelegate*>(sceneView.itemDelegate());
      QVERIFY(delegate != nullptr);
      QCOMPARE(delegate, sceneView.mAlignDelegate);

      QStyleOptionViewItem option;
      QModelIndex index = firstIndex;
      delegate->initStyleOption(&option, index);
      QCOMPARE(option.decorationPosition, QStyleOptionViewItem::Position::Top);
      QCOMPARE(option.decorationAlignment, Qt::AlignmentFlag::AlignHCenter);
      QCOMPARE(option.textElideMode, Qt::TextElideMode::ElideLeft);
      QCOMPARE(option.displayAlignment, Qt::AlignmentFlag::AlignVCenter);

      // 测试显示文本处理
      QString shortText = "Short text";
      QCOMPARE(delegate->displayText(shortText, QLocale()), shortText);
    }
  }
};

#include "SceneListViewTest.moc"
REGISTER_TEST(SceneListViewTest, false)
