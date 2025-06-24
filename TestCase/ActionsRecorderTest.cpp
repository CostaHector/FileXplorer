#include <QCoreApplication>
#include <QtTest>
#include <QToolButton>
#include "pub/MyTestSuite.h"
#include "pub/BeginToExposePrivateMember.h"
#include "Actions/ActionsRecorder.h"
#include "pub/EndToExposePrivateMember.h"

class ActionsRecorderTest : public MyTestSuite {
  Q_OBJECT
 public:
  ActionsRecorderTest() : MyTestSuite{false} {}
 private slots:
  void test_record_actions_from_nullptr_ok() {
    ActionsRecorder actRecorder;
    QCOMPARE(actRecorder.isDirty, true);
    QCOMPARE(actRecorder.GetKeys(), (QStringList{}));
    QCOMPARE(actRecorder.isDirty, false);
    QCOMPARE(actRecorder.FromToolbar(nullptr), 0);
    QCOMPARE(actRecorder.FromMenu(nullptr), 0);
    QCOMPARE(actRecorder.FromActionGroup(nullptr), 0);
    QCOMPARE(actRecorder.isDirty, false);
  }

  void test_record_actions_from_empty_toolbar_ok() {
    QToolBar rootToolbar;
    ActionsRecorder actRecorder;
    QCOMPARE(actRecorder.isDirty, true);
    QCOMPARE(actRecorder.GetKeys(), (QStringList{}));
    QCOMPARE(actRecorder.isDirty, false);
    QCOMPARE(actRecorder.FromToolbar(&rootToolbar), 0);
    QCOMPARE(actRecorder.isDirty, false);
  }

  void test_record_actions_from_flatten_toolbar_ok() {
    QAction file{"file"};
    QAction help{"help"};
    QToolBar rootToolbar{"root"};
    rootToolbar.addAction(&file);
    rootToolbar.addSeparator();
    rootToolbar.addAction(&help);

    const QMap<QString, QAction*> expectKeys2Action{
        {"file/root", &file},
        {"help/root", &help},
    };
    ActionsRecorder actRecorder;
    QCOMPARE(actRecorder.isDirty, true);
    QCOMPARE(actRecorder.GetKeys(), (QStringList{}));
    QCOMPARE(actRecorder.isDirty, false);
    QCOMPARE(actRecorder.FromToolbar(&rootToolbar), expectKeys2Action.size());
    QCOMPARE(actRecorder.isDirty, true);
    QCOMPARE(actRecorder.GetKeys(), expectKeys2Action.keys());
    QCOMPARE(actRecorder.isDirty, false);
  }

  void test_no_text_action_ok() {
    QAction noTextAction{};
    QToolBar rootToolbar{"root"};
    rootToolbar.addAction(&noTextAction);
    ActionsRecorder actRecorder;
    QCOMPARE(actRecorder.FromToolbar(&rootToolbar), 1);
    QCOMPARE(actRecorder.GetKeys(), QStringList{"/root"});
  }

  void test_record_actions_from_nested_toolbar_ok() {
    QAction open{"open"}, close{"close"};
    QAction edit{"edit"};
    QAction about{"about"};
    QToolBar rootToolbar{"root"}, fileToolBar{"file"}, helpToolBar{"help"};

    fileToolBar.addAction(&open);
    fileToolBar.addSeparator();
    fileToolBar.addAction(&close);

    helpToolBar.addAction(&about);

    rootToolbar.addWidget(&fileToolBar);
    rootToolbar.addSeparator();
    rootToolbar.addAction(&edit);
    rootToolbar.addSeparator();
    rootToolbar.addWidget(&helpToolBar);

    const QMap<QString, QAction*> expectKeys2Action{
        {"open/file/root", &open},    //
        {"close/file/root", &close},  //
        {"edit/root", &edit},         //
        {"about/help/root", &about},
    };

    ActionsRecorder actRecorder;
    QCOMPARE(actRecorder.FromToolbar(&rootToolbar), expectKeys2Action.size());

    QStringList expectsLst{expectKeys2Action.keys()};
    expectsLst.sort();
    QStringList actualList = actRecorder.GetKeys();
    actualList.sort();
    QCOMPARE(expectsLst, actualList);
  }

  void test_widget_skip() {
    QWidget* placeHolder = new QWidget{};
    placeHolder->setWindowTitle("placeholder");

    QToolBar* rootToolbar = new QToolBar{"root"};
    QAction* pPlaceHolderWidget = rootToolbar->addWidget(placeHolder);
    QVERIFY(pPlaceHolderWidget != nullptr);
    QAction* hello = new QAction{"hello"};
    rootToolbar->addAction(hello);

    const QMap<QString, QAction*> expectKeys2Action{
        {"hello/root", hello},  // not contains pPlaceHolderWidget
    };

    ActionsRecorder actRecorder;
    QCOMPARE(actRecorder.FromToolbar(rootToolbar), expectKeys2Action.size());
    QCOMPARE(actRecorder.mTextToActionMap, expectKeys2Action);
    QCOMPARE(actRecorder.GetKeys(), expectKeys2Action.keys());
  }

  void test_toolbar_with_toolbutton_with_default_action_ok() {
    QAction* defaultAction{new QAction{"Query Before"}};

    QToolButton* toolButton{new QToolButton{nullptr}};
    toolButton->setText("Toolbutton itself text(any plain Button)");
    toolButton->setDefaultAction(defaultAction);

    QToolBar* rootToolbar{new QToolBar{"root"}};
    rootToolbar->addWidget(toolButton);  // should not in answer

    const QMap<QString, QAction*> expectKeys2Action{
        {"Query Before/root", defaultAction},
    };

    ActionsRecorder actRecorder;
    QCOMPARE(actRecorder.FromToolbar(rootToolbar), expectKeys2Action.size());
    QCOMPARE(actRecorder.GetKeys(), expectKeys2Action.keys());
  }

  void test_toolbutton_with_default_action_and_qmenu_ok() {
    QAction* defaultAction{new QAction{"Query Before"}};
    QAction* deleteDirectly{new QAction{"Delete Directly"}};
    QAction* recycleThem{new QAction{"Recycle Them"}};

    QToolButton* toolButton{new QToolButton{nullptr}};
    toolButton->setText("Toolbutton itself text(any plain Button)");
    toolButton->setDefaultAction(defaultAction);

    QMenu* menu{new QMenu{"Operation Preference", toolButton}};
    menu->addAction(defaultAction);
    menu->addSeparator();
    menu->addAction(deleteDirectly);
    menu->addAction(recycleThem);
    toolButton->setMenu(menu);

    const QMap<QString, QAction*> expectKeys2Action{
        {"Query Before/OP", defaultAction},
        {"Query Before/Operation Preference/OP", defaultAction},
        {"Delete Directly/Operation Preference/OP", deleteDirectly},
        {"Recycle Them/Operation Preference/OP", recycleThem},
    };

    ActionsRecorder actRecorder;
    QCOMPARE(actRecorder.FromToolButton(toolButton, "OP"), expectKeys2Action.size());
    QCOMPARE(actRecorder.GetKeys(), expectKeys2Action.keys());
  }

  void test_menu_nested_ok() {
    QMenu open{"open"};
    QAction pdf{"pdf"}, docx{"docx"};
    open.addAction(&pdf);
    open.addSeparator();
    open.addAction(&docx);

    QAction close{"close"};
    QMenu rootMenu{"root"};
    rootMenu.addMenu(&open);
    rootMenu.addAction(&close);

    ActionsRecorder actRecorder;
    QCOMPARE(actRecorder.FromMenu(&rootMenu), 3);
    QStringList expectsLst{
        "pdf/open/root",   //
        "docx/open/root",  //
        "close/root",      //
    };
    expectsLst.sort();
    QStringList actualList = actRecorder.GetKeys();
    actualList.sort();
    QCOMPARE(expectsLst, actualList);
  }

  void test_menubar_nested_ok() {
    QMenu open{"open"};
    QAction pdf{"pdf"}, docx{"docx"};
    open.addAction(&pdf);
    open.addSeparator();
    open.addAction(&docx);

    QAction close{"close"};
    QMenu rootMenu1{"root1"};
    rootMenu1.addMenu(&open);
    rootMenu1.addAction(&close);

    QAction about{"about"};
    QMenu rootMenu2{"root2"};
    rootMenu2.addAction(&about);

    QMenuBar menubar;
    menubar.addMenu(&rootMenu1);
    menubar.addMenu(&rootMenu2);

    ActionsRecorder actRecorder;
    QCOMPARE(actRecorder.FromMenuBar(&menubar), 3 + 1);
    QStringList expectsLst{
        "pdf/open/root1",   //
        "docx/open/root1",  //
        "close/root1",      //
        "about/root2",      //
    };
    expectsLst.sort();
    QStringList actualList = actRecorder.GetKeys();
    actualList.sort();
    QCOMPARE(expectsLst, actualList);
  }

  void test_from_actionGrp_ok() {
    QAction* flatten = new QAction{"flatten"};
    QAction* remain = new QAction{"remain"};
    QActionGroup* actGrp = new QActionGroup{this};
    actGrp->addAction(flatten);
    actGrp->addAction(remain);
    ActionsRecorder actRecorder;
    QCOMPARE(actRecorder.FromActionGroup(actGrp), 2);
    QStringList expectsLst{
        "flatten",  //
        "remain",   //
    };
    expectsLst.sort();
    QStringList actualList = actRecorder.GetKeys();
    actualList.sort();
    QCOMPARE(expectsLst, actualList);
  }
};

ActionsRecorderTest g_ActionsRecorderTest;
#include "ActionsRecorderTest.moc"
