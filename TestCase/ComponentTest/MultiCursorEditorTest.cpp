#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "MultiCursorEditor.h"
#include "EndToExposePrivateMember.h"

class MultiCursorEditorTest : public PlainTestSuite {
  Q_OBJECT
 public:
  const QString mPlainText{
      R"(Hello world!
Nice to meet you!
Bye
)"};
 private slots:
  void default_ok() {
    MultiCursorEditor mce;
    QVERIFY(!mce.mBlinkTimer.isActive());
    QCOMPARE(mce.mStatus, MultiCursorEditor::SelectStatus::INIT);

    {  // not crash down
      mce.mousePressEvent(nullptr);
      mce.mouseMoveEvent(nullptr);
      mce.mouseReleaseEvent(nullptr);
      mce.paintEvent(nullptr);
      mce.keyPressEvent(nullptr);
      mce.toggleCursors();
      mce.toggleCursors();
    }

    mce.setPlainText(mPlainText);
    QString initialText = mce.toPlainText();
    QCOMPARE(initialText, mPlainText);

    // 直接设置光标在最开始部分, 预期:删除按键先删除'H'
    const QString expectFirstCharDeleted(R"(ello world!
Nice to meet you!
Bye
)");
    QCOMPARE(mce.textCursor().position(), 0);
    QKeyEvent deleteHEvent(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
    mce.keyPressEvent(&deleteHEvent);
    QCOMPARE(mce.toPlainText(), expectFirstCharDeleted);

    // 直接设置光标在最末尾部分, 预期:删除按键先删除'\n', 再删除'e'‵
    QTextCursor cursor = mce.textCursor();
    cursor.movePosition(QTextCursor::End);
    mce.setTextCursor(cursor);
    QCOMPARE(mce.textCursor().position(), expectFirstCharDeleted.size());

    const QString expectLast2CharsDeleted(R"(ello world!
Nice to meet you!
By)");

    QKeyEvent backspaceEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
    mce.keyPressEvent(&backspaceEvent);
    mce.keyPressEvent(&backspaceEvent);
    QCOMPARE(mce.toPlainText(), expectLast2CharsDeleted);

    QCOMPARE(mce.textCursor().position(), expectLast2CharsDeleted.size());
  }

  void top2Botton_multiline_delete_backspace_insert_escape_ok() {
    MultiCursorEditor mce;
    QVERIFY(!mce.mBlinkTimer.isActive());
    QCOMPARE(mce.mStatus, MultiCursorEditor::SelectStatus::INIT);

    mce.setPlainText(mPlainText);
    QString initialText = mce.toPlainText();
    QCOMPARE(initialText, mPlainText);
    QCOMPARE(mce.textCursor().position(), 0);

    // 获取第一行和第二行行首的坐标
    QTextCursor cursor = mce.textCursor();
    cursor.movePosition(QTextCursor::Start);
    QRect firstLineRect = mce.cursorRect(cursor);
    QPoint firstPos = firstLineRect.center();

    cursor.movePosition(QTextCursor::Down);
    cursor.movePosition(QTextCursor::StartOfLine);
    QRect secondLineRect = mce.cursorRect(cursor);
    QPoint secondPos = secondLineRect.center();

    // 先鼠标按在首行首列前, 按下Alt不放, 鼠标垂直向下拖动到第二行, 预期同时选中了前两行, 闪烁定时器活跃

    // 鼠标按在第一行行首
    QMouseEvent pressEvent(QEvent::MouseButtonPress, firstPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mce.mousePressEvent(&pressEvent);

    // 按下Alt键进入"准备多行选择模式"
    QKeyEvent altPressEvent(QEvent::KeyPress, Qt::Key_Alt, Qt::NoModifier);
    mce.keyPressEvent(&altPressEvent);
    QCOMPARE(mce.mStatus, MultiCursorEditor::SelectStatus::PREPARING);

    // 鼠标拖动到第二行
    QMouseEvent moveEvent(QEvent::MouseMove, secondPos, Qt::NoButton, Qt::NoButton, Qt::AltModifier);
    mce.mouseMoveEvent(&moveEvent);

    // 鼠标释放，进入"多行选择模式"
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, secondPos, Qt::LeftButton, Qt::NoButton, Qt::AltModifier);
    mce.mouseReleaseEvent(&releaseEvent);
    QCOMPARE(mce.mStatus, MultiCursorEditor::SelectStatus::MULTI_LINE_SELECTING);
    QCOMPARE(mce.mCursors.size(), 2);
    QCOMPARE(mce.mBlinkTimer.isActive(), true);

    // 按下字母'X', 预期前两行首个位置都插入了X
    const QString expectXCharInsertFirst2Lines(R"(XHello world!
XNice to meet you!
Bye
)");
    QKeyEvent xInsert2LinesEvent(QEvent::KeyPress, Qt::Key_X, Qt::NoModifier, "X");
    mce.keyPressEvent(&xInsert2LinesEvent);
    QCOMPARE(mce.toPlainText(), expectXCharInsertFirst2Lines);

    // 按下Backspace, 预期前两行首个位置的X被删除, 内容恢复到初始
    QKeyEvent backSpace2LinesXEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
    mce.keyPressEvent(&backSpace2LinesXEvent);
    QCOMPARE(mce.toPlainText(), mPlainText);

    // 按下Key_Delete, 预期前两行首个位置的H, N分别被删除
    const QString expectFirst2LineFirstCharDeleted(R"(ello world!
ice to meet you!
Bye
)");
    QKeyEvent delete2LinesEvent(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
    mce.keyPressEvent(&delete2LinesEvent);
    QCOMPARE(mce.toPlainText(), expectFirst2LineFirstCharDeleted);

    // 按下Escape, 退出了多行模式, 预期光标在首行首列0位置
    QKeyEvent backspaceEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    mce.keyPressEvent(&backspaceEvent);
    QCOMPARE(mce.textCursor().position(), 0);
    QCOMPARE(mce.mCursors.isEmpty(), true);
    QCOMPARE(mce.mBlinkTimer.isActive(), false);

    // 按下字母'a', 预期
    const QString expectACharInsert1Line(R"(aello world!
ice to meet you!
Bye
)");
    QKeyEvent charAEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, "a");
    mce.keyPressEvent(&charAEvent);
    QCOMPARE(mce.toPlainText(), expectACharInsert1Line);
  }

  void botton2top_multiline_insert_escape_ok() {
    MultiCursorEditor mce;
    QVERIFY(!mce.mBlinkTimer.isActive());
    QCOMPARE(mce.mStatus, MultiCursorEditor::SelectStatus::INIT);

    mce.setPlainText(mPlainText);
    QString initialText = mce.toPlainText();
    QCOMPARE(initialText, mPlainText);
    QCOMPARE(mce.textCursor().position(), 0);

    // 获取第一行和第二行行首的坐标
    QTextCursor cursor = mce.textCursor();
    cursor.movePosition(QTextCursor::Start);
    QRect firstLineRect = mce.cursorRect(cursor);
    QPoint firstPos = firstLineRect.center();

    cursor.movePosition(QTextCursor::Down);
    cursor.movePosition(QTextCursor::StartOfLine);
    QRect secondLineRect = mce.cursorRect(cursor);
    QPoint secondPos = secondLineRect.center();

    // 先鼠标按在第二行行首前, 按下Alt不放, 鼠标垂直向上拖动到第一行行首, 预期同时选中了前两行, 闪烁定时器活跃

    // 鼠标按在第一行行首
    QMouseEvent pressEvent(QEvent::MouseButtonPress, secondPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mce.mousePressEvent(&pressEvent);

    // 按下Alt键进入"准备多行选择模式"
    QKeyEvent altPressEvent(QEvent::KeyPress, Qt::Key_Alt, Qt::NoModifier);
    mce.keyPressEvent(&altPressEvent);
    QCOMPARE(mce.mStatus, MultiCursorEditor::SelectStatus::PREPARING);

    // 鼠标拖动到第二行行首
    QMouseEvent moveEvent(QEvent::MouseMove, firstPos, Qt::NoButton, Qt::NoButton, Qt::AltModifier);
    mce.mouseMoveEvent(&moveEvent);

    // 鼠标释放，进入"多行选择模式"
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, firstPos, Qt::LeftButton, Qt::NoButton, Qt::AltModifier);
    mce.mouseReleaseEvent(&releaseEvent);
    QCOMPARE(mce.mStatus, MultiCursorEditor::SelectStatus::MULTI_LINE_SELECTING);
    QCOMPARE(mce.mCursors.size(), 2);
    QCOMPARE(mce.mBlinkTimer.isActive(), true);

    // 按下字母'X', 预期前两行首个位置都插入了X
    const QString expectXCharInsertFirst2Lines(R"(XHello world!
XNice to meet you!
Bye
)");
    QKeyEvent xInsert2LinesEvent(QEvent::KeyPress, Qt::Key_X, Qt::NoModifier, "X");
    mce.keyPressEvent(&xInsert2LinesEvent);
    QCOMPARE(mce.toPlainText(), expectXCharInsertFirst2Lines);

    QKeyEvent xInsert2LinesEventTextEmpty(QEvent::KeyPress, Qt::Key_X, Qt::NoModifier);
    mce.keyPressEvent(&xInsert2LinesEventTextEmpty);
    QCOMPARE(mce.toPlainText(), expectXCharInsertFirst2Lines); // not insert, don't know why


    // 按下Escape, 退出了多行模式, 预期光标在第二行X,N之间
    const int expectPos = (QString{R"(XHello world!
X)"}).size();
    QKeyEvent backspaceEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    mce.keyPressEvent(&backspaceEvent);
    QCOMPARE(mce.textCursor().position(), expectPos);
    QCOMPARE(mce.mCursors.isEmpty(), true);
    QCOMPARE(mce.mBlinkTimer.isActive(), false);
  }
};

#include "MultiCursorEditorTest.moc"
REGISTER_TEST(MultiCursorEditorTest, false)
