#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ToolBarWidget.h"
#include "EndToExposePrivateMember.h"
#include <QLineEdit>

class ToolBarWidgetTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:

  void default_ok() {
    std::unique_ptr<ToolBarWidget> tb{new ToolBarWidget{QBoxLayout::Direction::LeftToRight}};
    QVERIFY(tb->mLayout != nullptr);
    QCOMPARE(tb->mLayout->direction(), QBoxLayout::Direction::LeftToRight);

    ToolBarWidget* subTb = new ToolBarWidget{QBoxLayout::Direction::LeftToRight, tb.get()};
    QAction openAct{"open", subTb};
    subTb->addAction(&openAct);

    QLineEdit* nameEdit = new QLineEdit{"Kaka", tb.get()};
    QAction saveAction{"save", tb.get()};
    QAction quitAction{"quit", tb.get()};

    tb->addWidget(subTb);
    tb->addSpacing();
    tb->addString("Name:");
    tb->addWidget(nameEdit);
    tb->addSeparator();
    tb->addSpacing();
    tb->addSeparator();
    tb->addAction(&saveAction);
    tb->addStretch();
    tb->addAction(&quitAction);

    QCOMPARE(tb->setDirection(QBoxLayout::Direction::TopToBottom), true);
    QCOMPARE(tb->setDirection(QBoxLayout::Direction::TopToBottom), false);
    QCOMPARE(tb->mLayout->direction(), QBoxLayout::Direction::TopToBottom);

    // 纵向加入分隔
    tb->addSeparator();

    // 已固定的不受setToolButtonStyle影响
    QAction helpAction{"help", tb.get()};
    QToolButton* fixedStyleBtn = tb->createToolButton(&helpAction, Qt::ToolButtonStyle::ToolButtonIconOnly, true);
    QCOMPARE(fixedStyleBtn->toolButtonStyle(), Qt::ToolButtonStyle::ToolButtonIconOnly);
    tb->addWidget(fixedStyleBtn);

    // 非递归修改, saveAction, quitAction
    tb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon, true);  // 重置
    QCOMPARE(tb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon, false), 2);
    QCOMPARE(fixedStyleBtn->toolButtonStyle(), Qt::ToolButtonStyle::ToolButtonIconOnly);

    // 递归修改, saveAction, quitAction
    tb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon, true);  // 重置
    QCOMPARE(tb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon, true), 3);
    QCOMPARE(fixedStyleBtn->toolButtonStyle(), Qt::ToolButtonStyle::ToolButtonIconOnly);
  }
};

#include "ToolBarWidgetTest.moc"
REGISTER_TEST(ToolBarWidgetTest, false)
