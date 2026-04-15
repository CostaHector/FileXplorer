#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "SceneStyleDelegate.h"
#include "EndToExposePrivateMember.h"

#include <QLocale>

class SceneStyleDelegateTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void default_ok() {  //
    SceneStyleDelegate sceneStyleDel;
    sceneStyleDel.initStyleOption(nullptr, {});

    QCOMPARE(sceneStyleDel.displayText("hello world", QLocale{QLocale::Language::English}), "hello world");
    sceneStyleDel.paint(nullptr, QStyleOptionViewItem{}, {});
    sceneStyleDel.drawRatingGrid(nullptr, QRect{}, 8, 10);
    sceneStyleDel.onSceneClicked({}, {}, {});
    SceneStyleDelegate::GetRatingAreaRect({});
  }
};

#include "SceneStyleDelegateTest.moc"
REGISTER_TEST(SceneStyleDelegateTest, false)
