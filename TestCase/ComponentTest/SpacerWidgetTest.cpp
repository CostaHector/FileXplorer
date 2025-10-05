#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "SpacerWidget.h"
#include "EndToExposePrivateMember.h"

class SpacerWidgetTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_spacer_widget_creation() {
    {
      QWidget* spacer = SpacerWidget::GetSpacerWidget(nullptr);
      QVERIFY(spacer != nullptr);
      QCOMPARE(spacer->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
      delete spacer;  // need delete manually
    }

    QWidget parent;
    {
      QWidget* spacer = SpacerWidget::GetSpacerWidget(&parent, Qt::Horizontal);
      QVERIFY(spacer != nullptr);
      QCOMPARE(spacer->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
      QCOMPARE(spacer->sizePolicy().verticalPolicy(), QSizePolicy::Preferred);
    }

    {
      QWidget* spacer = SpacerWidget::GetSpacerWidget(&parent, Qt::Vertical);
      QVERIFY(spacer != nullptr);
      QCOMPARE(spacer->sizePolicy().horizontalPolicy(), QSizePolicy::Preferred);
      QCOMPARE(spacer->sizePolicy().verticalPolicy(), QSizePolicy::Expanding);
    }
  }
};

#include "SpacerWidgetTest.moc"
REGISTER_TEST(SpacerWidgetTest, false)
