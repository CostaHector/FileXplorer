#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include <QSignalSpy>

#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "DoubleRowHeader.h"
#include "EndToExposePrivateMember.h"

#include <QTableView>
#include <QStandardItemModel>

class DoubleRowHeaderTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void cleanupTestCase() {  //
    Configuration().clear();
  }

  void init() {  //
    Configuration().clear();
  }

  void D0EV_default_setting_saved_ok() {
    // 0列 禁用双列过滤->允许双列过滤
    {  //
      std::unique_ptr<DoubleRowHeader> header{new DoubleRowHeader{"SettingSavedOk"}};
      header->resizeEvent(nullptr);

      QResizeEvent rzEvent{{20, 10}, {10, 5}};
      header->resizeEvent(&rzEvent);

      QCOMPARE(header->_ENABLE_FILTERS->isChecked(), false);
      QCOMPARE(header->isFilterEnabled(), false);
      // sizeHeight * 2 ok
      const int beforeHeight = header->QHeaderView::sizeHint().height();
      QCOMPARE(header->sizeHint().height(), beforeHeight);

      header->_ENABLE_FILTERS->toggle();
      QCOMPARE(header->_ENABLE_FILTERS->isChecked(), true);
      QCOMPARE(header->isFilterEnabled(), true);
      QCOMPARE(header->sizeHint().height(), beforeHeight * 2);
    }

    QCOMPARE(Configuration().contains("SettingSavedOk"
                                      "_ENABLE_FILTERS"),
             true);
    QCOMPARE(Configuration()
                 .value("SettingSavedOk"
                        "_ENABLE_FILTERS")
                 .toBool(),
             true);
  }

  void InitFilterEditors_ok() {
    QTableView tv;
    QStandardItemModel model(0, 3, &tv);
    model.setHorizontalHeaderLabels({"Name", "Country", "E-mail"});
    DoubleRowHeader header{"InitFilterEditorsOk", &tv};
    QCOMPARE(header.count(), 0);
    tv.setModel(&model);
    tv.setHorizontalHeader(&header);
    QCOMPARE(header.count(), 3);

    header.m_columnsShowSwitch = "101";  // 设置中间列隐藏

    QSignalSpy searchStatementChangedSpy{&header, &DoubleRowHeader::searchStatementChanged};

    QVERIFY(!header.m_connSectionResized);
    QCOMPARE(header.isFilterEnabled(), false);
    QCOMPARE(header.m_filterEditors.size(), 0);

    // 初始化lineEdit, 信号槽链接ok
    header._ENABLE_FILTERS->toggle();
    QCOMPARE(header._ENABLE_FILTERS->isChecked(), true);
    QCOMPARE(header.isFilterEnabled(), true);
    QVERIFY(header.m_connSectionResized);
    QCOMPARE(header.m_filterEditors.size(), 3);
    QCOMPARE(header.m_filterEditors[0]->isHidden(), false);
    QCOMPARE(header.m_filterEditors[1]->isHidden(), true);
    QCOMPARE(header.m_filterEditors[2]->isHidden(), false);
    header.m_filterEditors[0]->setText("Search Kaka");
    emit header.m_filterEditors[0]->returnPressed();
    {
      QCOMPARE(searchStatementChangedSpy.count(), 1);
      QVariantList searchSqlParms = searchStatementChangedSpy.takeLast();
      QCOMPARE(searchSqlParms.size(), 1);
      QVariant searchSqlVar = searchSqlParms.front();
      QVERIFY(searchSqlVar.canConvert<QString>());
      QString searchSql = searchSqlVar.toString();
      QVERIFY(searchSql.contains("Kaka", Qt::CaseInsensitive));
    }

    // 信号槽断开
    header._ENABLE_FILTERS->toggle();
    QCOMPARE(header._ENABLE_FILTERS->isChecked(), false);
    QCOMPARE(header.isFilterEnabled(), false);
    QVERIFY(!header.m_connSectionResized);
    QCOMPARE(header.m_filterEditors[0]->isHidden(), true);
    QCOMPARE(header.m_filterEditors[1]->isHidden(), true);
    QCOMPARE(header.m_filterEditors[2]->isHidden(), true);

    header.m_filterEditors[0]->setText("Search Cristiano Ronaldo");
    emit header.m_filterEditors[0]->returnPressed();
    QCOMPARE(searchStatementChangedSpy.count(), 0);

    // 设置全部列显示, 信号槽链接ok
    header.m_columnsShowSwitch = "111";
    header._ENABLE_FILTERS->toggle();
    QCOMPARE(header._ENABLE_FILTERS->isChecked(), true);
    QCOMPARE(header.isFilterEnabled(), true);
    QVERIFY(header.m_connSectionResized);
    QCOMPARE(header.m_filterEditors[0]->isHidden(), false);
    QCOMPARE(header.m_filterEditors[1]->isHidden(), false);
    QCOMPARE(header.m_filterEditors[2]->isHidden(), false);
    header.m_filterEditors[0]->setText("Search Cristiano Ronaldo");
    emit header.m_filterEditors[0]->returnPressed();
    {
      QCOMPARE(searchStatementChangedSpy.count(), 1);
      QVariantList searchSqlParms = searchStatementChangedSpy.takeLast();
      QCOMPARE(searchSqlParms.size(), 1);
      QVariant searchSqlVar = searchSqlParms.front();
      QVERIFY(searchSqlVar.canConvert<QString>());
      QString searchSql = searchSqlVar.toString();
      QVERIFY(searchSql.contains("Cristiano Ronaldo", Qt::CaseInsensitive));
    }
  }
};

#include "DoubleRowHeaderTest.moc"
REGISTER_TEST(DoubleRowHeaderTest, false)
