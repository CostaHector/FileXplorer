#include <QtTest/QtTest>
#include <QSignalSpy>
#include "PlainTestSuite.h"
#include "Logger.h"
#include "TDir.h"
#include "BeginToExposePrivateMember.h"
#include "AddressELineEdit.h"
#include "EndToExposePrivateMember.h"
#include <QToolButton>

class AddressELineEditTest : public PlainTestSuite {
  Q_OBJECT
public:
  AddressELineEditTest() : PlainTestSuite{} {
    static int objectCnt = 0;
    fprintf(stdout, "AddressELineEditTest object[%d] created\n", objectCnt++);
    std::fflush(stdout);
  }
  ~AddressELineEditTest() {}
  const QString mTEST_PATH = QFileInfo{PathTool::normPath(__FILE__)}.absolutePath();  // The delimeter is always slash/
  const QStringList mTEST_PATH_PART_LIST = mTEST_PATH.split(PathTool::PATH_SEP_CHAR);
private slots:

  void test_NormToolBarActionPath_ok() {
#ifdef _WIN32
    // Disk A to Disk Z
    QCOMPARE(AddressELineEdit::NormToolBarActionPath("/A:"),  "A:");
    QCOMPARE(AddressELineEdit::NormToolBarActionPath("/C:"),  "C:");
    QCOMPARE(AddressELineEdit::NormToolBarActionPath("/Z:"),  "Z:");
    QCOMPARE(AddressELineEdit::NormToolBarActionPath("/XX:"), "XX:");
    QCOMPARE(AddressELineEdit::NormToolBarActionPath(""), ""); // "" in windows is root
#else
    QCOMPARE(AddressELineEdit::NormToolBarActionPath(""), "");
    QCOMPARE(AddressELineEdit::NormToolBarActionPath("/"), "/");
    QCOMPARE(AddressELineEdit::NormToolBarActionPath("/home"), "/home");
    QCOMPARE(AddressELineEdit::NormToolBarActionPath("/home/user"), "/home/user");
#endif
  }

  void test_initial_state_ok() {
    AddressELineEdit addressLe;
    QVERIFY(addressLe.m_pathActionsTB != nullptr);
    QVERIFY(addressLe.m_pathComboBox != nullptr);
    QCOMPARE(addressLe.currentWidget(), addressLe.m_pathActionsTB);
    QCOMPARE(addressLe.m_pathComboBox->isEditable(), true);
    QVERIFY(addressLe.m_pathComboBox->lineEdit() != nullptr);
    addressLe.show();
    QVERIFY(QTest::qWaitForWindowActive(&addressLe));

    const QRect toolBarGeometry = addressLe.m_pathActionsTB->geometry();
    addressLe.updateAddressToolBarPathActions(mTEST_PATH);
    QList<QAction*> actsLst = addressLe.m_pathActionsTB->actions();
    for (QAction* pAct : actsLst) {  // QToolButton height should expand to its parent QToolBar
      if (pAct != nullptr) {
        QRect actionRect = addressLe.m_pathActionsTB->actionGeometry(pAct);
        QVERIFY(toolBarGeometry.top() - actionRect.top() <= 3);
        QVERIFY(actionRect.bottom() - toolBarGeometry.bottom() <= 3);
      }
    }
  }

  void test_changePathInvalid() {
    AddressELineEdit addressLe;
    addressLe.ChangePath("/home/to/inexists path");
    QCOMPARE(addressLe.m_pathComboBox->currentText(), "/home/to/inexists path");
  }

  void test_clickToolBarBlankArea_ToggleInto_ComboBoxEditMode_ok() {
    AddressELineEdit addressLe;
    addressLe.show();
    QVERIFY(QTest::qWaitForWindowActive(&addressLe));

    addressLe.updateAddressToolBarPathActions(mTEST_PATH);
    QCOMPARE(addressLe.pathFromFullActions(), mTEST_PATH);
    QCOMPARE(addressLe.currentWidget(), addressLe.m_pathActionsTB);  // Before: QToolBar Click Mode

    // QToolBar space area is near right edge
    QRect toolbarRect = addressLe.m_pathActionsTB->rect();
    QPoint blankPoint(toolbarRect.right() - 10, toolbarRect.center().y());
    QVERIFY(addressLe.m_pathActionsTB->actionAt(blankPoint) == nullptr);

    // click space area in QToolBar
    QTest::mouseClick(addressLe.m_pathActionsTB, Qt::LeftButton, Qt::NoModifier, blankPoint);

    QCOMPARE(addressLe.currentWidget(), addressLe.m_pathComboBox);  // After: QComboBox Edit Mode
    QVERIFY(addressLe.m_pathComboBox->hasFocus());
    QCOMPARE(addressLe.pathFromFullActions(), mTEST_PATH);  // path unchange

    // addressLe.clickMode(); // way 2: change back to click mode, now call destructor is ok
    // Or [recommend] disconnect signal connect in destructor to prevents signals reaching partially-destroyed objects
  }

  void test_precondition_split_behavior_ok() {
    // precondition:
    QCOMPARE(QString{"/home/to"}.split(PathTool::PATH_SEP_CHAR), (QStringList() << ""
                                                                                << "home"
                                                                                << "to"));
    QCOMPARE(QString{"C:/home/to"}.split(PathTool::PATH_SEP_CHAR), (QStringList() << "C:"
                                                                                  << "home"
                                                                                  << "to"));
  }

  void test_clickPathAction_RemainsInClickMode_and_PathChanged_ok() {
    AddressELineEdit addressLe;

    // Set test path
    QVERIFY(mTEST_PATH_PART_LIST.size() >= 2);

    addressLe.updateAddressToolBarPathActions(mTEST_PATH);  // Before: QToolBar Click Mode
    QCOMPARE(addressLe.m_pathComboBox->currentText(), mTEST_PATH);

    // Ensure toolbar has actions
    QToolBar* tb = addressLe.m_pathActionsTB;
    const QList<QAction*> actsLst = tb->actions();
    QVERIFY(!actsLst.isEmpty());
    QAction* firstAct = actsLst[0];  // for linux, index 0 is ""
    QVERIFY(firstAct != nullptr);
    QCOMPARE(firstAct->text(), "");

    QToolButton* firstButton = qobject_cast<QToolButton*>(tb->widgetForAction(firstAct));  // Click toolbutton directly instead of QToolBar!
    QVERIFY(firstButton != nullptr);

    addressLe.show();
    addressLe.activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(&addressLe));

    QSignalSpy spyToolBarActionTriggered(tb, &QToolBar::actionTriggered);
    QTest::mouseClick(firstButton, Qt::LeftButton, Qt::NoModifier, {}, 50);
    QTRY_COMPARE(spyToolBarActionTriggered.count(), 1);  // should emit m_pathActionsTB.triggered(firstAction)!

    QList<QVariant> actionTriggeredParams = spyToolBarActionTriggered.last();
    QCOMPARE(actionTriggeredParams.size(), 1);
    QVariant actVariant = actionTriggeredParams.front();
    const QString paraTypeName = actVariant.typeName();
    QCOMPARE(paraTypeName, "QAction*");
    auto* pTriggeredAct = actVariant.value<QAction*>();
    QCOMPARE(pTriggeredAct, firstAct);

    QCOMPARE(addressLe.currentWidget(), addressLe.m_pathActionsTB);  // After: Still QToolBar Click Mode
    QCOMPARE(addressLe.m_pathComboBox->currentText(), "");
    QCOMPARE(addressLe.pathFromFullActions(), "");
  }

  void test_EscapeKey_ToggleBackTo_ToolbarClickMode_ok() {
    AddressELineEdit addressLe;
    addressLe.show();
    addressLe.updateAddressToolBarPathActions(mTEST_PATH);  // Before: QToolBar Click Mode
    QCOMPARE(addressLe.m_pathComboBox->currentText(), mTEST_PATH);
    QVERIFY(QTest::qWaitForWindowActive(&addressLe));

    // Precondition: Switch to edit mode
    addressLe.inputMode();
    QCOMPARE(addressLe.currentWidget(), addressLe.m_pathComboBox);

    // Press Escape key
    QTest::keyClick(addressLe.m_pathComboBox, Qt::Key_Escape, Qt::NoModifier, 50);

    // Verify back to toolbar mode, Path Unchange
    QTRY_COMPARE(addressLe.currentWidget(), addressLe.m_pathActionsTB);
    QCOMPARE(addressLe.m_pathComboBox->currentText(), mTEST_PATH);
  }

  void test_ReturnPressed_AfterPathInput_Result_ClickMode_and_PathChanged_ok() {
    AddressELineEdit addressLe;
    addressLe.show();
    QVERIFY(QTest::qWaitForWindowActive(&addressLe));

    // Precondition: Switch to edit mode
    addressLe.inputMode();
    // Path Input and return pressed
    addressLe.m_pathComboBox->setCurrentText(mTEST_PATH);
    QCOMPARE(addressLe.pathFromFullActions(), "");  // path empty now
    QCOMPARE(addressLe.m_pathComboBox->lineEdit()->text(), mTEST_PATH);
    emit addressLe.m_pathComboBox->lineEdit()->returnPressed();

    // Click Mode and Path Changed
    QCOMPARE(addressLe.currentWidget(), addressLe.m_pathActionsTB);
    QCOMPARE(addressLe.pathFromFullActions(), mTEST_PATH);
  }

  void test_simple_drag_drop_cut_and_copy_ok() {
    const QList<FsNodeEntry> entries{{"subfolder/henry cavill need copy.txt", false, "0123456789"}};
    TDir tdir;
    QCOMPARE(tdir.createEntries(entries), 1);
    const QString subPath = tdir.itemPath("subfolder");
    QString srcFilePath = tdir.itemPath("subfolder/henry cavill need copy.txt");
    QString dstFilePath = tdir.itemPath("henry cavill need copy.txt");

    AddressELineEdit addressLe;
    addressLe.updateAddressToolBarPathActions(subPath);  // 进入到tdir/subfolder路径下
    addressLe.move(0, 0);
    addressLe.adjustSize();
    addressLe.show();
    addressLe.activateWindow();
    QTRY_VERIFY(QTest::qWaitForWindowActive(&addressLe));

    QList<QAction*> actsLst = addressLe.m_pathActionsTB->actions();
    QVERIFY(actsLst.size() > 2);

    QAction* lastAct = actsLst[actsLst.size() - 1];
    QAction* second2LastAct = actsLst[actsLst.size() - 2];
    QVERIFY(lastAct != nullptr);
    QCOMPARE(lastAct->text(), "subfolder");
    QVERIFY(second2LastAct != nullptr);

    auto simulateDragEnterMoveDrop = [](QToolBar* pTb, const QPoint pnt, QMimeData* pMimeData, Qt::KeyboardModifier modifier) -> bool {
      // 1. 模拟dragEnter事件
      QDragEnterEvent dragEnterEvent(pnt, Qt::CopyAction | Qt::MoveAction | Qt::LinkAction, pMimeData, Qt::LeftButton, modifier);
      QApplication::sendEvent(pTb, &dragEnterEvent);
      if (!dragEnterEvent.isAccepted()) {
        return false;
      }

      // 2. 模拟dragMove事件
      QDragMoveEvent dragMoveEvent(pnt, Qt::CopyAction | Qt::MoveAction | Qt::LinkAction, pMimeData, Qt::LeftButton, modifier);
      QApplication::sendEvent(pTb, &dragMoveEvent);
      if (!dragMoveEvent.isAccepted()) {
        return false;
      }

      // 3. 模拟drop事件
      QDropEvent dropEvent(pnt, Qt::CopyAction | Qt::MoveAction | Qt::LinkAction, pMimeData, Qt::LeftButton, modifier);
      QApplication::sendEvent(pTb, &dropEvent);
      if (!dropEvent.isAccepted()) {
        return false;
      }
      return true;
    };

    // 1. MOVE "henry cavill need copy.txt" FROM subfolder TO tdir(second2LastAct)
    {
      QMimeData moveMimeData;
      moveMimeData.setUrls({QUrl::fromLocalFile(srcFilePath)});
      const QRect second2LastRect = addressLe.m_pathActionsTB->actionGeometry(second2LastAct);
      const QPoint cutDropPosition = second2LastRect.center();
      QCOMPARE(second2LastAct, addressLe.m_pathActionsTB->actionAt(cutDropPosition));
      QCOMPARE(addressLe.pathFromCursorAction(second2LastAct).contains("subfolder"), false);

      QCOMPARE(simulateDragEnterMoveDrop(addressLe.m_pathActionsTB, cutDropPosition, &moveMimeData, Qt::KeyboardModifier::NoModifier), true);
      QVERIFY(QFile::exists(dstFilePath));
      QVERIFY(!QFile::exists(srcFilePath));  // move not copy, so src file not exist
      QCOMPARE(QFile{dstFilePath}.size(), 10);
    }

    // 2. COPY "henry cavill need copy.txt" FROM tdir TO subfolder(lastAct)
    srcFilePath.swap(dstFilePath);
    {
      QMimeData copyMimeData;
      copyMimeData.setUrls({QUrl::fromLocalFile(srcFilePath)});
      const QRect lastRect = addressLe.m_pathActionsTB->actionGeometry(lastAct);
      const QPoint copyDropPosition = lastRect.center();
      QCOMPARE(lastAct, addressLe.m_pathActionsTB->actionAt(copyDropPosition));
      QCOMPARE(addressLe.pathFromCursorAction(lastAct).contains("subfolder"), true);

      QCOMPARE(simulateDragEnterMoveDrop(addressLe.m_pathActionsTB, copyDropPosition, &copyMimeData, Qt::KeyboardModifier::ControlModifier), true);
      QVERIFY(QFile::exists(dstFilePath));
      QVERIFY(QFile::exists(srcFilePath));  // copy not move, so src file remains unchaneg
      QCOMPARE(QFile{dstFilePath}.size(), 10);
      QCOMPARE(QFile{srcFilePath}.size(), 10);
    }
  }
};

#include "AddressELineEditTest.moc"
REGISTER_TEST(AddressELineEditTest, false)
