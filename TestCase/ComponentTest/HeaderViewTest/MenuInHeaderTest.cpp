#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "MemoryKey.h"
#include "Configuration.h"
#include "BeginToExposePrivateMember.h"
#include "MenuInHeader.h"
#include "EndToExposePrivateMember.h"
#include "InputDialogHelper.h"

#include <QStandardItemModel>
#include <QTableView>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class MenuInHeaderTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {  //
    Configuration().clear();
  }

  void cleanupTestCase() {  //
    Configuration().clear();
  }

  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void D0EV_save_configs_in_destructor_ok() {
    Configuration().clear();
    {  //
      std::unique_ptr<MenuInHeader> horHeader{new MenuInHeader{"DestructSavedSettingsOk", Qt::Orientation::Horizontal, nullptr}};
      QCOMPARE(horHeader->count(), 0);
      QVERIFY(horHeader->m_menu != nullptr);

      const int befCnt{horHeader->m_menu->actions().size()};
      horHeader->AddActionToMenu(nullptr);
      const int aftCnt = befCnt + 1;
      QCOMPARE(horHeader->m_menu->actions().size(), aftCnt);
    }
    QStringList keys = Configuration().allKeys();
    QVERIFY(keys.size() >= 5);
  }

  void setClickedSection_GetClickedSection_ok() {
    QTableView tv;

    QStandardItemModel model(0, 3, &tv);
    model.setHorizontalHeaderLabels({"Name", "Country", "E-mail"});
    MenuInHeader horHeader{"ClickedSectionGetterAndSetterOk", Qt::Orientation::Horizontal, &tv};
    QCOMPARE(horHeader.count(), 0);
    tv.setModel(&model);
    tv.setHorizontalHeader(&horHeader);
    QCOMPARE(horHeader.count(), 3);

    QCOMPARE(horHeader.GetClickedSection(), MenuInHeader::INVALID_CLICKED_COLUMN);

    horHeader.setClickedSection(0);
    QCOMPARE(horHeader.GetClickedSection(), 0);

    horHeader.InvalidateClickedSection();
    QCOMPARE(horHeader.GetClickedSection(), MenuInHeader::INVALID_CLICKED_COLUMN);

    // not crash down
    horHeader.contextMenuEvent(nullptr);

    // click header (0,0) update index
    const QPoint originPoint{0, 0};
    QContextMenuEvent event(QContextMenuEvent::Mouse, originPoint, horHeader.mapToGlobal(originPoint));
    horHeader.contextMenuEvent(&event);
    QVERIFY(event.isAccepted());
    QCOMPARE(horHeader.GetClickedSection(), 0);
  }

  void onSetSectionResizeMode_ok() {
    {
      QTableView tv;
      QStandardItemModel model(0, 1, &tv);
      model.setHorizontalHeaderLabels({"Name"});

      // 当前是ResizeToContents, 切换到别的场景时, 需要先恢复初始, 再切换
      MenuInHeader horHeader{"SetSectionResizeModeOk", Qt::Orientation::Horizontal, &tv};
      QCOMPARE(horHeader.count(), 0);
      tv.setModel(&model);
      tv.setHorizontalHeader(&horHeader);
      QCOMPARE(horHeader.count(), 1);

      horHeader.onSetSectionResizeMode(nullptr);

      horHeader.onSetSectionResizeMode(horHeader._RESIZE_MODE_FIXED);
      QVERIFY(!horHeader.onSetSectionResizeMode(horHeader._RESIZE_MODE_FIXED));  // unchange
      QCOMPARE(horHeader.sectionResizeMode(0), QHeaderView::ResizeMode::Fixed);

      QVERIFY(horHeader.onSetSectionResizeMode(horHeader._RESIZE_MODE_RESIZE_TO_CONTENTS));
      QCOMPARE(horHeader.sectionResizeMode(0), QHeaderView::ResizeMode::ResizeToContents);

      horHeader._RESIZE_MODE_INTERACTIVE->setChecked(true);
      emit horHeader.mResizeModeIntAction.getActionGroup()->triggered(horHeader._RESIZE_MODE_INTERACTIVE);
      QCOMPARE(horHeader.mResizeModeIntAction.curVal(), QHeaderView::ResizeMode::Interactive);
      QCOMPARE(horHeader.sectionResizeMode(0), QHeaderView::ResizeMode::Interactive);

      QVERIFY(!horHeader.onSetSectionResizeMode(horHeader._RESIZE_MODE_INTERACTIVE));  // unchange
    }

    QCOMPARE(Configuration()
                 .value("SetSectionResizeModeOk"
                        "_RESIZE_MODE")
                 .toInt(),
             ((int)QHeaderView::ResizeMode::Interactive));
  }

  void onSetDefaultSectionSize_ok() {
    {
      QTableView tv;
      QStandardItemModel model(0, 1, &tv);
      model.setHorizontalHeaderLabels({"Name"});

      MenuInHeader horHeader{"SetDefaultSectionSizeOk", Qt::Orientation::Horizontal, &tv};
      QCOMPARE(horHeader.count(), 0);
      tv.setModel(&model);
      tv.setHorizontalHeader(&horHeader);
      QCOMPARE(horHeader.count(), 1);

      int beforeValue = horHeader.defaultSectionSize();
      int afterValue = 99;
      QVERIFY(afterValue != beforeValue);

      MOCKER(InputDialogHelper::GetIntWithInitial)
          .expects(exactly(3))                                   //
          .will(returnValue(std::make_pair(false, 99999)))       // UserCancel0
          .then(returnValue(std::make_pair(true, beforeValue)))  // Yes but unchange
          .then(returnValue(std::make_pair(true, afterValue)));  // Yes and changed ok

      QCOMPARE(horHeader.onSetDefaultSectionSize(), false);  // UserCancel0
      QCOMPARE(horHeader.defaultSectionSize(), beforeValue);

      QCOMPARE(horHeader.onSetDefaultSectionSize(), false);  // Yes but unchange
      QCOMPARE(horHeader.defaultSectionSize(), beforeValue);

      QCOMPARE(horHeader.onSetDefaultSectionSize(), true);  // Yes and changed ok
      QCOMPARE(horHeader.defaultSectionSize(), afterValue);
    }
    QCOMPARE(Configuration()
                 .value("SetDefaultSectionSizeOk/DEFAULT_SECTION_SIZE")
                 .toInt(),
             99);
  }

  void onSetMaxSectionSize_ok() {
    {
      QTableView tv;
      QStandardItemModel model(0, 1, &tv);
      model.setHorizontalHeaderLabels({"Name"});

      MenuInHeader horHeader{"SetMaxSectionSizeOk", Qt::Orientation::Horizontal, &tv};
      QCOMPARE(horHeader.count(), 0);
      tv.setModel(&model);
      tv.setHorizontalHeader(&horHeader);
      QCOMPARE(horHeader.count(), 1);

      int beforeValue = horHeader.maximumSectionSize();
      int afterValue = 999;
      QVERIFY(afterValue != beforeValue);

      MOCKER(InputDialogHelper::GetIntWithInitial)
          .expects(exactly(3))                                   //
          .will(returnValue(std::make_pair(false, 99999)))       // UserCancel0
          .then(returnValue(std::make_pair(true, beforeValue)))  // Yes but unchange
          .then(returnValue(std::make_pair(true, afterValue)));  // Yes and changed ok

      QCOMPARE(horHeader.onSetMaxSectionSize(), false);  // UserCancel0
      QCOMPARE(horHeader.maximumSectionSize(), beforeValue);

      QCOMPARE(horHeader.onSetMaxSectionSize(), false);  // Yes but unchange
      QCOMPARE(horHeader.maximumSectionSize(), beforeValue);

      QCOMPARE(horHeader.onSetMaxSectionSize(), true);  // Yes and changed ok
      QCOMPARE(horHeader.maximumSectionSize(), afterValue);
    }
    QCOMPARE(Configuration()
                 .value("SetMaxSectionSizeOk/MAX_SECTION_SIZE")
                 .toInt(),
             999);
  }
};

#include "MenuInHeaderTest.moc"
REGISTER_TEST(MenuInHeaderTest, false)
