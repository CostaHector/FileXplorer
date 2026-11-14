#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QSignalSpy>

#include "BeginToExposePrivateMember.h"
#include "AccountCardEditor.h"
#include "EndToExposePrivateMember.h"

#include <QPushButton>

class AccountCardEditorTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void stage_and_revert_signals_emitted_ok() {
    AccountCardEditor editor;

    QSignalSpy stageSpy(&editor, &AccountCardEditor::stageCurrentChanges);
    QSignalSpy revertSpy(&editor, &AccountCardEditor::revertCurrentChanges);
    QVERIFY(stageSpy.isValid());
    QVERIFY(revertSpy.isValid());
    QCOMPARE(stageSpy.count(), 0);
    QCOMPARE(revertSpy.count(), 0);

    // 从 editor 的子控件中按文本定位两个按钮
    QPushButton* stageButton{editor.mStageButton};
    QPushButton* revertButton{editor.mRevertButton};
    QVERIFY(stageButton != nullptr);
    QVERIFY(revertButton != nullptr);

    // 点击 Cancel 只触发 revertCurrentChanges
    revertButton->click();
    QCOMPARE(revertSpy.count(), 1);
    QCOMPARE(stageSpy.count(), 0);

    // 点击 Stage 只触发 stageCurrentChanges
    stageButton->click();
    QCOMPARE(stageSpy.count(), 1);
    QCOMPARE(revertSpy.count(), 1);
  }
};

#include "AccountCardEditorTest.moc"
REGISTER_TEST(AccountCardEditorTest, false)