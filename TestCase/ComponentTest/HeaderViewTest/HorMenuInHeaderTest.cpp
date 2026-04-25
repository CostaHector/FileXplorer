#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QSignalSpy>

#include "MemoryKey.h"
#include "Configuration.h"
#include "BeginToExposePrivateMember.h"
#include "HorMenuInHeader.h"
#include "EndToExposePrivateMember.h"

#include <QTableView>
#include <QStandardItemModel>
#include "ColumnVisibilityDialog.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class HorMenuInHeaderTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {  //
  }

  void cleanupTestCase() {  //
    Configuration().clear();
  }

  void init() {
    GlobalMockObject::reset();
    Configuration().clear();
  }
  void cleanup() {  //
    GlobalMockObject::verify();
  }

  void D0EV_ok() {
    {
      std::unique_ptr<HorMenuInHeader> horMenu{new HorMenuInHeader{"NoTitles_ok"}};
      QCOMPARE(horMenu->count(), 0);
      QCOMPARE(horMenu->isSortingEnabled(), true);

      QStringList titles = horMenu->getTitles();
      QCOMPARE(titles.size(), 0);
    }
    const QStringList keys = Configuration().allKeys();
    QVERIFY(keys.size() > 2);
  }

  void getTitles_ok() {  //
    QTableView tv;
    QStandardItemModel model(0, 3, &tv);
    model.setHorizontalHeaderLabels({"Name", "Country", "E-mail"});
    HorMenuInHeader horHeader{"getTitlesOk", &tv};
    QCOMPARE(horHeader.count(), 0);
    tv.setModel(&model);
    tv.setHorizontalHeader(&horHeader);
    QCOMPARE(horHeader.count(), 3);

    QStringList titles = horHeader.getTitles();
    QCOMPARE(titles, (QStringList{{"Name", "Country", "E-mail"}}));

    QString switches = horHeader.GetColumnsShowSwitch();
    const bool allColumnNotHide{switches.cend() == std::find(switches.cbegin(), switches.cend(), HorMenuInHeader::SW_OFF)};
    QVERIFY(allColumnNotHide);
    QCOMPARE(horHeader.isColumnHidden(0), false);
    QCOMPARE(horHeader.isColumnHidden(1), false);
    QCOMPARE(horHeader.isColumnHidden(2), false);
  }

  void onColumnVisibilityAdjust_ok() {
    QTableView tv;
    QStandardItemModel model(0, 3, &tv);
    model.setHorizontalHeaderLabels({"Name", "Country", "E-mail"});
    HorMenuInHeader horHeader{"getTitlesOk", &tv};
    QCOMPARE(horHeader.count(), 0);
    tv.setModel(&model);
    tv.setHorizontalHeader(&horHeader);
    QCOMPARE(horHeader.count(), 3);

    QSignalSpy reqUpdateColumnVisibiltySpy{&horHeader, &HorMenuInHeader::reqUpdateColumnVisibilty};

    MOCKER(ColumnVisibilityDialog::GetSwitches)                    //
        .expects(exactly(3))                                       //
        .will(returnValue(std::make_pair(QString{""}, false)))     // unchange
        .then(returnValue(std::make_pair(QString{"011"}, true)))   // yes 1
        .then(returnValue(std::make_pair(QString{"000"}, true)));  // yes 2

    {
      // unchange
      QString beforeSwitches{horHeader.m_columnsShowSwitch};
      QCOMPARE(horHeader.onColumnVisibilityAdjust(), false);
      QCOMPARE(reqUpdateColumnVisibiltySpy.count(), 0);
      QCOMPARE(horHeader.m_columnsShowSwitch, beforeSwitches);
    }

    {
      horHeader.m_columnsShowSwitch = "1111";  // 比3长
      QCOMPARE(horHeader.onColumnVisibilityAdjust(), true);
      QCOMPARE(reqUpdateColumnVisibiltySpy.count(), 1);
      reqUpdateColumnVisibiltySpy.takeLast();
      QCOMPARE(horHeader.m_columnsShowSwitch, "011");
    }

    {
      horHeader.m_columnsShowSwitch = "11";  // 比3短
      QCOMPARE(horHeader.onColumnVisibilityAdjust(), true);
      QCOMPARE(reqUpdateColumnVisibiltySpy.count(), 1);
      reqUpdateColumnVisibiltySpy.takeLast();
      QCOMPARE(horHeader.m_columnsShowSwitch, "000");
    }

    {  // all switch on
      horHeader.onShowAllColumns();
      QCOMPARE(horHeader.m_columnsShowSwitch, "111");
      QCOMPARE(reqUpdateColumnVisibiltySpy.count(), 1);
      reqUpdateColumnVisibiltySpy.takeLast();
    }
  }

  void onHideThisColumnTriggered_ok() {
    QTableView tv;
    QStandardItemModel model(0, 3, &tv);
    model.setHorizontalHeaderLabels({"Name", "Country", "E-mail"});
    HorMenuInHeader horHeader{"getTitlesOk", &tv};
    QCOMPARE(horHeader.count(), 0);
    tv.setModel(&model);
    tv.setHorizontalHeader(&horHeader);
    QCOMPARE(horHeader.count(), 3);

    QSignalSpy reqHideAColumnSpy{&horHeader, &HorMenuInHeader::reqHideAColumn};

    horHeader.onShowAllColumns();
    QVERIFY(horHeader.m_columnsShowSwitch.startsWith("111"));

    horHeader.InvalidateClickedSection();
    QCOMPARE(horHeader.onHideThisColumnTriggered(), false);
    QCOMPARE(reqHideAColumnSpy.count(), 0);

    QCOMPARE(horHeader.isColumnHidden(0), false);
    horHeader.setClickedSection(0);  // 选中0列
    QCOMPARE(horHeader.GetClickedSection(), 0);
    QCOMPARE(horHeader.onHideThisColumnTriggered(), true);  // 会清除记忆的列号
    QCOMPARE(horHeader.GetClickedSection(), HorMenuInHeader::INVALID_CLICKED_COLUMN);
    QCOMPARE(horHeader.isColumnHidden(0), true);
    QCOMPARE(reqHideAColumnSpy.count(), 1);
    QCOMPARE(reqHideAColumnSpy.takeLast(), (QVariantList{0, true}));  // 列0， 隐藏

    horHeader.setClickedSection(0);
    QCOMPARE(horHeader.onHideThisColumnTriggered(), false);  // unchange
    QCOMPARE(reqHideAColumnSpy.count(), 0);
  }
};

#include "HorMenuInHeaderTest.moc"
REGISTER_TEST(HorMenuInHeaderTest, false)
