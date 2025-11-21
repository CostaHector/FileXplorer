#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "HarTableView.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"
#include "ImageTestPrecoditionTools.h"

class HarTableViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
 private slots:
  void initTestCase() {
    HarFilesMocker::mockHarFiles().clear();  //
    HarTableViewMock::mockExportToPath().clear();
    QVERIFY(tDir.IsValid());

    auto* pHarView = new (std::nothrow) HarFiles;
    QVERIFY(pHarView != nullptr);
    delete pHarView;
  }

  void cleanupTestCase() {
    HarFilesMocker::mockHarFiles().clear();  //
    HarTableViewMock::mockExportToPath().clear();
  }

  void har_behavior_correct() {
    // data mock
    using namespace ImageTestPrecoditionTools;
    QString svgContent = GetSvgContentTemplate().arg(SVG_FILL_COLORS[0]).arg("svg");

    HarFiles& harInst = HarFilesMocker::mockHarFiles();
    harInst.mHarItems = {{"file1.txt", QByteArray("Content1"), "text/plain", "http://example.com/file1"},
                         {"image.png", QByteArray("invalid PNG content"), "image/png", "http://example.com/image"},
                         {"new added one.svg", svgContent.toUtf8(), "image/svg", "http://example.com/image"}};

    HarTableView harView;
    QVERIFY(harView.mHarModel != nullptr);
    QVERIFY(harView.mSortProxyModel != nullptr);
    QVERIFY(harView.mEXPORT_TO != nullptr);
    QVERIFY(harView.mQUICK_PREVIEW != nullptr);
    QVERIFY(harView.mPreviewLabel == nullptr);

    QCOMPARE(harView.mHarModel->rowCount(), 0);
    harView("ATestHarWith3Items.har");
    QCOMPARE(harView.mHarModel->rowCount(), 3);
    QVERIFY(harView.mQUICK_PREVIEW->isCheckable());

    // GetWinTitleStr should be ok, todo
    {
      QCOMPARE(harView.windowTitle(), harView.GetWinTitleStr("ATestHarWith3Items.har"));  //
      QCOMPARE(harView.windowTitle(), "Har Viewer | ATestHarWith3Items.har");
    }

    // expect should ok
    {
      // 1.0 has 3 selections to a inexist path, -1 returned
      harView.selectAll();
      HarTableViewMock::mockExportToPath() = "Path/to/an inexist path";
      QCOMPARE(harView.SaveSelectionFilesTo(), -1);

      // 2.0 to a valid path
      HarTableViewMock::mockExportToPath() = tDir.path();
      {
        // 2.1 harView has no selection
        harView.selectionModel()->clear();
        QCOMPARE(harView.SaveSelectionFilesTo(), 0);

        // 2.2 harView has 3 selections
        harView.selectAll();
        QCOMPARE(harView.SaveSelectionFilesTo(), 3);

        QSet<QString> expectSnapshotItems{"file1.txt", "image.png", "new added one.svg"};
        QCOMPARE(tDir.Snapshot(QDir::Filter::Files), expectSnapshotItems);
      }
    }

    {
      // sort by 2nd column File Type. "image/png" < "image/svg" < "text/plain"
      harView.sortByColumn(2, Qt::AscendingOrder);
      QModelIndex firstIndex = harView.mSortProxyModel->index(0, 0);
      QCOMPARE(firstIndex.data().toString(), "image.png");

      // sort by 1st column File Name. "file1.txt" < "image.png" < "new added one.svg"
      harView.sortByColumn(0, Qt::AscendingOrder);
      firstIndex = harView.mSortProxyModel->index(0, 0);
      QCOMPARE(firstIndex.data().toString(), "file1.txt");
    }

    {
      QVERIFY(harView.mPreviewLabel == nullptr);
      // PreviewImage switch on
      harView.mQUICK_PREVIEW->setChecked(true);
      emit harView.mQUICK_PREVIEW->toggled(true);

      // preview on text false, on invalid "image.png" false, on valid "new added one.svg" true
      QModelIndex sourceTxtIndex = harView.mHarModel->index(0, 0);
      QCOMPARE(sourceTxtIndex.data().toString(), "file1.txt");
      QModelIndex proxyTxtIndex = harView.mSortProxyModel->mapFromSource(sourceTxtIndex);
      harView.selectionModel()->setCurrentIndex(proxyTxtIndex, QItemSelectionModel::SelectCurrent);
      QVERIFY(!harView.PreviewImage(proxyTxtIndex, QModelIndex()));

      QModelIndex sourcePngIndex = harView.mHarModel->index(1, 0);
      QCOMPARE(sourcePngIndex.data().toString(), "image.png");
      QModelIndex proxyPngIndex = harView.mSortProxyModel->mapFromSource(sourcePngIndex);
      harView.selectionModel()->setCurrentIndex(proxyPngIndex, QItemSelectionModel::SelectCurrent);
      QVERIFY(!harView.PreviewImage(proxyPngIndex, QModelIndex()));

      QModelIndex sourceSvgIndex = harView.mHarModel->index(2, 0);
      QCOMPARE(sourceSvgIndex.data().toString(), "new added one.svg");
      QModelIndex proxySvgIndex = harView.mSortProxyModel->mapFromSource(sourceSvgIndex);
      QVERIFY(harView.PreviewImage(proxySvgIndex, QModelIndex()));
      QVERIFY(harView.mPreviewLabel != nullptr);

      // PreviewImage switch off
      harView.mQUICK_PREVIEW->setChecked(false);
      emit harView.mQUICK_PREVIEW->toggled(false);
      // preview on valid "new added one.svg" false
      QVERIFY(!harView.PreviewImage(proxySvgIndex, QModelIndex()));
    }

    harView.showEvent(nullptr);
    QShowEvent defaultShowEvent;
    harView.showEvent(&defaultShowEvent);

    harView.close();
  }
};

#include "HarTableViewTest.moc"
REGISTER_TEST(HarTableViewTest, false)
