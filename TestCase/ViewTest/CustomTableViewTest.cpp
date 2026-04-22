#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"

#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "StyleKey.h"

#include "BeginToExposePrivateMember.h"
#include "CustomTableView.h"
#include "DoubleRowHeader.h"
#include "VerMenuInHeader.h"
#include "ScrollBarPolicyMenu.h"
#include "EndToExposePrivateMember.h"

#include "ModelTestHelper.h"
#include "AddressBarActions.h"
#include "ViewHelper.h"
#include "AddressBarActions.h"
#include "ViewActions.h"
#include "SizeTool.h"

#include "InputDialogHelper.h"

#include "MouseKeyboardEventHelper.h"
using namespace MouseKeyboardEventHelper;
using namespace SizeTool;
struct FileInfo {
  FileInfo(QString&& name_, QByteArray&& contents_, QString&& createTime_, QString&& fileExtenstion_)
    : name{std::move(name_)}
    , fileSize{contents_.size()}
    , contents{contents_}
    , createTime{std::move(createTime_)}
    , fileExtenstion{std::move(fileExtenstion_)} {}
  QString name;
  qint64 fileSize;
  QString createTime;
  QString fileExtenstion;
  QByteArray contents;
};

constexpr int NEW_BG_OVERLAY_OPACITY{249};

#define SIMPLE_TABLE_VIEW "SIMPLE_TABLE_VIEW"
class SimpleCustomTableView : public CustomTableView {
public:
  explicit SimpleCustomTableView(QStandardItemModel* pModel, QWidget* parent = nullptr)
    : CustomTableView{SIMPLE_TABLE_VIEW, parent} {
    setModel(pModel);
    InitTableView();
  }
};

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class CustomTableViewTest : public PlainTestSuite {
  Q_OBJECT
public:
  const QList<FileInfo> nodes{
      {"1.txt", "contents 333", "2025/11/20 00:00:00.001", "txt"}, //
      {"2.zip", "contents 22", "2025/11/20 00:00:00.000", "zip"},  //
      {"3.txt", "contents 1", "2025/11/20 00:00:00.003", "txt"},   //
  };
  //
  const QStringList mTitles{"Name", "Size", "Create Time", "File Extenstion"};
  static constexpr int EXPECT_COL_CNT = 4;
  void InitModel(QStandardItemModel& model) {
    model.setColumnCount(EXPECT_COL_CNT);
    for (const FileInfo& fi : nodes) {
      model.appendRow(QList<QStandardItem*>{
          {new QStandardItem{fi.name},                      //
           new QStandardItem{QString::number(fi.fileSize)}, //
           new QStandardItem{fi.createTime},                //
           new QStandardItem{fi.fileExtenstion}},           //
      });
    }
    model.setHorizontalHeaderLabels(mTitles);
  }
private slots:
  void initTestCase() { //
    QCOMPARE(nodes.size(), 3);
  }

  void cleanupTestCase() { //
    Configuration().clear();
  }

  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void QTableView_standardItemModel_ok() {
    Configuration().clear();

    QTableView tv;
    QVERIFY(tv.horizontalHeader() != nullptr);
    QVERIFY(tv.horizontalHeader()->model() == nullptr);

    QStandardItemModel model;
    InitModel(model);
    tv.setModel(&model);

    QCOMPARE(tv.horizontalHeader()->count(), EXPECT_COL_CNT);
    QVERIFY(tv.horizontalHeader()->model() != nullptr);
    QCOMPARE(tv.horizontalHeader()->model()->columnCount(), EXPECT_COL_CNT);

    tv.setSortingEnabled(false);
    QCOMPARE(model.index(0, 0).data(Qt::DisplayRole).toString(), "1.txt");
    QCOMPARE(model.index(1, 0).data(Qt::DisplayRole).toString(), "2.zip");
    QCOMPARE(model.index(2, 0).data(Qt::DisplayRole).toString(), "3.txt");

    tv.setSortingEnabled(true);
    tv.sortByColumn(0, Qt::SortOrder::DescendingOrder);
    QCOMPARE(model.index(0, 0).data(Qt::DisplayRole).toString(), "3.txt");
    QCOMPARE(model.index(1, 0).data(Qt::DisplayRole).toString(), "2.zip");
    QCOMPARE(model.index(2, 0).data(Qt::DisplayRole).toString(), "1.txt");

    tv.sortByColumn(2, Qt::SortOrder::AscendingOrder); // sort by size
    QCOMPARE(model.index(0, 0).data(Qt::DisplayRole).toString(), "2.zip");
    QCOMPARE(model.index(1, 0).data(Qt::DisplayRole).toString(), "1.txt");
    QCOMPARE(model.index(2, 0).data(Qt::DisplayRole).toString(), "3.txt");
    QCOMPARE(model.index(0, 2).data(Qt::DisplayRole).toString(), "2025/11/20 00:00:00.000");
    QCOMPARE(model.index(1, 2).data(Qt::DisplayRole).toString(), "2025/11/20 00:00:00.001");
    QCOMPARE(model.index(2, 2).data(Qt::DisplayRole).toString(), "2025/11/20 00:00:00.003");

    QCOMPARE(tv.verticalHeader()->count(), nodes.size());
    QVERIFY(tv.verticalHeader()->model() != nullptr);
  }

  void ShowOrHideColumnCore_ok() {
    QStandardItemModel model;
    SimpleCustomTableView tv{&model};
    QVERIFY(tv.m_horHeader != nullptr);

    {
      tv.m_horHeader->SetColumnsShowSwitch("");
      QCOMPARE(tv.m_horHeader->GetColumnsShowSwitch(), "");
      QCOMPARE(tv.ShowOrHideColumnCore(), false);
    }

    {
      // 0 columns
      QCOMPARE(model.columnCount(), 0);
      InitModel(model);
      // 4 columns
      QCOMPARE(model.columnCount(), 4);

      // switchs count: 1 should at least >= columns count: 4
      tv.m_horHeader->SetColumnsShowSwitch("1");
      QCOMPARE(tv.m_horHeader->GetColumnsShowSwitch(), "1");
      QCOMPARE(tv.ShowOrHideColumnCore(), false);

      tv.m_horHeader->SetColumnsShowSwitch("111111");
      QCOMPARE(tv.m_horHeader->GetColumnsShowSwitch(), "111111");
      QCOMPARE(tv.ShowOrHideColumnCore(), true);
    }
  }

  void CustomTableView_standardItemModel_ok() {
    Configuration().clear();

    QString headerStateKey;

    // 测试析构自动保存配置
    QMap<QString, QVariant> key2ExpectValue;
    {
      QStandardItemModel model;
      InitModel(model);

      SimpleCustomTableView tv{&model};
      QVERIFY(tv.m_horHeader != nullptr);
      QVERIFY(tv.m_verHeader != nullptr);
      tv.setModel(&model);
      tv.InitTableView();
      DoubleRowHeader& horHeader = *tv.m_horHeader;
      VerMenuInHeader& verHeader = *tv.m_verHeader;

      // empty configuration
      headerStateKey = horHeader.m_headerStateKey;
      // actions checkable property
      {
        // in tableview itself
        QVERIFY(tv._SHOW_HORIZONTAL_HEADER->isCheckable());
        QVERIFY(tv._SHOW_VERTICAL_HEADER->isCheckable());
        QVERIFY(!tv._RESIZE_ROW_TO_CONTENTS->isCheckable());
        QVERIFY(!tv._RESIZE_COLUMN_TO_CONTENTS->isCheckable());
        QVERIFY(tv._AUTO_SCROLL->isCheckable());
        QVERIFY(tv._ALTERNATING_ROW_COLORS->isCheckable());
        QVERIFY(tv._SHOW_GRID->isCheckable());
      }

      // tableview itself
      QVERIFY(!horHeader.isHidden());
      QVERIFY(!verHeader.isHidden());
      QVERIFY(tv._SHOW_HORIZONTAL_HEADER->isChecked());
      QVERIFY(tv._SHOW_VERTICAL_HEADER->isChecked());
      key2ExpectValue[tv.m_showHorizontalHeaderKey] = true;
      key2ExpectValue[tv.m_showVerticalHeaderKey] = true;

      QVERIFY(tv._AUTO_SCROLL->isChecked());
      QVERIFY(tv.hasAutoScroll());
      key2ExpectValue[tv.m_autoScrollKey] = true;

      QVERIFY(tv._ALTERNATING_ROW_COLORS->isChecked());
      QVERIFY(tv.alternatingRowColors());
      key2ExpectValue[tv.m_alternatingRowColorsKey] = true;

      QVERIFY(!tv._SHOW_GRID->isChecked());
      QVERIFY(!tv.showGrid());
      key2ExpectValue[tv.m_showGridKey] = false;

      QVERIFY(tv.m_horScrollBarPolicyMenu != nullptr);
      QVERIFY(tv.m_verScrollBarPolicyMenu != nullptr);
      QCOMPARE(tv.m_horScrollBarPolicyMenu->GetScrollBarPolicy(), Qt::ScrollBarPolicy::ScrollBarAsNeeded);
      QCOMPARE(tv.m_verScrollBarPolicyMenu->GetScrollBarPolicy(), Qt::ScrollBarPolicy::ScrollBarAsNeeded);
      key2ExpectValue[tv.m_horScrollBarPolicyMenu->GetName()] = Qt::ScrollBarPolicy::ScrollBarAsNeeded;
      key2ExpectValue[tv.m_verScrollBarPolicyMenu->GetName()] = Qt::ScrollBarPolicy::ScrollBarAsNeeded;

      QCOMPARE(horHeader.getTitles(), mTitles);

      // Default Section size
      LOG_D("horizontalHeader [%d, %d]", tv.horizontalHeader()->minimumSectionSize(), tv.horizontalHeader()->maximumSectionSize());
      LOG_D("verticalHeader [%d, %d]", tv.verticalHeader()->minimumSectionSize(), tv.verticalHeader()->maximumSectionSize());
      QCOMPARE(tv.horizontalHeader()->defaultSectionSize(), TABLE_DEFAULT_COLUMN_SECTION_SIZE);
      // QCOMPARE(tv.verticalHeader()->defaultSectionSize(), TABLE_DEFAULT_ROW_SECTION_SIZE);  //
      // don't check ver
      key2ExpectValue[horHeader.m_defaultSectionSizeKey] = TABLE_DEFAULT_COLUMN_SECTION_SIZE;
      // key2ExpectValue[verHeader.m_defaultSectionSizeKey] = TABLE_DEFAULT_ROW_SECTION_SIZE;
      // don't check ver

      QCOMPARE(tv.horizontalHeader()->maximumSectionSize(), TABLE_MAX_COLUMN_SECTION_SIZE);
      QCOMPARE(tv.verticalHeader()->maximumSectionSize(), TABLE_MAX_ROW_SECTION_SIZE);
      key2ExpectValue[horHeader.m_maxSectionSizeKey] = TABLE_MAX_COLUMN_SECTION_SIZE;
      key2ExpectValue[verHeader.m_maxSectionSizeKey] = TABLE_MAX_ROW_SECTION_SIZE;
    }

    for (auto it = key2ExpectValue.cbegin(); it != key2ExpectValue.cend(); ++it) {
      const QString itemName = it.key();
      QVERIFY2(Configuration().contains(itemName), qPrintable(itemName));
      QCOMPARE(Configuration().value(itemName), it.value());
    }
    QVERIFY(Configuration().contains(headerStateKey));
    QVERIFY(!Configuration().value(headerStateKey).toByteArray().isEmpty());

    // modify on the former ini file
    {
      QStandardItemModel model;
      InitModel(model);
      SimpleCustomTableView tv{&model};
      tv.setModel(&model);
      tv.InitTableView();
      DoubleRowHeader& horHeader = *tv.m_horHeader;
      VerMenuInHeader& verHeader = *tv.m_verHeader;

      tv._SHOW_HORIZONTAL_HEADER->setChecked(false);
      emit tv._SHOW_HORIZONTAL_HEADER->toggled(tv._SHOW_HORIZONTAL_HEADER->isChecked());
      QVERIFY(horHeader.isHidden());

      tv._SHOW_VERTICAL_HEADER->setChecked(false);
      emit tv._SHOW_VERTICAL_HEADER->toggled(tv._SHOW_VERTICAL_HEADER->isChecked());
      QVERIFY(verHeader.isHidden());
      key2ExpectValue[tv.m_showVerticalHeaderKey] = false;

      tv._SHOW_HORIZONTAL_HEADER->setChecked(true);
      emit tv._SHOW_HORIZONTAL_HEADER->toggled(tv._SHOW_HORIZONTAL_HEADER->isChecked());
      QVERIFY(!horHeader.isHidden());
      key2ExpectValue[tv.m_showHorizontalHeaderKey] = true;

      tv._AUTO_SCROLL->setChecked(false);
      emit tv._AUTO_SCROLL->toggled(tv._AUTO_SCROLL->isChecked());
      QVERIFY(!tv.hasAutoScroll());
      key2ExpectValue[tv.m_autoScrollKey] = false;

      tv._ALTERNATING_ROW_COLORS->setChecked(false);
      emit tv._ALTERNATING_ROW_COLORS->toggled(tv._ALTERNATING_ROW_COLORS->isChecked());
      QVERIFY(!tv.alternatingRowColors());
      key2ExpectValue[tv.m_alternatingRowColorsKey] = false;

      tv._SHOW_GRID->setChecked(true);
      emit tv._SHOW_GRID->toggled(tv._SHOW_GRID->isChecked());
      QVERIFY(tv.showGrid());
      key2ExpectValue[tv.m_showGridKey] = true;

      QCOMPARE(tv.horizontalScrollBarPolicy(), Qt::ScrollBarPolicy::ScrollBarAsNeeded);
      key2ExpectValue[tv.m_horScrollBarPolicyMenu->GetName()] = Qt::ScrollBarPolicy::ScrollBarAsNeeded;

      tv.m_verScrollBarPolicyMenu->_ALWAYS_ON->setChecked(true);
      emit tv.m_verScrollBarPolicyMenu->triggered(tv.m_verScrollBarPolicyMenu->_ALWAYS_ON);
      QCOMPARE(tv.verticalScrollBarPolicy(), Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
      key2ExpectValue[tv.m_verScrollBarPolicyMenu->GetName()] = Qt::ScrollBarPolicy::ScrollBarAlwaysOn;

      QCOMPARE(horHeader.getTitles(), mTitles);
    }

    for (auto it = key2ExpectValue.cbegin(); it != key2ExpectValue.cend(); ++it) {
      const QString itemName = it.key();
      QVERIFY2(Configuration().contains(itemName), qPrintable(itemName));
      QCOMPARE(Configuration().value(itemName), it.value());
    }

    // reopen it
    {
      QStandardItemModel model;
      InitModel(model);
      SimpleCustomTableView tv{&model};
      tv.setModel(&model);
      tv.InitTableView();

      // hor header: shown, ver header: hidden
      QVERIFY(!tv.hasAutoScroll());
      QVERIFY(!tv.alternatingRowColors());
      QVERIFY(tv.showGrid());
      QCOMPARE(tv.horizontalScrollBarPolicy(), Qt::ScrollBarPolicy::ScrollBarAsNeeded);
      QCOMPARE(tv.verticalScrollBarPolicy(), Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
    }
  }

  void saveSettings_ok() {
    const QString tableInstanceName{"saveSettingTest_ok"};
    {
      CustomTableView view{tableInstanceName};
      QVERIFY(view.m_defaultShowHorizontalHeader);
      QVERIFY(view.m_defaultShowVerticalHeader);
      QVERIFY(!view._SHOW_HORIZONTAL_HEADER->isChecked());
      QVERIFY(!view._SHOW_VERTICAL_HEADER->isChecked());
      QCOMPARE(view.m_horHeader->isHidden(), false);
      QCOMPARE(view.m_verHeader->isHidden(), false);

      // 默认行为不会修改缺省值
      view.initExclusivePreferenceSetting();
      QVERIFY(view.m_defaultShowHorizontalHeader);
      QVERIFY(view.m_defaultShowVerticalHeader);

      // 模拟子类override initExclusivePreferenceSetting body如下
      view.m_defaultShowHorizontalHeader = false;
      view.m_defaultShowVerticalHeader = false;

      view.m_bgOverlayOpacity = NEW_BG_OVERLAY_OPACITY;

      view.InitTableView();
      QVERIFY(!view.m_defaultShowHorizontalHeader);
      QVERIFY(!view.m_defaultShowVerticalHeader);
      QVERIFY(!view._SHOW_HORIZONTAL_HEADER->isChecked());
      QVERIFY(!view._SHOW_VERTICAL_HEADER->isChecked());

      QCOMPARE(view.m_horHeader->isHidden(), true);
      QCOMPARE(view.m_verHeader->isHidden(), true);
    }
    QVERIFY(Configuration().contains(tableInstanceName + "/SHOW_HORIZONTAL_HEADER"));
    QVERIFY(Configuration().contains(tableInstanceName + "/SHOW_VERTICAL_HEADER"));
    QVERIFY(Configuration().contains(tableInstanceName + "/AUTO_SCROLL"));
    QVERIFY(Configuration().contains(tableInstanceName + "/ALTERNATING_ROW_COLORS"));
    QVERIFY(Configuration().contains(tableInstanceName + "/SHOW_GRID"));

    QCOMPARE(Configuration().value(tableInstanceName + "/SHOW_HORIZONTAL_HEADER").toBool(), false);
    QCOMPARE(Configuration().value(tableInstanceName + "/SHOW_VERTICAL_HEADER").toBool(), false);
    QVERIFY(Configuration().contains(StyleKey::BACKGROUND_OVERLAY_OPACITY.name));
    QCOMPARE(Configuration().value(StyleKey::BACKGROUND_OVERLAY_OPACITY.name).toInt(), NEW_BG_OVERLAY_OPACITY);
  }

  void SetBgOverlayOpacity_ok() {
    Configuration().setValue(StyleKey::BACKGROUND_OVERLAY_OPACITY.name, NEW_BG_OVERLAY_OPACITY);

    std::pair<bool, int> cancel0{false, -999};
    std::pair<bool, int> accepet1Unchanged{true, NEW_BG_OVERLAY_OPACITY};
    std::pair<bool, int> accepet2Changed{true, NEW_BG_OVERLAY_OPACITY + 1};
    MOCKER(InputDialogHelper::GetIntWithInitial) //
        .expects(exactly(3))                     //
        .will(returnValue(cancel0))              // cancel
        .then(returnValue(accepet1Unchanged))    // accept but unchange
        .then(returnValue(accepet2Changed));     // accept and changed

    CustomTableView view("CustomTableViewSetBgOverlayOpacity");
    QCOMPARE(view.m_bgOverlayOpacity, NEW_BG_OVERLAY_OPACITY);

    view._BG_OVERLAY_OPACITY->trigger(); // cancel
    QCOMPARE(view.m_bgOverlayOpacity, NEW_BG_OVERLAY_OPACITY);

    QVERIFY(!view.SetBgOverlayOpacity()); // accept but unchange
    QCOMPARE(view.m_bgOverlayOpacity, NEW_BG_OVERLAY_OPACITY);

    QVERIFY(view.SetBgOverlayOpacity()); // accept and changed
    QCOMPARE(view.m_bgOverlayOpacity, NEW_BG_OVERLAY_OPACITY + 1);
  }

  void override_member_function_ok() {
    CustomTableView view("CustomTableView_override_member_function");

    view.contextMenuEvent(nullptr); // not crash down
    {
      QPoint centerPnt{view.geometry().center()};
      QContextMenuEvent validMenuEvent(QContextMenuEvent::Mouse, centerPnt, view.mapToGlobal(centerPnt));
      view.contextMenuEvent(&validMenuEvent);
      QCOMPARE(validMenuEvent.isAccepted(), true);
    }

    view.paintEvent(nullptr);
    {
      QPaintEvent validPaintEvent{QRect{0, 0, 1, 1}};
      view.paintEvent(&validPaintEvent);
    }

    view.scrollContentsBy(0, 0);
  }

  void mouseSideClick_NavigationSignals() {
    CustomTableView view("CustomTableViewMouseSideKey");

    auto& addressInst = g_addressBarActions();
    auto& viewInst = g_viewActions();

    QSignalSpy backAddressSpy(addressInst._BACK_TO, &QAction::triggered);
    QSignalSpy forwardAddressSpy(addressInst._FORWARD_TO, &QAction::triggered);
    QSignalSpy backViewSpy(viewInst._VIEW_BACK_TO, &QAction::triggered);
    QSignalSpy forwardViewSpy(viewInst._VIEW_FORWARD_TO, &QAction::triggered);

    { // accepted events
      QVERIFY(SendMousePressEvent<CustomTableView>(view, Qt::BackButton, Qt::NoModifier));
      QCOMPARE(backAddressSpy.count(), 1);

      QVERIFY(SendMousePressEvent<CustomTableView>(view, Qt::ForwardButton, Qt::NoModifier));
      QCOMPARE(forwardAddressSpy.count(), 1);

      QVERIFY(SendMousePressEvent<CustomTableView>(view, Qt::BackButton, Qt::ControlModifier));
      QCOMPARE(backViewSpy.count(), 1);

      QVERIFY(SendMousePressEvent<CustomTableView>(view, Qt::ForwardButton, Qt::ControlModifier));
      QCOMPARE(forwardViewSpy.count(), 1);
    }

    // Alt+back: nothing happen
    {
      SendMousePressEvent<CustomTableView>(view, Qt::BackButton, Qt::AltModifier);
      QCOMPARE(backViewSpy.count(), 1);
      QCOMPARE(backAddressSpy.count(), 1);
    }

    // left click: nothing happen
    {
      SendMousePressEvent<CustomTableView>(view, Qt::LeftButton, Qt::NoModifier);
      QCOMPARE(backAddressSpy.count(), 1);
      QCOMPARE(forwardViewSpy.count(), 1);
    }

    // all signal params ok
    QVERIFY(backAddressSpy.count() > 0);
    QVERIFY(forwardAddressSpy.count() > 0);
    QVERIFY(backViewSpy.count() > 0);
    QVERIFY(forwardViewSpy.count() > 0);

    QCOMPARE(backAddressSpy.back()[0].toBool(), false);
    QCOMPARE(forwardAddressSpy.back()[0].toBool(), false);
    QCOMPARE(backViewSpy.back()[0].toBool(), false);
    QCOMPARE(forwardViewSpy.back()[0].toBool(), false);
  }
};

#include "CustomTableViewTest.moc"
REGISTER_TEST(CustomTableViewTest, false)
