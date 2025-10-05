#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "BeginToExposePrivateMember.h"
#include "NavigationAndAddressBar.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"

#include "IntoNewPathMocker.h"

class NavigationAndAddressBarTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;

 private slots:
  void initTestCase() {
    QVERIFY(tDir.IsValid());
    QList<FsNodeEntry> nodes{
        {"folder1/file1.txt", false, "contents in file1"},  //
        {"folder2/file2.txt", false, "contents in file2"},  //
        {"folder3/file3.txt", false, "contents in file3"},  //
    };
    QCOMPARE(tDir.createEntries(nodes), 3);  //
    PathSearchMockerClear();
  }

  void cleanupTestCase() { PathSearchMockerClear(); }

  void address_and_navi_works_ok() {
    QWidget wid;
    NavigationAndAddressBar naviAddr{"Navigation&AddressBar Test", &wid};
    {  // 0. before BindFileSystemViewCallback called. the related member should be nullptr, call memeber should not result in crash down
      QCOMPARE(naviAddr.m_IntoNewPath, nullptr);
      QCOMPARE(naviAddr.m_on_searchTextChanged, nullptr);
      QCOMPARE(naviAddr.m_on_searchEnterKey, nullptr);

      QVERIFY(naviAddr.m_addressLine != nullptr);
      QVERIFY(naviAddr.mFsSearchLE != nullptr);
      QVERIFY(naviAddr.m_fsFilterBtn != nullptr);
      QVERIFY(naviAddr.mLastNextFolderTb != nullptr);

      naviAddr.onBackward();
      naviAddr.onForward();
      naviAddr.onUpTo();
      naviAddr.onIteratorToNextFolder();
      naviAddr.onIteratorToLastFolder();
      naviAddr.onSearchTextChanged();
      naviAddr.onSearchTextReturnPressed();
    }

    IntoNewPathMocker pathMocker;
    SearchTextChangedMocker searchTxtChangeMocker;
    SearchTextEnterMocker searchTxtEnterMocker;

    QFileSystemModel fileSystemModel;

    {  // 1.0 BindFileSystemViewCallback ok
      naviAddr.BindFileSystemViewCallback(pathMocker, searchTxtChangeMocker, searchTxtEnterMocker, &fileSystemModel);

      QVERIFY(naviAddr.m_IntoNewPath != nullptr);
      QVERIFY(naviAddr.m_on_searchTextChanged != nullptr);
      QVERIFY(naviAddr.m_on_searchEnterKey != nullptr);
    }

    PathSearchMockerClear();
    {
      // ChangePath will not influence IntoNewPathParms
      QCOMPARE(IntoNewPathParms::GetInst(), (IntoNewPathParms{"", false, false}));
      QVERIFY(naviAddr.m_addressLine->ChangePath(tDir.itemPath("folder2")));
      QCOMPARE(naviAddr.m_addressLine->pathFromLineEdit(), tDir.itemPath("folder2"));
      QCOMPARE(naviAddr.m_addressLine->pathFromFullActions(), tDir.itemPath("folder2"));
      QCOMPARE(IntoNewPathParms::GetInst(), (IntoNewPathParms{"", false, false}));
    }

    PathSearchMockerClear();
    {
      // mFsSearchLE returnPressed, or textChanged should works on
      QCOMPARE(SearchTextChangedParms::GetInst(), (SearchTextChangedParms{""}));
      naviAddr.mFsSearchLE->setText("M");  // auto trigger textChanged
      QCOMPARE(SearchTextChangedParms::GetInst(), (SearchTextChangedParms{"M"}));

      QCOMPARE(SearchTextEnterParms::GetInst(), (SearchTextEnterParms{""}));
      naviAddr.mFsSearchLE->setText("Henry Cavill");
      emit naviAddr.mFsSearchLE->returnPressed();
      QCOMPARE(SearchTextEnterParms::GetInst(), (SearchTextEnterParms{"Henry Cavill"}));
    }

    PathSearchMockerClear();
    {  // onUpTo,
      bool isParentPathSameAsCurrent = false;
      const QString expectParPath = naviAddr.m_addressLine->parentPath(&isParentPathSameAsCurrent);
      QVERIFY(!isParentPathSameAsCurrent);
      QVERIFY(naviAddr.onUpTo());
      QCOMPARE(IntoNewPathParms::GetInst(), (IntoNewPathParms{expectParPath, true, false}));
      IntoNewPathParms::GetInst().clear();

      naviAddr.m_addressLine->ChangePath("");
      const QString expectSameParPath = naviAddr.m_addressLine->parentPath(&isParentPathSameAsCurrent);
      QCOMPARE(expectSameParPath, "");
      QVERIFY(isParentPathSameAsCurrent);
      QVERIFY(!naviAddr.onUpTo());
      QCOMPARE(IntoNewPathParms::GetInst(), (IntoNewPathParms{"", false, false}));
    }

    PathSearchMockerClear();
    {  // onBackward, onForward,
      QVERIFY(naviAddr.m_addressLine->ChangePath(tDir.itemPath("folder2")));
      QVERIFY(!naviAddr.m_pathRD.undoPathAvailable());
      QVERIFY(!naviAddr.onBackward());
      QCOMPARE(IntoNewPathParms::GetInst(), (IntoNewPathParms{"", false, false}));
      QVERIFY(!naviAddr.m_pathRD.undoPathAvailable());
      QVERIFY(!naviAddr.onForward());
      QCOMPARE(IntoNewPathParms::GetInst(), (IntoNewPathParms{"", false, false}));

      naviAddr.m_pathRD(tDir.itemPath("folder2"));  // undostack:[folder2], redoStack:[]
      QVERIFY(!naviAddr.m_pathRD.undoPathAvailable());
      naviAddr.m_pathRD(tDir.itemPath("folder3"));  // undostack:[folder2, folder3], redoStack:[]
      QVERIFY(naviAddr.m_pathRD.undoPathAvailable());
      QVERIFY(!naviAddr.m_pathRD.redoPathAvailable());
      QVERIFY(naviAddr.onBackward());  // undostack:[folder2], redoStack:[folder3], enter folder2
      QCOMPARE(IntoNewPathParms::GetInst(), (IntoNewPathParms{tDir.itemPath("folder2"), false, false}));
      IntoNewPathParms::GetInst().clear();

      QVERIFY(naviAddr.m_pathRD.redoPathAvailable());
      QVERIFY(naviAddr.onForward());  // undostack:[folder2, folder3], redoStack:[], enter folder3
      QCOMPARE(IntoNewPathParms::GetInst(), (IntoNewPathParms{tDir.itemPath("folder3"), false, false}));
    }

    PathSearchMockerClear();
    {  // onIteratorToNextFolder, onIteratorToLastFolder
      // 2->3->1
      QVERIFY(naviAddr.m_addressLine->ChangePath(tDir.itemPath("folder2")));
      QVERIFY(naviAddr.onIteratorToNextFolder());
      QCOMPARE(IntoNewPathParms::GetInst(), (IntoNewPathParms{tDir.itemPath("folder3"), true, false}));
      QVERIFY(naviAddr.m_addressLine->ChangePath(tDir.itemPath("folder3")));
      QVERIFY(naviAddr.onIteratorToNextFolder());  // already the back element, wrap to the first one
      QCOMPARE(IntoNewPathParms::GetInst(), (IntoNewPathParms{tDir.itemPath("folder1"), true, false}));
      QVERIFY(naviAddr.m_addressLine->ChangePath(tDir.itemPath("folder1")));

      // 1->3->2
      QVERIFY(naviAddr.m_addressLine->ChangePath(tDir.itemPath("folder1")));
      QVERIFY(naviAddr.onIteratorToLastFolder());
      QCOMPARE(IntoNewPathParms::GetInst(), (IntoNewPathParms{tDir.itemPath("folder3"), true, false}));
      QVERIFY(naviAddr.m_addressLine->ChangePath(tDir.itemPath("folder3")));
      QVERIFY(naviAddr.onIteratorToLastFolder());  // already the back element, wrap to the first one
      QCOMPARE(IntoNewPathParms::GetInst(), (IntoNewPathParms{tDir.itemPath("folder2"), true, false}));
      QVERIFY(naviAddr.m_addressLine->ChangePath(tDir.itemPath("folder2")));
    }
    PathSearchMockerClear();

    {  // onGetFocus ok
      naviAddr.onGetFocus();
      // don't expect focus. widget not shown up at all so will not own focus
      QCOMPARE(naviAddr.mFsSearchLE->selectedText(), naviAddr.mFsSearchLE->text());
    }
  }

  void SplitToolButton_2_action_in_one_button_works_ok() {
    SplitToolButton button;
    button.setFixedSize(100, 100);
    const QPoint centerPnt = button.geometry().center();
    const QPoint topHalfPnt = centerPnt - QPoint(0, 10);
    const QPoint bottomHalfPnt = centerPnt + QPoint(0, 10);

    {  // 0. pass nullptr to mousePressEvent should not crash down
      QMouseEvent* mouseEventNullptr = nullptr;
      button.mousePressEvent(mouseEventNullptr);
    }

    // 1.0 both mouse click on top Half or keyboard "Ctrl+[" should emit topHalfClicked
    {
      QSignalSpy topSpy(&button, &SplitToolButton::topHalfClicked);
      QMouseEvent topEvent(QEvent::MouseButtonPress, topHalfPnt, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
      button.mousePressEvent(&topEvent);
      QCOMPARE(topSpy.count(), 1);
      QVERIFY(topEvent.isAccepted());

      QCOMPARE(button.topShortcut->key(), QKeySequence(Qt::ControlModifier | Qt::Key_BracketLeft));
      emit button.topShortcut->activated();
      QCOMPARE(topSpy.count(), 2);
    }

    // 2.0 both mouse click on bottom Half or keyboard "Ctrl+[" should emit topHalfClicked
    {
      QSignalSpy bottomSpy(&button, &SplitToolButton::bottomHalfClicked);
      QMouseEvent bottomEvent(QEvent::MouseButtonPress, bottomHalfPnt, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
      button.mousePressEvent(&bottomEvent);
      QCOMPARE(bottomSpy.count(), 1);
      QVERIFY(bottomEvent.isAccepted());

      QCOMPARE(button.bottomShortcut->key(), QKeySequence(Qt::ControlModifier | Qt::Key_BracketRight));
      emit button.bottomShortcut->activated();
      QCOMPARE(bottomSpy.count(), 2);
    }

    // 3.0 tooltip ok
    {
      const QString toolTipString = button.GetToolTipString("previous folder", "next folder");
      QVERIFY(toolTipString.contains("previous folder"));
      QVERIFY(toolTipString.contains("next folder"));
      QVERIFY(toolTipString.contains("Ctrl+["));
      QVERIFY(toolTipString.contains("Ctrl+]"));
    }
  }
};

#include "NavigationAndAddressBarTest.moc"
REGISTER_TEST(NavigationAndAddressBarTest, false)
