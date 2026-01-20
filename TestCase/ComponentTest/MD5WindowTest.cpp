#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QTestEventList>

#include "BeginToExposePrivateMember.h"
#include "MD5Window.h"
#include "EndToExposePrivateMember.h"
#include <QUrl>
#include <QMimeData>

class MD5WindowTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  static int nonEmptyLineCount(const QString& content) {
    return content.split('\n', Qt::SplitBehaviorFlags::SkipEmptyParts).size();
  }

  void test_operator_call_ok() {
    MD5Window mw;
    QVERIFY(mw._SAMPLED_512_KB != nullptr);
    QCOMPARE(mw._SAMPLED_512_KB->isChecked(), true);
    QVERIFY(mw._MD5 != nullptr);
    QCOMPARE(mw._MD5->isChecked(), true);

    QStringList cppSourceFiles{__FILE__, __FILE__};
    QCOMPARE(mw.operator()(cppSourceFiles), 1); // first time, only 1 unique, 1 no need recalculate, 1 need
    QCOMPARE(mw.mPathsList.size(), 1);
    QCOMPARE(mw.mAlreadyCalculatedHashMap.size(), 1);
    QVERIFY(mw.m_md5TextEdit != nullptr);
    const QString contents1 = mw.m_md5TextEdit->toPlainText();
    QCOMPARE(nonEmptyLineCount(contents1), 2);

    // same abs file path add will be skipped
    QCOMPARE(mw.operator()(cppSourceFiles), 0); // again, +2 valid line, both of them no need recalculate
    QCOMPARE(mw.mPathsList.size(), 1);
    QCOMPARE(mw.mAlreadyCalculatedHashMap.size(), 1);
    const QString contents2 = mw.m_md5TextEdit->toPlainText();
    QCOMPARE(nonEmptyLineCount(contents2), 4);

    // hash algorithm changed. toggled signal will not trigger actionGroup in triggered
    mw._SHA1->setChecked(true);
    emit mw.mHashAlgIntAct.getActionGroup()->triggered(mw._SHA1); // +1 valid line, because there are only 1 element in mPathsList
    QCOMPARE(mw.mPathsList.size(), 1);
    QCOMPARE(mw.mAlreadyCalculatedHashMap.size(), 2);
    const QString contents3 = mw.m_md5TextEdit->toPlainText();
    QCOMPARE(nonEmptyLineCount(contents3), 5);

    mw._ONLY_FIRST_16_BYTES->setChecked(true);
    emit mw.mBytesRangeIntAct.getActionGroup()->triggered(mw._ONLY_FIRST_16_BYTES); // +1 valid line, because there are only 1 element in mPathsList
    QCOMPARE(mw.mPathsList.size(), 1);
    QCOMPARE(mw.mAlreadyCalculatedHashMap.size(), 3);
    const QString contents4 = mw.m_md5TextEdit->toPlainText();
    QCOMPARE(nonEmptyLineCount(contents4), 6);
  }

  void test_drop_event_ok() {
    MD5Window mw;
    const QPoint dropPos = mw.geometry().center();
    const QPoint dragEnterPos = dropPos;

    QMimeData emptyMimeData;
    emptyMimeData.setText("No urls");

    QMimeData urlsMimeData;
    urlsMimeData.setText("2 urls");
    QList<QUrl> urlsList{QUrl::fromLocalFile(__FILE__), QUrl::fromLocalFile(__FILE__)};
    urlsMimeData.setUrls(urlsList);

    QDragEnterEvent ignoreDragEnter(dragEnterPos, Qt::IgnoreAction, &emptyMimeData, Qt::LeftButton, Qt::NoModifier);
    mw.dragEnterEvent(&ignoreDragEnter);
    QCOMPARE(ignoreDragEnter.isAccepted(), false); // no urls, refuse drag enter

    QDragEnterEvent acceptDragEnter(dragEnterPos, Qt::IgnoreAction, &urlsMimeData, Qt::LeftButton, Qt::NoModifier);
    mw.dragEnterEvent(&acceptDragEnter);
    QCOMPARE(acceptDragEnter.isAccepted(), true); // has urls, accpet drag enter


    QDropEvent ignoreDropEvent(dropPos, Qt::IgnoreAction, &emptyMimeData, Qt::LeftButton, Qt::NoModifier);
    mw.dropEvent(&ignoreDropEvent); // invalid mimedata no need drop
    QCOMPARE(ignoreDropEvent.isAccepted(), false);
    QCOMPARE(mw.mPathsList.size(), 0);

    QDropEvent drop1stEvent(dropPos, Qt::IgnoreAction, &urlsMimeData, Qt::LeftButton, Qt::NoModifier);
    mw.dropEvent(&drop1stEvent); // first time drop
    QCOMPARE(drop1stEvent.isAccepted(), true);
    QCOMPARE(mw.mPathsList.size(), 1);
    QCOMPARE(mw.mAlreadyCalculatedHashMap.size(), 1);
    QVERIFY(mw.m_md5TextEdit != nullptr);
    const QString contents1 = mw.m_md5TextEdit->toPlainText();
    QCOMPARE(nonEmptyLineCount(contents1), 2);

    QDropEvent drop2ndEvent(dropPos, Qt::IgnoreAction, &urlsMimeData, Qt::LeftButton, Qt::NoModifier);
    mw.dropEvent(&drop2ndEvent); // again drop
    QCOMPARE(drop2ndEvent.isAccepted(), true);
    QCOMPARE(mw.mPathsList.size(), 1);
    QCOMPARE(mw.mAlreadyCalculatedHashMap.size(), 1);
    const QString contents2 = mw.m_md5TextEdit->toPlainText();
    QCOMPARE(nonEmptyLineCount(contents2), 4);

    mw.close();
  }

};

#include "MD5WindowTest.moc"
REGISTER_TEST(MD5WindowTest, false)
