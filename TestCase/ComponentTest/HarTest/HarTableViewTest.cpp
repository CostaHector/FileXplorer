#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "HarTableView.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"
#include "ImageTestPrecoditionTools.h"

#include <QFileDialog>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class HarTableViewTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir tDir;
  const QByteArray pngInvalidBa{"invalid PNG content"};
  const QString svgContentTemplate{ImageTestPrecoditionTools::GetSvgContentTemplate()      //
                                       .arg(ImageTestPrecoditionTools::SVG_FILL_COLORS[0]) //
                                       .arg("svg")};
  const QByteArray svgValidBa{svgContentTemplate.toUtf8()};

private slots:
  void initTestCase() { //
    QVERIFY(tDir.IsValid());
  }

  void cleanupTestCase() { //
    HarFilesMocker::mockHarFiles().clear();
  }

  void init() {
    GlobalMockObject::reset();
    HarFilesMocker::mockHarFiles().clear();
  }

  void cleanup() { //
    GlobalMockObject::verify();
  }

  void default_ok() {
    HarTableView harView;
    QVERIFY(harView.mHarModel != nullptr);
    QVERIFY(harView.mSortFilterProxy != nullptr);
    QVERIFY(harView.mEXPORT_TO != nullptr);

    QCOMPARE(harView.mHarModel->rowCount(), 0);

    QCOMPARE(harView.GetWinTitleStr(""), "Har Viewer");
    QCOMPARE(harView.GetWinTitleStr("aaa.har"), "Har Viewer | aaa.har");

    QSignalSpy windowTitleChangedSpy{&harView, &HarTableView::windowTitleChanged};
    QCOMPARE(harView.operator()("inexistHarFile.har"), 0);
    QCOMPARE(windowTitleChangedSpy.count(), 1);
    QCOMPARE(windowTitleChangedSpy.takeLast(), QVariantList{"Har Viewer | inexistHarFile.har"});

    harView.setFilter("Kaka");
  }

  void har_behavior_correct() {
    // data mock
    HarFiles& harInst = HarFilesMocker::mockHarFiles();
    harInst.mHarItems = {
        {"file1.txt", QByteArray("Content1"), "text/plain", "http://example.com/file1"}, //
        {"image.png", pngInvalidBa, "image/png", "http://example.com/image"},            //
        {"new added one.svg", svgValidBa, "image/svg", "http://example.com/image"},      //
    };

    HarTableView harView;
    QCOMPARE(harView.mHarModel->rowCount(), 0);
    harView("ATestHarWith3Items.har");
    QCOMPARE(harView.mHarModel->rowCount(), 3);

    // GetWinTitleStr should be ok, todo
    {
      // SaveSelectionFilesTo ok
      MOCKER(QFileDialog::getExistingDirectory)
          .expects(exactly(2))                                   //
          .will(returnValue(QString{"Path/to/an inexist path"})) //
          .then(returnValue(tDir.path()));

      // 1.0 has 3 selections to a inexist path, -1 returned
      harView.selectAll();
      QCOMPARE(harView.SaveSelectionFilesTo(), -1);

      // 2.0 to a valid path
      {
        // 2.1 but has no selection
        harView.selectionModel()->clear();
        QCOMPARE(harView.SaveSelectionFilesTo(), 0);

        // 2.2 and has 3 selections
        harView.selectAll();
        QCOMPARE(harView.SaveSelectionFilesTo(), 3);

        QSet<QString> expectSnapshotItems{"file1.txt", "image.png", "new added one.svg"};
        QCOMPARE(tDir.Snapshot(QDir::Filter::Files), expectSnapshotItems);
      }
    }

    {
      // Sort ok
      // sort by 2nd column File Type. "image/png" < "image/svg" < "text/plain"
      harView.sortByColumn(2, Qt::AscendingOrder);
      QModelIndex firstIndex = harView.mSortFilterProxy->index(0, 0);
      QCOMPARE(firstIndex.data().toString(), "image.png");

      // sort by 1st column File Name. "file1.txt" < "image.png" < "new added one.svg"
      harView.sortByColumn(0, Qt::AscendingOrder);
      firstIndex = harView.mSortFilterProxy->index(0, 0);
      QCOMPARE(firstIndex.data().toString(), "file1.txt");
    }

    {
      // filter ok
      // before: "file1.txt", "image.png", "new added one.svg"
      QCOMPARE(harView.mSortFilterProxy->rowCount(), 3);

      harView.setFilter("Kaka");
      QCOMPARE(harView.mSortFilterProxy->rowCount(), 0);

      harView.setFilter("new added one");
      QCOMPARE(harView.mSortFilterProxy->rowCount(), 1);

      harView.setFilter("");
      QCOMPARE(harView.mSortFilterProxy->rowCount(), 3);
    }

    {
      // PreviewImage ok

      QSignalSpy pixmapByteArrayChangedSpy{&harView, &HarTableView::pixmapByteArrayChanged};

      // preview on invalid index false
      QVERIFY(!harView.PreviewImage({}));
      QCOMPARE(pixmapByteArrayChangedSpy.count(), 0);

      // preview on text false, type not match
      QModelIndex sourceTxtIndex = harView.mHarModel->index(0, 0);
      QCOMPARE(sourceTxtIndex.data().toString(), "file1.txt");
      QModelIndex proxyTxtIndex = harView.mSortFilterProxy->mapFromSource(sourceTxtIndex);
      QVERIFY(!harView.PreviewImage(proxyTxtIndex));
      QCOMPARE(pixmapByteArrayChangedSpy.count(), 0);

      // on broken "image.png", also emit
      QModelIndex sourcePngIndex = harView.mHarModel->index(1, 0);
      QCOMPARE(sourcePngIndex.data().toString(), "image.png");
      QModelIndex proxyPngIndex = harView.mSortFilterProxy->mapFromSource(sourcePngIndex);
      QVERIFY(harView.PreviewImage(proxyPngIndex));
      QCOMPARE(pixmapByteArrayChangedSpy.count(), 1);
      QCOMPARE(pixmapByteArrayChangedSpy.takeLast(), (QVariantList{pngInvalidBa, QString{"png"}}));

      // on valid "new added one.svg" true
      QModelIndex sourceSvgIndex = harView.mHarModel->index(2, 0);
      QCOMPARE(sourceSvgIndex.data().toString(), "new added one.svg");
      QModelIndex proxySvgIndex = harView.mSortFilterProxy->mapFromSource(sourceSvgIndex);
      QVERIFY(harView.PreviewImage(proxySvgIndex));
      QCOMPARE(pixmapByteArrayChangedSpy.count(), 1);
      QCOMPARE(pixmapByteArrayChangedSpy.takeLast(), (QVariantList{svgValidBa, QString{"svg"}}));
    }
  }
};

#include "HarTableViewTest.moc"
REGISTER_TEST(HarTableViewTest, false)
