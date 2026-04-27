#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "DuplicatesImagesFinder.h"
#include "EndToExposePrivateMember.h"

#include "FileOpActs.h"

class DuplicatesImagesFinderTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void default_ok() {
    DuplicatesImagesFinder rif;

    QVERIFY(rif.m_toolBar != nullptr);
    QVERIFY(rif.m_table != nullptr);
    QVERIFY(rif.m_lo != nullptr);

    emit rif.m_table->windowTitleChanged("Hello world");
    QCOMPARE(rif.windowTitle(), "Hello world");

    QCOMPARE(rif("inexist/folder"), false);
    QCOMPARE(rif(""), false);

    rif.showEvent(nullptr); // not crash down
    rif.closeEvent(nullptr); // not crash down

    QShowEvent defaultShowEvent;
    rif.showEvent(&defaultShowEvent);

    QCloseEvent defaultCloseEvent;
    rif.closeEvent(&defaultCloseEvent);

    QCOMPARE(FileOpActs::GetInst()._DUPLICATE_IMAGES_FINDER->isChecked(), false);
  }
};

#include "DuplicatesImagesFinderTest.moc"
REGISTER_TEST(DuplicatesImagesFinderTest, false)
