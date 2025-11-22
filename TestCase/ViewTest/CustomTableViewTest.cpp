#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"

#include "BeginToExposePrivateMember.h"
#include "CustomTableView.h"
#include "DoubleRowHeader.h"
#include "VerMenuInHeader.h"
#include "EndToExposePrivateMember.h"

#include "ModelTestHelper.h"
#include "AddressBarActions.h"
#include "ViewHelper.h"
#include "AddressBarActions.h"
#include "ViewActions.h"

#include "MouseKeyboardEventHelper.h"
using namespace MouseKeyboardEventHelper;

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

#define SIMPLE_TABLE_VIEW "SIMPLE_TABLE_VIEW"
class SimpleCustomTableView : public CustomTableView {
public:
  explicit SimpleCustomTableView(QStandardItemModel* pModel, QWidget* parent = nullptr)
    : CustomTableView{SIMPLE_TABLE_VIEW, parent} {
    setModel(pModel);
    InitTableView(true);
  }

private:
};

class CustomTableViewTest : public PlainTestSuite {
  Q_OBJECT
public:
  const QList<FileInfo> nodes{
      {"1.txt", "contents 333", "2025/11/20 00:00:00.001", "txt"}, //
      {"2.zip", "contents 22", "2025/11/20 00:00:00.000", "zip"},  //
      {"3.txt", "contents 1", "2025/11/20 00:00:00.003", "txt"},   //
  };
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
    UserSpecifiedIntValueMock::mockColumnsShowSwitch() = HorMenuInHeader::DEFAULT_SWITCHES();
  }

  void cleanupTestCase() { //
    Configuration().clear();
    UserSpecifiedIntValueMock::mockColumnsShowSwitch() = HorMenuInHeader::DEFAULT_SWITCHES();
  }

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

    QCOMPARE(tv.verticalHeader()->count(), nodes.size());
    QVERIFY(tv.verticalHeader()->model() != nullptr);
  }

  void CustomTableView_standardItemModel_ok() {
    Configuration().clear();

    QString headerStateKey;

    QMap<QString, QVariant> key2ExpectValue;
    {
      QStandardItemModel model;
      InitModel(model);

      SimpleCustomTableView tv{&model};
      QVERIFY(tv.m_horHeader != nullptr);
      QVERIFY(tv.m_verHeader != nullptr);
      tv.setModel(&model);
      tv.InitTableView(true);
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

        // in MenuInHeader
        QVERIFY(!horHeader._SET_DEFAULT_SECTION_SIZE->isCheckable());
        QVERIFY(!horHeader._SET_MAX_SECTION_SIZE->isCheckable());
        QVERIFY(horHeader._STRETCH_DETAIL_SECTION->isCheckable());
        QVERIFY(horHeader._RESIZE_MODE_INTERACTIVE->isCheckable());
        QVERIFY(horHeader._RESIZE_MODE_STRETCH->isCheckable());
        QVERIFY(horHeader._RESIZE_MODE_FIXED->isCheckable());
        QVERIFY(horHeader._RESIZE_MODE_RESIZE_TO_CONTENTS->isCheckable());
        QCOMPARE(horHeader.mResizeModeIntAction.getActionGroup()->exclusionPolicy(), QActionGroup::ExclusionPolicy::Exclusive);

        // in HorMenuInHeader
        QVERIFY(!horHeader._COLUMNS_VISIBILITY->isCheckable());
        QVERIFY(!horHeader._HIDE_THIS_COLUMN->isCheckable());
        QVERIFY(horHeader._ENABLE_COLUMN_SORT->isCheckable());

        // in DoubleRowHeader
        QVERIFY(horHeader._ENABLE_FILTERS->isCheckable());
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

      // column visibility switches = all `1`, no column hidden, but filter disabled
      const QString expectAllShownSwitches{HorMenuInHeader::DEFAULT_SWITCHES()};
      QVERIFY(expectAllShownSwitches.size() > 0);
      QVERIFY(!expectAllShownSwitches.contains(HorMenuInHeader::SW_OFF));
      QCOMPARE(horHeader.m_columnsShowSwitch, expectAllShownSwitches);
      QVERIFY(!horHeader.isSectionHidden(0));
      QVERIFY(!horHeader.isSectionHidden(1));
      QVERIFY(!horHeader.isSectionHidden(2));
      QVERIFY(!horHeader.isSectionHidden(3));
      QCOMPARE(horHeader.m_columnsShowSwitch, expectAllShownSwitches);
      QVERIFY(!horHeader.isFilterEnabled());
      QVERIFY(horHeader.m_filterEditors.isEmpty());
      key2ExpectValue[horHeader.m_enableFilterKey] = false;
      key2ExpectValue[horHeader.m_columnVisibiltyKey] = expectAllShownSwitches;

      // sorting, Header ResizeMode, Strech last section, titles
      QVERIFY(horHeader.isSortingEnabled());
      QVERIFY(tv.isSortingEnabled());
      key2ExpectValue[horHeader.m_sortByColumnSwitchKey] = true;

      QVERIFY(horHeader._RESIZE_MODE_INTERACTIVE->isChecked());
      QVERIFY(verHeader._RESIZE_MODE_INTERACTIVE->isChecked());
      QCOMPARE(horHeader.sectionResizeMode(0), QHeaderView::ResizeMode::Interactive);
      QCOMPARE(verHeader.sectionResizeMode(0), QHeaderView::ResizeMode::Interactive);
      key2ExpectValue[horHeader.m_resizeModeKey] = (int) QHeaderView::ResizeMode::Interactive;
      key2ExpectValue[verHeader.m_resizeModeKey] = (int) QHeaderView::ResizeMode::Interactive;

      QVERIFY(!horHeader._STRETCH_DETAIL_SECTION->isChecked());
      QVERIFY(!verHeader._STRETCH_DETAIL_SECTION->isChecked());
      QVERIFY(!horHeader.stretchLastSection());
      QVERIFY(!verHeader.stretchLastSection());
      key2ExpectValue[horHeader.m_stretchLastSectionKey] = false;
      key2ExpectValue[verHeader.m_stretchLastSectionKey] = false;

      QCOMPARE(horHeader.getTitles(), mTitles);

      // Default Section size
      LOG_D("horizontalHeader [%d, %d]", tv.horizontalHeader()->minimumSectionSize(), tv.horizontalHeader()->maximumSectionSize());
      LOG_D("verticalHeader [%d, %d]", tv.verticalHeader()->minimumSectionSize(), tv.verticalHeader()->maximumSectionSize());
      QCOMPARE(tv.horizontalHeader()->defaultSectionSize(), MenuInHeader::TABLE_DEFAULT_COLUMN_SECTION_SIZE);
      // QCOMPARE(tv.verticalHeader()->defaultSectionSize(), MenuInHeader::TABLE_DEFAULT_ROW_SECTION_SIZE);  //
      // don't check ver
      key2ExpectValue[horHeader.m_defaultSectionSizeKey] = MenuInHeader::TABLE_DEFAULT_COLUMN_SECTION_SIZE;
      // key2ExpectValue[verHeader.m_defaultSectionSizeKey] = MenuInHeader::TABLE_DEFAULT_ROW_SECTION_SIZE;
      // don't check ver

      QCOMPARE(tv.horizontalHeader()->maximumSectionSize(), MenuInHeader::TABLE_MAX_COLUMN_SECTION_SIZE);
      QCOMPARE(tv.verticalHeader()->maximumSectionSize(), MenuInHeader::TABLE_MAX_ROW_SECTION_SIZE);
      key2ExpectValue[horHeader.m_maxSectionSizeKey] = MenuInHeader::TABLE_MAX_COLUMN_SECTION_SIZE;
      key2ExpectValue[verHeader.m_maxSectionSizeKey] = MenuInHeader::TABLE_MAX_ROW_SECTION_SIZE;
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
      tv.InitTableView(true);
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

      // all column/filter shown
      QVERIFY(!horHeader.isSectionHidden(0));
      QVERIFY(!horHeader.isSectionHidden(1));
      QVERIFY(!horHeader.isSectionHidden(2));
      QVERIFY(!horHeader.isSectionHidden(3));
      horHeader._ENABLE_FILTERS->setChecked(true);
      QSignalSpy reqParentTableUpdateGeometriesSpy{&horHeader, &DoubleRowHeader::reqParentTableUpdateGeometries};
      emit horHeader._ENABLE_FILTERS->toggled(horHeader._ENABLE_FILTERS->isChecked());
      QCOMPARE(reqParentTableUpdateGeometriesSpy.count(), 1);
      QVERIFY(reqParentTableUpdateGeometriesSpy.takeLast().isEmpty()); // expect no params at all
      QCOMPARE(horHeader.m_filterEditors.size(), EXPECT_COL_CNT);
      QVERIFY(!horHeader.m_filterEditors[0]->isHidden());
      QVERIFY(!horHeader.m_filterEditors[1]->isHidden());
      QVERIFY(!horHeader.m_filterEditors[2]->isHidden());
      QVERIFY(!horHeader.m_filterEditors[3]->isHidden());
      key2ExpectValue[horHeader.m_enableFilterKey] = true;

      // only index=0 column/filter hide
      QPoint pnt{2, 2};
      QContextMenuEvent cme{QContextMenuEvent::Reason::Mouse, pnt};
      horHeader.contextMenuEvent(&cme);
      // or mock here horHeader.setClickedSection(0);
      QCOMPARE(horHeader.m_clickedColumn, 0);
      emit horHeader._HIDE_THIS_COLUMN->triggered();
      QVERIFY(horHeader.isSectionHidden(0));
      QVERIFY(!horHeader.isSectionHidden(1));
      QVERIFY(!horHeader.isSectionHidden(2));
      QVERIFY(!horHeader.isSectionHidden(3));
      QVERIFY(horHeader.m_clickedColumn < 0); // should invalidate right after hide action triggered
      QCOMPARE(horHeader.m_filterEditors.size(), EXPECT_COL_CNT);
      QVERIFY(horHeader.m_filterEditors[0]->isHidden());
      QVERIFY(!horHeader.m_filterEditors[1]->isHidden());
      QVERIFY(!horHeader.m_filterEditors[2]->isHidden());
      QVERIFY(!horHeader.m_filterEditors[3]->isHidden());
      const QString column0HideSws{QChar{'0'} + horHeader.m_columnsShowSwitch.mid(1)};
      QCOMPARE(horHeader.m_columnsShowSwitch, column0HideSws);
      key2ExpectValue[horHeader.m_columnVisibiltyKey] = column0HideSws;

      // all column/filter hide
      const QString allHideSws(column0HideSws.size(), QChar{'0'});
      UserSpecifiedIntValueMock::mockColumnsShowSwitch() = allHideSws;
      emit horHeader._COLUMNS_VISIBILITY->triggered(false);
      QVERIFY(horHeader.isSectionHidden(0));
      QVERIFY(horHeader.isSectionHidden(1));
      QVERIFY(horHeader.isSectionHidden(2));
      QVERIFY(horHeader.isSectionHidden(3));
      QCOMPARE(horHeader.m_filterEditors.size(), EXPECT_COL_CNT);
      QVERIFY(horHeader.m_filterEditors[0]->isHidden());
      QVERIFY(horHeader.m_filterEditors[1]->isHidden());
      QVERIFY(horHeader.m_filterEditors[2]->isHidden());
      QVERIFY(horHeader.m_filterEditors[3]->isHidden());
      QCOMPARE(horHeader.m_columnsShowSwitch, allHideSws);
      key2ExpectValue[horHeader.m_columnVisibiltyKey] = allHideSws;

      // only index=0 column/filter hide
      UserSpecifiedIntValueMock::mockColumnsShowSwitch() = column0HideSws;
      emit horHeader._COLUMNS_VISIBILITY->triggered(false);
      QVERIFY(horHeader.isSectionHidden(0));
      QVERIFY(!horHeader.isSectionHidden(1));
      QVERIFY(!horHeader.isSectionHidden(2));
      QVERIFY(!horHeader.isSectionHidden(3));
      QCOMPARE(horHeader.m_filterEditors.size(), EXPECT_COL_CNT);
      QVERIFY(horHeader.m_filterEditors[0]->isHidden());
      QVERIFY(!horHeader.m_filterEditors[1]->isHidden());
      QVERIFY(!horHeader.m_filterEditors[2]->isHidden());
      QVERIFY(!horHeader.m_filterEditors[3]->isHidden());
      QCOMPARE(horHeader.m_columnsShowSwitch, column0HideSws);
      key2ExpectValue[horHeader.m_columnVisibiltyKey] = column0HideSws;

      horHeader._ENABLE_COLUMN_SORT->setChecked(false);
      emit horHeader._ENABLE_COLUMN_SORT->toggled(horHeader._ENABLE_COLUMN_SORT->isChecked());
      QVERIFY(!horHeader.isSortingEnabled());
      QVERIFY(!tv.isSortingEnabled());
      key2ExpectValue[horHeader.m_sortByColumnSwitchKey] = false;

      horHeader._RESIZE_MODE_RESIZE_TO_CONTENTS->setChecked(true);
      emit horHeader.mResizeModeIntAction.getActionGroup()->triggered(horHeader._RESIZE_MODE_RESIZE_TO_CONTENTS);
      QCOMPARE(horHeader.sectionResizeMode(0), QHeaderView::ResizeMode::ResizeToContents);
      key2ExpectValue[horHeader.m_resizeModeKey] = (int) QHeaderView::ResizeMode::ResizeToContents;

      horHeader._STRETCH_DETAIL_SECTION->setChecked(true);
      emit horHeader._STRETCH_DETAIL_SECTION->toggled(horHeader._STRETCH_DETAIL_SECTION->isChecked());
      QVERIFY(horHeader.stretchLastSection());
      QVERIFY(!verHeader.stretchLastSection());
      key2ExpectValue[horHeader.m_stretchLastSectionKey] = true;
      key2ExpectValue[verHeader.m_stretchLastSectionKey] = false;

      UserSpecifiedIntValueMock::MockQInputDialogGetInt(false, 66);
      emit horHeader._SET_DEFAULT_SECTION_SIZE->triggered(false);
      QVERIFY(horHeader.defaultSectionSize() != 66);
      UserSpecifiedIntValueMock::MockQInputDialogGetInt(true, 66);
      emit horHeader._SET_DEFAULT_SECTION_SIZE->triggered(false);
      key2ExpectValue[horHeader.m_defaultSectionSizeKey] = 66;
      QCOMPARE(horHeader.defaultSectionSize(), 66);

      UserSpecifiedIntValueMock::MockQInputDialogGetInt(false, 888);
      emit horHeader._SET_MAX_SECTION_SIZE->triggered(false);
      QVERIFY(horHeader.maximumSectionSize() != 888);
      UserSpecifiedIntValueMock::MockQInputDialogGetInt(true, 888);
      emit horHeader._SET_MAX_SECTION_SIZE->triggered(false);
      QCOMPARE(horHeader.maximumSectionSize(), 888);
      key2ExpectValue[horHeader.m_maxSectionSizeKey] = 888;

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
      tv.InitTableView(true);
      DoubleRowHeader& horHeader = *tv.m_horHeader;
      VerMenuInHeader& verHeader = *tv.m_verHeader;

      // hor header: shown, ver header: hidden
      QVERIFY(!horHeader.isHidden());
      QVERIFY(verHeader.isHidden());
      QVERIFY(tv._SHOW_HORIZONTAL_HEADER->isChecked());
      QVERIFY(!tv._SHOW_VERTICAL_HEADER->isChecked());

      QVERIFY(!horHeader.isSortingEnabled());
      QVERIFY(!tv.isSortingEnabled());
      QVERIFY(!tv.hasAutoScroll());
      QVERIFY(!tv.alternatingRowColors());
      QVERIFY(tv.showGrid());
      QCOMPARE(tv.horizontalScrollBarPolicy(), Qt::ScrollBarPolicy::ScrollBarAsNeeded);
      QCOMPARE(tv.verticalScrollBarPolicy(), Qt::ScrollBarPolicy::ScrollBarAlwaysOn);

      // filter should enabled
      QVERIFY(horHeader.isFilterEnabled());
      QCOMPARE(horHeader.m_filterEditors.size(), EXPECT_COL_CNT);
      // filter linedit all shown except column=0
      QVERIFY(horHeader.isSectionHidden(0));
      QVERIFY(!horHeader.isSectionHidden(1));
      QVERIFY(!horHeader.isSectionHidden(2));
      QVERIFY(!horHeader.isSectionHidden(3));
      QVERIFY(horHeader.m_filterEditors[0]->isHidden());
      QVERIFY(!horHeader.m_filterEditors[1]->isHidden());
      QVERIFY(!horHeader.m_filterEditors[2]->isHidden());
      QVERIFY(!horHeader.m_filterEditors[3]->isHidden());

      QCOMPARE(horHeader.sectionResizeMode(0), QHeaderView::ResizeMode::ResizeToContents);

      QVERIFY(horHeader.stretchLastSection());
      QVERIFY(!verHeader.stretchLastSection());

      QCOMPARE(horHeader.defaultSectionSize(), 66);
      QCOMPARE(horHeader.maximumSectionSize(), 888);

      // search signal ok
      horHeader.m_filterEditors[0]->setText("Chris Evans|Henry Cavill"); // Name
      horHeader.m_filterEditors[1]->setText(">1024"); // Size: 1024 B

      QSignalSpy searchStatementChangedSpy{&horHeader, &DoubleRowHeader::searchStatementChanged};
      emit horHeader.m_filterEditors[0]->returnPressed();
      QCOMPARE(searchStatementChangedSpy.count(), 1);
      QList<QVariant> actualParams = searchStatementChangedSpy.takeLast();
      QList<QVariant> expectParams {
        R"((`Name` LIKE "%Chris Evans%" OR `Name` LIKE "%Henry Cavill%") AND `Size`>1024)"
      };
      QCOMPARE(actualParams, expectParams);
    }
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
