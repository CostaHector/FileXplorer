#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "CastDBCardEditor.h"
#include "JsonFieldBoundary.h"

#include <QSignalSpy>
#include <QSpinBox>
#include <QPushButton>

    class CastDBCardEditorTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void stageCurrentChanges_revertCurrentChanges_ok() {
    CastDBCardEditor cardEditor;
    QVERIFY(cardEditor.mRateEdit != nullptr);
    cardEditor.mRateEdit->setValue(JsonFieldBoundary::MovieRateE::RATE_BUTT_V);
    QCOMPARE(cardEditor.mRateEdit->value(), JsonFieldBoundary::MovieRateE::RATE_MAX_V);
    QCOMPARE(cardEditor.mRateEdit->minimum(), JsonFieldBoundary::MovieRateE::RATE_MIN_UNINITIALIZED_V);
    QCOMPARE(cardEditor.mRateEdit->maximum(), JsonFieldBoundary::MovieRateE::RATE_MAX_V);

    QPushButton* pStageBtn = cardEditor.GetStageButton();
    QVERIFY(pStageBtn != nullptr);
    QSignalSpy stageCurrentChangesSpy{&cardEditor, &CastDBCardEditor::stageCurrentChanges};
    emit pStageBtn->clicked();
    QCOMPARE(stageCurrentChangesSpy.count(), 1);
    stageCurrentChangesSpy.clear();

    QPushButton* pRevertBtn = cardEditor.GetRevertButton();
    QVERIFY(pRevertBtn != nullptr);
    QSignalSpy revertCurrentChangesSpy{&cardEditor, &CastDBCardEditor::revertCurrentChanges};
    emit pRevertBtn->clicked();
    QCOMPARE(revertCurrentChangesSpy.count(), 1);
    revertCurrentChangesSpy.clear();
  }

};

#include "CastDBCardEditorTest.moc"
REGISTER_TEST(CastDBCardEditorTest, false)