#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "JsonTableView.h"
#include "CastManager.h"
#include "StudiosManager.h"
#include "EndToExposePrivateMember.h"
#include "JsonActions.h"
#include "TDir.h"
#include "UserInteractiveMock.h"
#include "RenameWidget_Replace.h"

#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QMessageBox>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

#include "JsonTestPrecoditionTools.h"
using namespace JsonTestPrecoditionTools;

class JsonTableViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
  const QString workPath{tDir.path()};
  CastManager& castInst = CastManager::getInst();
  StudiosManager& studioInst = StudiosManager::getInst();
 private slots:
  void initTestCase() {
    QVERIFY(tDir.IsValid());
    const QList<FsNodeEntry> nodes{
        //
        {"a.json", false, JSON_CONTENTS_A_RANK_IN_MODEL},               //
        {"b.json", false, JSON_CONTENTS_B_RANK_IN_MODEL},               //
        {"jazz .txt", false, "don't care me here"},                     //
        {"a.mp4", false, "only care me in rename json related files"},  //
        {"casts_list_test.txt", false, "only care me in export"},       //
        {"studios_list_test.txt", false, "only care me in export"},     //
    };
    QCOMPARE(tDir.createEntries(nodes), nodes.size());

    castInst.InitializeImpl(tDir.itemPath("casts_list_test.txt"));
    castInst.CastSet().clear();

    studioInst.InitializeImpl(tDir.itemPath("studios_list_test.txt"));
    studioInst.ProStudioMap().clear();
  }

  void init() {
    GlobalMockObject::reset();
    MOCKER(QInputDialog::getItem).stubs().will(invoke(UserInteractiveMock::InputDialog::invoke_getItem));
  }

  void cleanup() {  //
    GlobalMockObject::verify();
  }

  void empty_table_ok() {
    QSortFilterProxyModel jsonProxyModel;
    JsonTableModel jsonModel;
    JsonTableView jsonView{&jsonModel, &jsonProxyModel};

    auto& inst = g_JsonActions();
    // not crash down when no rows
    QCOMPARE(jsonModel.rowCount(), 0);

    QVERIFY(jsonView._JsonModel != nullptr);
    QVERIFY(jsonView._JsonProxyModel != nullptr);
    QVERIFY(jsonView.m_DetailEdit != nullptr);

    QVERIFY(!jsonView.CurrentIndexSource().isValid());
    QVERIFY(jsonView.selectedRowsSource(JSON_KEY_E::Name).isEmpty());

    emit inst._CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->triggered();
    emit inst._LOWER_ALL_WORDS->triggered();
    QCOMPARE(jsonView.onSelectionCaseOperation(true), -1);
    QCOMPARE(jsonView.onSelectionCaseOperation(false), -1);

    emit inst._CLEAR_STUDIO->triggered();
    emit inst._CLEAR_CAST->triggered();
    emit inst._CLEAR_TAGS->triggered();

    emit inst._CAST_FIELD_SET->triggered();
    emit inst._CAST_FIELD_APPEND->triggered();
    emit inst._CAST_FIELD_RMV->triggered();

    emit inst._TAGS_FIELD_SET->triggered();
    emit inst._TAGS_FIELD_APPEND->triggered();
    emit inst._TAGS_FIELD_RMV->triggered();

    QCOMPARE(jsonView.onSaveCurrentChanges(), 0);  // nothing selected
    QCOMPARE(jsonView.onSyncNameField(), 0);
    QCOMPARE(jsonView.onExportCastStudioToDictonary(), 0);
    QCOMPARE(jsonView.onRenameJsonAndRelated(), 0);
    QCOMPARE(jsonView.onSetStudio(), 0);
    QCOMPARE(jsonView.onInitCastAndStudio(), 0);
    QCOMPARE(jsonView.onHintCastAndStudio(), 0);
    QCOMPARE(jsonView.onFormatCast(), 0);
    QCOMPARE(jsonView.onClearStudio(), 0);

    QCOMPARE(jsonView.onAppendFromSelection(true), -1);
    QCOMPARE(jsonView.onAppendFromSelection(false), -1);

    QString selectedText;
    JsonTableView::EDITOR_WIDGET_TYPE widgetType{JsonTableView::EDITOR_WIDGET_TYPE::LINE_EDIT};
    QVERIFY(!jsonView.GetSelectedTextInCell(selectedText, widgetType));
  }

  void onSyncNameField_ok() {
    QSortFilterProxyModel jsonProxyModel;
    JsonTableModel jsonModel;
    JsonTableView jsonView{&jsonModel, &jsonProxyModel};

    // set a directory
    QCOMPARE(jsonView.ReadADirectory(workPath), 2);
    QCOMPARE(jsonModel.rowCount(), 2);
    {
      // onSyncNameField ok. `Name` fields value should be file base name after sync
      QCOMPARE(jsonModel.index(0, JSON_KEY_E::Name).data().toString(), "GameTurbo - A rank - GGG YYYYY");
      QCOMPARE(jsonModel.index(1, JSON_KEY_E::Name).data().toString(), "GameTurbo - B rank - XX YY and ZZ DD EE");
      // 1.1 select none, return 0
      jsonView.selectionModel()->clear();
      QCOMPARE(jsonView.onSyncNameField(), 0);
      // 1.2 select single row "a.json", return 1,
      jsonView.selectionModel()->clear();
      jsonView.selectRow(0);
      QCOMPARE(jsonView.onSyncNameField(), 1);
      QCOMPARE(jsonModel.index(0, JSON_KEY_E::Name).data().toString(), "a");
      // 1.3 select single row "b.json", return 1,
      jsonView.selectionModel()->clear();
      jsonView.selectRow(1);
      QCOMPARE(jsonView.onSyncNameField(), 1);
      QCOMPARE(jsonModel.index(1, JSON_KEY_E::Name).data().toString(), "b");
      // 1.4 no need sync anymore. its `Name` fields value already sync with file base name
      jsonView.selectAll();
      QCOMPARE(jsonView.onSyncNameField(), 0);
    }

    auto& inst = g_JsonActions();
    {
      // sync has no been write into local disk files. reload will loose the unsave changes
      // 1.1 reload ok
      jsonView.selectionModel()->clear();
      emit inst._RELOAD_JSON_FROM_FROM_DISK->triggered();
      QCOMPARE(jsonModel.index(0, JSON_KEY_E::Name).data().toString(), "GameTurbo - A rank - GGG YYYYY");
      QCOMPARE(jsonModel.index(1, JSON_KEY_E::Name).data().toString(), "GameTurbo - B rank - XX YY and ZZ DD EE");

      // 1.2 select both of them, sync ok
      jsonView.selectAll();
      QCOMPARE(jsonView.onSyncNameField(), 2);
      QCOMPARE(jsonModel.index(0, JSON_KEY_E::Name).data().toString(), "a");
      QCOMPARE(jsonModel.index(1, JSON_KEY_E::Name).data().toString(), "b");
    }
  }

  void init_hint_cast_studio_ok() {
    QSortFilterProxyModel jsonProxyModel;
    JsonTableModel jsonModel;
    JsonTableView jsonView{&jsonModel, &jsonProxyModel};

    QCOMPARE(jsonView.ReadADirectory(workPath), 2);
    QCOMPARE(jsonModel.rowCount(), 2);
    {
      // onInitCastAndStudio ok
      // 1.1 no selection skip
      jsonView.selectionModel()->clear();
      QCOMPARE(jsonView.onInitCastAndStudio(), 0);

      // 1.2 2 row selected
      jsonView.selectAll();
      QCOMPARE(jsonView.onInitCastAndStudio(), 2);  // don't expect its value
    }

    {  // onHintCastAndStudio ok
      jsonView.selectionModel()->clear();
      QCOMPARE(jsonView.onHintCastAndStudio(), 0);
      // not in selection mode, selection some rows first, no curentIndex
      jsonView.selectAll();
      QVERIFY(!jsonView.currentIndex().isValid());
      QCOMPARE(jsonView.onHintCastAndStudio(), -1);

      // we don't expect rows modified count
      jsonView.selectRow(0);
      jsonView.setCurrentIndex(jsonProxyModel.index(0, 0));
      QCOMPARE(jsonView.onHintCastAndStudio(), 1);
    }
  }

  void onAppendFromSelection_ok() {
    QSortFilterProxyModel jsonProxyModel;
    JsonTableModel jsonModel;
    JsonTableView jsonView{&jsonModel, &jsonProxyModel};

    QLineEdit lineEditCell{&jsonView};
    QPlainTextEdit plainTextEditCell{&jsonView};
    QTextEdit textEditCell{&jsonView};
    QWidget invalidEditCell{&jsonView};

    // set a directory
    QCOMPARE(jsonView.ReadADirectory(workPath), 2);
    QCOMPARE(jsonModel.rowCount(), 2);
    // normal/UPPERCASE STRING. precondition: `Name` Field is QLineEdit by default
    QCOMPARE(jsonView.pWidgetInCellMock, nullptr);
    jsonView.selectRow(0);
    jsonView.setCurrentIndex(jsonProxyModel.index(0, JsonKey::Name));  // nullptr
    jsonView.onAppendFromSelection(false);

    // only 2 unique one
    lineEditCell.setText("Chris Evans, Henry Cavill,Henry Cavill,Chris Evans");
    lineEditCell.selectAll();
    jsonView.pWidgetInCellMock = &lineEditCell;
    jsonModel.setData(jsonModel.index(0, JSON_KEY_E::Cast), "", Qt::EditRole);
    QCOMPARE(jsonView.onAppendFromSelection(false), 2);
    QCOMPARE(jsonModel.index(0, JSON_KEY_E::Cast).data().toString(), "Chris Evans,Henry Cavill");
    jsonModel.setData(jsonModel.index(0, JSON_KEY_E::Cast), "", Qt::EditRole);

    // only 2 unique UPPERCASE words ok
    lineEditCell.setText("MICHAEL FASSBENDER,Ricardo Kaka,MICHAEL FASSBENDER,ALVARO MORATA,Christiano Ronaldo,ALVARO MORATA");
    lineEditCell.selectAll();
    jsonView.pWidgetInCellMock = &lineEditCell;
    QCOMPARE(jsonView.onAppendFromSelection(true), 2);
    QCOMPARE(jsonModel.index(0, JSON_KEY_E::Cast).data().toString(), "Alvaro Morata,Michael Fassbender");
    jsonModel.setData(jsonModel.index(0, JSON_KEY_E::Cast), "", Qt::EditRole);

    jsonView.pWidgetInCellMock = &plainTextEditCell;
    plainTextEditCell.setPlainText("MICHAEL FASSBENDER,Ricardo Kaka,MICHAEL FASSBENDER,ALVARO MORATA,Christiano Ronaldo,ALVARO MORATA");
    plainTextEditCell.selectAll();
    QCOMPARE(jsonView.onAppendFromSelection(true), 2);
    jsonModel.setData(jsonModel.index(0, JSON_KEY_E::Cast), "", Qt::EditRole);

    jsonView.pWidgetInCellMock = &textEditCell;
    textEditCell.setPlainText("MICHAEL FASSBENDER,Ricardo Kaka,MICHAEL FASSBENDER,ALVARO MORATA,Christiano Ronaldo,ALVARO MORATA");
    textEditCell.selectAll();
    QCOMPARE(jsonView.onAppendFromSelection(true), 2);
    jsonModel.setData(jsonModel.index(0, JSON_KEY_E::Cast), "", Qt::EditRole);

    jsonView.pWidgetInCellMock = &invalidEditCell;  // cannot convert
    QCOMPARE(jsonView.onAppendFromSelection(true), -1);
  }

  void onSelectionCaseOperation_ok() {
    QSortFilterProxyModel jsonProxyModel;
    JsonTableModel jsonModel;
    JsonTableView jsonView{&jsonModel, &jsonProxyModel};

    QLineEdit lineEditCell{&jsonView};
    QPlainTextEdit plainTextEditCell{&jsonView};
    QTextEdit textEditCell{&jsonView};
    QWidget invalidEditCell{&jsonView};

    // set a directory
    QCOMPARE(jsonView.ReadADirectory(workPath), 2);
    QCOMPARE(jsonModel.rowCount(), 2);

    jsonModel.setData(jsonModel.index(0, JSON_KEY_E::Detail), "", Qt::EditRole);

    jsonView.setCurrentIndex({});  // set invalid
    QCOMPARE(jsonView.onSelectionCaseOperation(false), -1);
    jsonView.setCurrentIndex(jsonProxyModel.index(0, JsonKey::Detail));
    QVERIFY(jsonView.currentIndex().isValid());

    lineEditCell.setText("chris evans");
    lineEditCell.selectAll();
    jsonView.pWidgetInCellMock = &lineEditCell;
    jsonView.onSelectionCaseOperation(true);  // just title first letter
    QCOMPARE(lineEditCell.text(), "Chris Evans");
    QCOMPARE(lineEditCell.selectedText(), "Chris Evans");
    jsonView.onSelectionCaseOperation(false);  // lower all
    QCOMPARE(lineEditCell.text(), "chris evans");
    QCOMPARE(lineEditCell.selectedText(), "chris evans");

    plainTextEditCell.setPlainText("chris evans");
    plainTextEditCell.selectAll();
    jsonView.pWidgetInCellMock = &plainTextEditCell;
    jsonView.onSelectionCaseOperation(true);
    QCOMPARE(plainTextEditCell.toPlainText(), "Chris Evans");
    QCOMPARE(plainTextEditCell.textCursor().selectedText(), "Chris Evans");
    jsonView.onSelectionCaseOperation(false);
    QCOMPARE(plainTextEditCell.toPlainText(), "chris evans");
    QCOMPARE(plainTextEditCell.textCursor().selectedText(), "chris evans");

    textEditCell.setPlainText("chris evans");
    textEditCell.selectAll();
    jsonView.pWidgetInCellMock = &textEditCell;
    jsonView.onSelectionCaseOperation(true);
    QCOMPARE(textEditCell.toPlainText(), "Chris Evans");
    QCOMPARE(textEditCell.textCursor().selectedText(), "Chris Evans");  // 此处如何检查QPlainTextEdit选中
    jsonView.onSelectionCaseOperation(false);
    QCOMPARE(textEditCell.toPlainText(), "chris evans");
    QCOMPARE(textEditCell.textCursor().selectedText(), "chris evans");

    jsonView.pWidgetInCellMock = &invalidEditCell;  // cannot convert
    QCOMPARE(jsonView.onSelectionCaseOperation(true), -1);
    jsonView.pWidgetInCellMock = nullptr;
  }

  void onFormatCast_ok() {
    QSortFilterProxyModel jsonProxyModel;
    JsonTableModel jsonModel;
    JsonTableView jsonView{&jsonModel, &jsonProxyModel};
    QCOMPARE(jsonView.ReadADirectory(workPath), 2);
    QCOMPARE(jsonModel.rowCount(), 2);

    jsonView.selectionModel()->clear();
    QCOMPARE(jsonView.onFormatCast(), 0);

    jsonModel.setData(jsonModel.index(0, JSON_KEY_E::Cast), "Henry Cavill, Chris Evans, Henry Cavill", Qt::EditRole);
    jsonView.selectRow(0);
    QCOMPARE(jsonView.onFormatCast(), 1);
    QCOMPARE(jsonModel.index(0, JSON_KEY_E::Cast).data().toString(), "Chris Evans,Henry Cavill");
    jsonView.close();
  }

  void setStudioCastTags_ok() {
    QSortFilterProxyModel jsonProxyModel;
    JsonTableModel jsonModel;
    JsonTableView jsonView{&jsonModel, &jsonProxyModel};
    QCOMPARE(jsonView.ReadADirectory(workPath), 2);
    QCOMPARE(jsonModel.rowCount(), 2);

    {
      // onSetStudio ok, onClearStudio ok
      jsonView.selectionModel()->clear();
      QCOMPARE(jsonView.onSetStudio(), 0);  // nothing selected
      QCOMPARE(jsonView.onClearStudio(), 0);

      jsonView.selectAll();
      jsonView.onClearStudio();
      QCOMPARE(jsonModel.index(0, JSON_KEY_E::Studio).data().toString(), "");
      QCOMPARE(jsonModel.index(1, JSON_KEY_E::Studio).data().toString(), "");

      UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(false, "Michael Fassbender");
      QCOMPARE(jsonView.onSetStudio(), 0);

      UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "");  // empty reject
      QCOMPARE(jsonView.onSetStudio(), 0);

      UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "Michael Fassbender");
      QCOMPARE(jsonView.onSetStudio(), 2);
      QCOMPARE(jsonModel.index(0, JSON_KEY_E::Studio).data().toString(), "Michael Fassbender");
      QCOMPARE(jsonModel.index(1, JSON_KEY_E::Studio).data().toString(), "Michael Fassbender");

      QCOMPARE(jsonView.onClearStudio(), 2);
      QCOMPARE(jsonModel.index(0, JSON_KEY_E::Studio).data().toString(), "");
      QCOMPARE(jsonModel.index(1, JSON_KEY_E::Studio).data().toString(), "");
    }

    {
      // set/add/rmv casts/tags ok
      jsonView.selectionModel()->clear();
      QCOMPARE(jsonView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::APPEND), 0);

      jsonView.selectAll();
      MOCKER((UserInteractiveMock::QUESTION_TYPE)QMessageBox::question)
          .stubs()
          .will(returnValue(QMessageBox::StandardButton::No))
          .then(returnValue(QMessageBox::StandardButton::No))
          .then(returnValue(QMessageBox::StandardButton::Yes))
          .then(returnValue(QMessageBox::StandardButton::Yes));
      QCOMPARE(jsonView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::CLEAR), 0);
      QCOMPARE(jsonView.onSetCastOrTags(FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::CLEAR), 0);
      QVERIFY(jsonView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::CLEAR) >= 0);
      QVERIFY(jsonView.onSetCastOrTags(FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::CLEAR) >= 0);
      QCOMPARE(jsonModel.index(0, JSON_KEY_E::Cast).data().toString(), "");
      QCOMPARE(jsonModel.index(1, JSON_KEY_E::Cast).data().toString(), "");
      QCOMPARE(jsonModel.index(0, JSON_KEY_E::Tags).data().toString(), "");
      QCOMPARE(jsonModel.index(1, JSON_KEY_E::Tags).data().toString(), "");

      UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(false, "Chris Evans");  // user ignored
      QCOMPARE(jsonView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::APPEND), 0);
      QCOMPARE(jsonView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::REMOVE), 0);
      QCOMPARE(jsonView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::SET), 0);
      UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "Chris Evans,Henry Cavill");
      QCOMPARE(jsonView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::APPEND), 2);
      QCOMPARE(jsonModel.index(0, JSON_KEY_E::Cast).data().toString(), "Chris Evans,Henry Cavill");
      QCOMPARE(jsonModel.index(1, JSON_KEY_E::Cast).data().toString(), "Chris Evans,Henry Cavill");
      UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "Chris Evans");
      QCOMPARE(jsonView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::REMOVE), 2);
      QCOMPARE(jsonModel.index(0, JSON_KEY_E::Cast).data().toString(), "Henry Cavill");
      QCOMPARE(jsonModel.index(1, JSON_KEY_E::Cast).data().toString(), "Henry Cavill");
      UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "Michael Fassbender");
      QCOMPARE(jsonView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::SET), 2);
      QCOMPARE(jsonModel.index(0, JSON_KEY_E::Cast).data().toString(), "Michael Fassbender");
      QCOMPARE(jsonModel.index(1, JSON_KEY_E::Cast).data().toString(), "Michael Fassbender");
      UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "");  // empty reject
      QCOMPARE(jsonView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::SET), 0);
      QCOMPARE(jsonModel.index(0, JSON_KEY_E::Cast).data().toString(), "Michael Fassbender");
      QCOMPARE(jsonModel.index(1, JSON_KEY_E::Cast).data().toString(), "Michael Fassbender");

      UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "SuperHero");
      QCOMPARE(jsonView.onSetCastOrTags(FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::SET), 2);
      QCOMPARE(jsonModel.index(0, JSON_KEY_E::Tags).data().toString(), "SuperHero");
      QCOMPARE(jsonModel.index(1, JSON_KEY_E::Tags).data().toString(), "SuperHero");
    }

    {
      UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "Marvel");
      jsonView.selectAll();
      QCOMPARE(jsonView.onSetStudio(), 2);
      QCOMPARE(jsonModel.index(0, JSON_KEY_E::Studio).data().toString(), "Marvel");
      QCOMPARE(jsonModel.index(1, JSON_KEY_E::Studio).data().toString(), "Marvel");

      jsonView.selectRow(0);
      UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "Chris Evans,Michael Fassbender");
      QCOMPARE(jsonView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::SET), 1);
      QCOMPARE(jsonModel.index(0, JSON_KEY_E::Cast).data().toString(), "Chris Evans,Michael Fassbender");

      jsonView.selectRow(1);
      UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "Henry Cavill");
      QCOMPARE(jsonView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::SET), 1);
      QCOMPARE(jsonModel.index(1, JSON_KEY_E::Cast).data().toString(), "Henry Cavill");
    }
  }

  void onSaveCurrentChanges_ok() {
    QVERIFY(tDir.exists("a.json"));
    QVERIFY(tDir.exists("b.json"));

    QSortFilterProxyModel jsonProxyModel;
    JsonTableModel jsonModel;
    JsonTableView jsonView{&jsonModel, &jsonProxyModel};
    QCOMPARE(jsonView.ReadADirectory(workPath), 2);
    QCOMPARE(jsonModel.rowCount(), 2);

    QCOMPARE(jsonModel.fileName(jsonModel.index(0, JSON_KEY_E::Name)), "a.json");
    QCOMPARE(jsonModel.fileName(jsonModel.index(1, JSON_KEY_E::Name)), "b.json");
    QCOMPARE(jsonModel.index(0, JSON_KEY_E::Name).data().toString(), "GameTurbo - A rank - GGG YYYYY");
    QCOMPARE(jsonModel.index(1, JSON_KEY_E::Name).data().toString(), "GameTurbo - B rank - XX YY and ZZ DD EE");

    jsonProxyModel.sort(JSON_KEY_E::Name, Qt::DescendingOrder);
    QCOMPARE(jsonProxyModel.mapToSource(jsonProxyModel.index(0, JSON_KEY_E::Name)).data(Qt::DisplayRole).toString(),
             "GameTurbo - B rank - XX YY and ZZ DD EE");
    QCOMPARE(jsonProxyModel.mapToSource(jsonProxyModel.index(1, JSON_KEY_E::Name)).data(Qt::DisplayRole).toString(),
             "GameTurbo - A rank - GGG YYYYY");

    jsonProxyModel.sort(JSON_KEY_E::Name, Qt::AscendingOrder);
    QCOMPARE(jsonProxyModel.mapToSource(jsonProxyModel.index(0, JSON_KEY_E::Name)).data(Qt::DisplayRole).toString(),
             "GameTurbo - A rank - GGG YYYYY");
    QCOMPARE(jsonProxyModel.mapToSource(jsonProxyModel.index(1, JSON_KEY_E::Name)).data(Qt::DisplayRole).toString(),
             "GameTurbo - B rank - XX YY and ZZ DD EE");

    jsonModel.setData(jsonModel.index(0, JSON_KEY_E::Studio), "Marvel", Qt::EditRole);
    jsonModel.setData(jsonModel.index(1, JSON_KEY_E::Studio), "", Qt::EditRole);

    jsonModel.setData(jsonModel.index(0, JSON_KEY_E::Cast), "Chris Evans, Michael Fassbender", Qt::EditRole);
    jsonModel.setData(jsonModel.index(1, JSON_KEY_E::Cast), "Henry Cavill", Qt::EditRole);

    // no row selected
    jsonView.selectionModel()->clear();
    QCOMPARE(jsonView.onSaveCurrentChanges(), 0);

    // 2 row selected
    jsonView.selectAll();
    QCOMPARE(jsonView.onSaveCurrentChanges(), 2);

    QVERIFY(tDir.checkFileContents("a.json", {"Chris Evans", "Michael Fassbender"}));
    QVERIFY(tDir.checkFileContents("b.json", {"Henry Cavill"}));
  }

  void onExportCastStudioToDictonary_ok() {
    QSortFilterProxyModel jsonProxyModel;
    JsonTableModel jsonModel;
    JsonTableView jsonView{&jsonModel, &jsonProxyModel};
    QCOMPARE(jsonView.ReadADirectory(workPath), 2);
    QCOMPARE(jsonModel.rowCount(), 2);

    jsonModel.setData(jsonModel.index(0, JSON_KEY_E::Studio), "Marvel", Qt::EditRole);
    jsonModel.setData(jsonModel.index(1, JSON_KEY_E::Studio), "", Qt::EditRole);

    jsonModel.setData(jsonModel.index(0, JSON_KEY_E::Cast), "Henry Cavill, Chris Evans, Henry Cavill", Qt::EditRole);
    jsonModel.setData(jsonModel.index(1, JSON_KEY_E::Cast), "Michael Fassbender", Qt::EditRole);

    castInst.CastSet().clear();
    studioInst.ProStudioMap().clear();

    // no row selected
    jsonView.selectionModel()->clear();
    QCOMPARE(jsonView.onExportCastStudioToDictonary(), 0);
    QVERIFY(castInst.CastSet().isEmpty());
    QVERIFY(studioInst.ProStudioMap().isEmpty());

    // 2 row selected
    jsonView.selectAll();
    QVERIFY(jsonView.onExportCastStudioToDictonary() >= 0);

    CAST_MGR_DATA_T expectCast{"chris evans", "henry cavill", "michael fassbender"};
    QCOMPARE(castInst.CastSet(), expectCast);

    STUDIO_MGR_DATA_T expectStudios{{"marvel", "Marvel"}};
    QCOMPARE(studioInst.ProStudioMap(), expectStudios);
  }

  void onRenameJsonAndRelated_ok() {
    QVERIFY(tDir.exists("a.json"));
    QVERIFY(tDir.exists("a.mp4"));

    QSortFilterProxyModel jsonProxyModel;
    JsonTableModel jsonModel;
    JsonTableView jsonView{&jsonModel, &jsonProxyModel};
    QCOMPARE(jsonView.ReadADirectory(workPath), 2);
    QCOMPARE(jsonModel.rowCount(), 2);

    // rename "a.json" to "Super Hero - Captain America.json".
    // will also rename images/videos/and other files
    // whose base name match pattern "a", or "a \d{1,3}" or "a | \d{1,3}"
    jsonView.selectionModel()->clear();
    QCOMPARE(jsonView.onRenameJsonAndRelated(), 0);

    QString expectWorkPath{workPath};
    QStringList expectSelectedNames{"a.json", "a.mp4"};
    QString expectDefOldName{"a"};
    QString expectDefNewName{"a"};
    bool expectDisableOldNameEdit{true};

    std::pair<bool, QString> userRejectPr(false, "Super Hero - Captain America");
    std::pair<bool, QString> userAcceptPr(true, "Super Hero - Captain America");
    MOCKER(RenameWidget_Replace::QueryAndConfirm)
        .expects(exactly(2))  //
        .with(eq(expectWorkPath), eq(expectSelectedNames), eq(expectDefOldName), eq(expectDefNewName), eq(expectDisableOldNameEdit))
        .will(returnValue(userRejectPr))  //
        .then(returnValue(userAcceptPr));
    jsonView.selectRow(0);
    QCOMPARE(jsonView.onRenameJsonAndRelated(), 0);
    QCOMPARE(jsonView.onRenameJsonAndRelated(), 2);  // two file get renamed
  }
};

#include "JsonTableViewTest.moc"
REGISTER_TEST(JsonTableViewTest, false)
