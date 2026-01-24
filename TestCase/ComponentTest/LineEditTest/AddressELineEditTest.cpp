#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "BeginToExposePrivateMember.h"
#include "AddressELineEdit.h"
#include "EndToExposePrivateMember.h"
#include "PublicTool.h"
#include "ViewHelper.h"
#include <QMimeData>
#include <QUrl>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class AddressELineEditTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void initTestCase() { GlobalMockObject::reset(); }
  void cleanupTestCase() { GlobalMockObject::verify(); }

  void parentPath_ok() {
    const QString folderPath{"/path/to/destination/folder"};
    AddressELineEdit addLe;
    addLe.inputMode();
    addLe.m_pathComboBox->lineEdit()->setText(folderPath);
    QVERIFY(addLe.isInputMode());
    addLe.onReturnPressed();
    QVERIFY(addLe.isClickMode());

    bool bIsSame = false;
    QCOMPARE(addLe.parentPath(&bIsSame), "/path/to/destination");
    QVERIFY(!bIsSame);

    addLe.m_pathComboBox->lineEdit()->setText("");
    addLe.onReturnPressed();
    bIsSame = false;
    QCOMPARE(addLe.parentPath(&bIsSame), "");
    QVERIFY(bIsSame);
  }

  void returnPressed_actionTrigger_folder_ok() {
    // 文件夹, 会触发input切换到click模式, 并且修改toolbars中的actions
    const QString folderPath{"/path/to/destination/folder"};
    AddressELineEdit addLe;
    addLe.inputMode();
    addLe.m_pathComboBox->lineEdit()->setText(folderPath);
    QVERIFY(addLe.isInputMode());

    QSignalSpy addressToolBarSignal{&addLe, &AddressELineEdit::pathActionsTriggeredOrLineEditReturnPressed};
    addLe.onReturnPressed();
    QVERIFY(addLe.isClickMode());
    QCOMPARE(addLe.pathFromFullActions(), folderPath);
    QCOMPARE(addLe.m_pathComboBox->currentText(), folderPath);

    QCOMPARE(addressToolBarSignal.size(), 1);
    QVariantList parms0 = addressToolBarSignal.takeFirst();
    QCOMPARE(parms0.size(), 1);
    QCOMPARE(parms0.front().toString(), folderPath);

    QVERIFY(addLe.m_pathActionsTB != nullptr);
    QList<QAction *> acts = addLe.m_pathActionsTB->actions();
    QCOMPARE(acts.size(), 1 + 5); // computer, "", path, to, destination, folder

    addLe.onPathActionTriggered(acts.front());
    QCOMPARE(addressToolBarSignal.size(), 1);
    QVariantList parms1 = addressToolBarSignal.takeFirst();
    QCOMPARE(parms1.size(), 1);
    QCOMPARE(parms1.front().toString(), "");
    QList<QAction *> onlyComputerAct = addLe.m_pathActionsTB->actions();
    QCOMPARE(onlyComputerAct.size(), 1 + 1); // "Computer", ""
  }

  void returnPressed_file_ok() {
    // 文件, 不会触发input切换到click模式, 不会修改toolbars中的actions
    const QString filePath{__FILE__};
    MOCKER(FileTool::OpenLocalFile).expects(exactly(1)).with(eq(filePath)).will(returnValue(true));

    AddressELineEdit addLe;
    addLe.inputMode();
    QVERIFY(addLe.isInputMode());
    addLe.m_pathComboBox->lineEdit()->setText(filePath);

    QVERIFY(addLe.m_pathActionsTB != nullptr);
    QList<QAction *> beforeActs = addLe.m_pathActionsTB->actions();

    QSignalSpy addressToolBarSignal{&addLe, &AddressELineEdit::pathActionsTriggeredOrLineEditReturnPressed};
    addLe.onReturnPressed();
    QVERIFY(addLe.isInputMode());
    QCOMPARE(addLe.m_pathComboBox->currentText(), filePath);

    QCOMPARE(addressToolBarSignal.size(), 0);

    QList<QAction *> afterActs = addLe.m_pathActionsTB->actions();
    QCOMPARE(afterActs, beforeActs);
  }

  void escape_key_switch_to_clickMode_ok() {
    // Escape键入, 强制切换到ClickMode
    const QString filePath{__FILE__};
    AddressELineEdit addLe;
    addLe.inputMode();
    addLe.m_pathComboBox->lineEdit()->setText(filePath);
    QVERIFY(addLe.isInputMode());

    QKeyEvent escapeKeyEve{QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier, QString(), false, 1};
    addLe.keyPressEvent(&escapeKeyEve);
    QVERIFY(addLe.isClickMode());
  }

  void click_blank_zone_switch_to_inputMode_ok() {
    const QString folderPath{"/path/to/destination/folder"};
    AddressELineEdit addLe;
    addLe.inputMode();
    addLe.m_pathComboBox->lineEdit()->setText(folderPath);
    QVERIFY(addLe.isInputMode());
    emit addLe.m_pathComboBox->lineEdit()->returnPressed();
    QVERIFY(addLe.isClickMode());

    // 点击ToolBar的空白区域, 返回QAction=nullptr, 切换到InputMode
    QPoint outOfWidgetPoint{(int) 1E6, (int) 1E6};
    QMouseEvent blankZoneClicked{QEvent::MouseButtonPress, outOfWidgetPoint, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier};
    addLe.mousePressEvent(&blankZoneClicked);
    QVERIFY(addLe.isInputMode());

    // 当前已经是InputMode了, 继续点击空白区域, 预期无变化
    addLe.mousePressEvent(&blankZoneClicked);
    QVERIFY(addLe.isInputMode());
  }

  void dropEvent_ok() {
    // 预期可以拖拽, 实现 复制/移动/链接 功能
    const QString folderPath{"/path/to/destination/folder"};
    AddressELineEdit addLe;
    addLe.inputMode();
    addLe.m_pathComboBox->lineEdit()->setText(folderPath);
    QVERIFY(addLe.isInputMode());
    emit addLe.m_pathComboBox->lineEdit()->returnPressed();
    QVERIFY(addLe.isClickMode());

    MOCKER(View::onDropMimeData)
        .expects(exactly(1))                                 //
        .with(any(), Qt::DropAction::CopyAction, folderPath) //
        .will(returnValue(true));

    QMimeData urlsMimeData;
    urlsMimeData.setText("2 urls");
    QList<QUrl> urlsList{QUrl::fromLocalFile(__FILE__), QUrl::fromLocalFile(__FILE__)};
    urlsMimeData.setUrls(urlsList);

    QPoint outOfWidgetPoint{(int) 1E6, (int) 1E6}; // 全路径
    QDropEvent dropEvent{outOfWidgetPoint,
                         Qt::DropAction::MoveAction | Qt::DropAction::CopyAction | Qt::DropAction::LinkAction, //
                         &urlsMimeData,
                         Qt::LeftButton,
                         Qt::ControlModifier};
    addLe.dropEvent(&dropEvent);
  }

  void dragEnterEvent_ok() {
    // 预期拖拽进入时更改DropAction和外观到LinkAction
    const QString folderPath{"/path/to/destination/folder"};
    AddressELineEdit addLe;
    addLe.inputMode();
    addLe.m_pathComboBox->lineEdit()->setText(folderPath);
    QVERIFY(addLe.isInputMode());
    emit addLe.m_pathComboBox->lineEdit()->returnPressed();
    QVERIFY(addLe.isClickMode());

    QMimeData urlsMimeData;
    urlsMimeData.setText("2 urls");
    QList<QUrl> urlsList{QUrl::fromLocalFile(__FILE__), QUrl::fromLocalFile(__FILE__)};
    urlsMimeData.setUrls(urlsList);
    QPoint outOfWidgetPoint{(int) 1E6, (int) 1E6};                                                                       // 全路径
    QDragEnterEvent dragEnterEvent(outOfWidgetPoint,                                                                     //
                                   Qt::DropAction::MoveAction | Qt::DropAction::CopyAction | Qt::DropAction::LinkAction, //
                                   &urlsMimeData,                                                                        //
                                   Qt::MouseButton::LeftButton,                                                          //
                                   Qt::KeyboardModifier::AltModifier);
    addLe.dragEnterEvent(&dragEnterEvent);
    QVERIFY(dragEnterEvent.isAccepted());
    QCOMPARE(dragEnterEvent.dropAction(), Qt::DropAction::LinkAction);
  }

  void dragMoveEvent_ok() {
    // 预期可以拖拽时更改DropAction和外观到MoveAction
    const QString folderPath{"/path/to/destination/folder"};
    AddressELineEdit addLe;
    addLe.inputMode();
    addLe.m_pathComboBox->lineEdit()->setText(folderPath);
    QVERIFY(addLe.isInputMode());
    emit addLe.m_pathComboBox->lineEdit()->returnPressed();
    QVERIFY(addLe.isClickMode());

    // 1. 没有url, 忽略
    QMimeData urlsMimeData;
    QPoint outOfWidgetPoint{(int) 1E6, (int) 1E6}; // 全路径
    QDragMoveEvent dragEvent{outOfWidgetPoint,
                             Qt::DropAction::MoveAction | Qt::DropAction::CopyAction | Qt::DropAction::LinkAction, //
                             &urlsMimeData,
                             Qt::LeftButton,
                             Qt::KeyboardModifier::NoModifier};
    addLe.dragMoveEvent(&dragEvent);
    QVERIFY(!dragEvent.isAccepted());
    QVERIFY(dragEvent.dropAction() != Qt::DropAction::MoveAction);

    // 2. 有url, 成功修改为MoveAction
    urlsMimeData.setText("2 urls");
    QList<QUrl> urlsList{QUrl::fromLocalFile(__FILE__), QUrl::fromLocalFile(__FILE__)};
    urlsMimeData.setUrls(urlsList);
    addLe.dragMoveEvent(&dragEvent);
    QVERIFY(dragEvent.isAccepted());
    QCOMPARE(dragEvent.dropAction(), Qt::DropAction::MoveAction);
  }
};

#include "AddressELineEditTest.moc"
REGISTER_TEST(AddressELineEditTest, false)
