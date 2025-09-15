#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "WidgetReorderHelper.h"
#include <QLabel>

QStringList GetLabelTextsFromBoxLayout(const QBoxLayout& layout) {
  QStringList result;
  result.reserve(layout.count());
  for (int i = 0; i < layout.count(); ++i) {
    const QWidget* widget = layout.itemAt(i)->widget();
    if (const QLabel* label = qobject_cast<const QLabel*>(widget)) {
      result.append(label->text());
    }
  }
  return result;
}
QStringList GetLabelTextsFromSplitter(const QSplitter& splitter) {
  QStringList result;
  result.reserve(splitter.count());
  for (int i = 0; i < splitter.count(); ++i) {
    if (const QLabel* label = qobject_cast<const QLabel*>(splitter.widget(i))) {
      result.append(label->text());
    }
  }
  return result;
}
QStringList GetLabelTextsFromToolBar(const QToolBar& toolbar) {
  QStringList result;
  foreach (QAction* action, toolbar.actions()) {
    if (const QLabel* label = qobject_cast<const QLabel*>(toolbar.widgetForAction(action))) {
      result.append(label->text());
    }
  }
  return result;
}

class WidgetReorderHelperTest : public PlainTestSuite {
  Q_OBJECT
public:
  WidgetReorderHelperTest() : PlainTestSuite{} {}
private slots:
  void test_move_element_index_out_of_bound_skip() {
    QVector<int> vEmpty;
    const QVector<int> expectEmptyVec{vEmpty};          // should remains
    QCOMPARE(MoveElementFrontOf(vEmpty, 4, 6), false);  // out of bound
    QCOMPARE(vEmpty, expectEmptyVec);
    QCOMPARE(MoveElementFrontOf(vEmpty, 0, 0), false);  // out of bound
    QCOMPARE(vEmpty, expectEmptyVec);

    QVector<int> vec3Elements{1, 2, 3};
    const QVector<int> expectVec3Ans{vec3Elements};
    QCOMPARE(MoveElementFrontOf(vec3Elements, -1, 0), false);  // out of bound
    QCOMPARE(vec3Elements, expectVec3Ans);
    QCOMPARE(MoveElementFrontOf(vec3Elements, 0, 4), false);  // out of bound
    QCOMPARE(vec3Elements, expectVec3Ans);
    QCOMPARE(MoveElementFrontOf(vec3Elements, -1, 4), false);  // out of bound
    QCOMPARE(vec3Elements, expectVec3Ans);
  }

  void test_move_element_no_need_move_skip() {
    QVector<int> vec3Elements{1, 2, 3};
    const QVector<int> expectVec3Ans{vec3Elements};
    QCOMPARE(MoveElementFrontOf(vec3Elements, 0, 0), true);
    QCOMPARE(vec3Elements, expectVec3Ans);
    QCOMPARE(MoveElementFrontOf(vec3Elements, 2, 2), true);
    QCOMPARE(vec3Elements, expectVec3Ans);
    QCOMPARE(MoveElementFrontOf(vec3Elements, 0, 1), true);
    QCOMPARE(vec3Elements, expectVec3Ans);
    QCOMPARE(MoveElementFrontOf(vec3Elements, 1, 2), true);
    QCOMPARE(vec3Elements, expectVec3Ans);
  }

  void test_move_element_front() {
    QVector<int> v1Front0{0, 1, 2};
    MoveElementFrontOf(v1Front0, 1, 0);
    QCOMPARE(v1Front0, (QVector<int>{1, 0, 2}));

    QVector<int> v2Front1{0, 1, 2};
    MoveElementFrontOf(v2Front1, 2, 1);
    QCOMPARE(v2Front1, (QVector<int>{0, 2, 1}));

    QVector<int> v2Front0{0, 1, 2};
    MoveElementFrontOf(v2Front0, 2, 0);
    QCOMPARE(v2Front0, (QVector<int>{2, 0, 1}));
  }

  void test_move_element_back() {
    QVector<int> v0Front2{0, 1, 2};
    MoveElementFrontOf(v0Front2, 0, 2);
    QCOMPARE(v0Front2, (QVector<int>{1, 0, 2}));

    QVector<int> v0Front3{0, 1, 2};
    MoveElementFrontOf(v0Front3, 0, 3);
    QCOMPARE(v0Front3, (QVector<int>{1, 2, 0}));

    QVector<int> v1Front3{0, 1, 2};
    MoveElementFrontOf(v1Front3, 1, 3);
    QCOMPARE(v1Front3, (QVector<int>{0, 2, 1}));
  }

  void test_str3210_2_vec_ok() {
    QVector<int> v;
    QVERIFY(IsValidMediaTypeSeq("3210", v));
    QCOMPARE(v, (QVector<int>{3, 2, 1, 0}));
  }

  void test_str012_2_vec_ok() {
    QVector<int> v;
    QVERIFY(IsValidMediaTypeSeq("012", v));
    QCOMPARE(v, (QVector<int>{0, 1, 2}));
  }

  void test_str10_2_vec_ok() {
    QVector<int> v;
    QVERIFY(IsValidMediaTypeSeq("10", v));
    QCOMPARE(v, (QVector<int>{1, 0}));
  }

  void test_str013_2_vec_nok() {
    QVector<int> v;
    QVERIFY(!IsValidMediaTypeSeq("013", v));
  }

  void test_vector_int_tostr_ok() {  //
    QCOMPARE(MediaTypeSeqStr(QVector<int>{0, 4, 1, 3, 2}), "04132");
    QCOMPARE(MediaTypeSeqStr(QVector<int>{0, 1, 3, 2}), "0132");
    QCOMPARE(MediaTypeSeqStr(QVector<int>{1, 0}), "10");
    QCOMPARE(MediaTypeSeqStr(QVector<int>{0}), "0");
  }

  void test_reorder_boxlayout_ok() {
    QHBoxLayout hlayout;
    QVBoxLayout vlayout;
    constexpr int CNT = 2;
    QBoxLayout* layoutLst[CNT]{&hlayout, &vlayout};
    for (int i = 0; i < CNT; ++i) {
      QBoxLayout& layout = *layoutLst[i];
      QLabel lable0{"0"}, lable1{"1"}, lable3{"2"};
      layout.addWidget(&lable0);
      layout.addWidget(&lable1);
      layout.addWidget(&lable3);
      QCOMPARE(GetLabelTextsFromBoxLayout(layout), (QStringList{"0", "1", "2"}));

      QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(0, 0, layout), false);  // no need reorder
      QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(0, 1, layout), false);  // no need reorder
      QCOMPARE(GetLabelTextsFromBoxLayout(layout), (QStringList{"0", "1", "2"}));

      QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(0, 3, layout), true);  // move past the end (append)
      QCOMPARE(GetLabelTextsFromBoxLayout(layout), (QStringList{"1", "2", "0"}));

      QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(0, 2, layout), true);  // move not past the end
      QCOMPARE(GetLabelTextsFromBoxLayout(layout), (QStringList{"2", "1", "0"}));

      QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(2, 1, layout), true);  // move front
      QCOMPARE(GetLabelTextsFromBoxLayout(layout), (QStringList{"2", "0", "1"}));

      QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(2, 0, layout), true);  // move to the first (push front)
      QCOMPARE(GetLabelTextsFromBoxLayout(layout), (QStringList{"1", "2", "0"}));

      // extra test: out of bound
      QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(-1, 0, layout), false);   // fromIndex
      QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(3, 0, layout), false);    // fromIndex
      QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(0, -1, layout), false);   // destIndex
      QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(0, 4, layout), false);    // destIndex
    }
  }

  void test_reorder_splitter_ok() {
    QSplitter spiltter;
    QLabel lable0{"0"}, lable1{"1"}, lable3{"2"};
    spiltter.addWidget(&lable0);
    spiltter.addWidget(&lable1);
    spiltter.addWidget(&lable3);
    QCOMPARE(GetLabelTextsFromSplitter(spiltter), (QStringList{"0", "1", "2"}));

    QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(0, 0, spiltter), false);  // no need reorder
    QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(0, 1, spiltter), false);  // no need reorder
    QCOMPARE(GetLabelTextsFromSplitter(spiltter), (QStringList{"0", "1", "2"}));

    QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(0, 3, spiltter), true);  // move past the end (append)
    QCOMPARE(GetLabelTextsFromSplitter(spiltter), (QStringList{"1", "2", "0"}));

    QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(0, 2, spiltter), true);  // move not past the end
    QCOMPARE(GetLabelTextsFromSplitter(spiltter), (QStringList{"2", "1", "0"}));

    QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(2, 1, spiltter), true);  // move front
    QCOMPARE(GetLabelTextsFromSplitter(spiltter), (QStringList{"2", "0", "1"}));

    QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(2, 0, spiltter), true);  // move to the first (push front)
    QCOMPARE(GetLabelTextsFromSplitter(spiltter), (QStringList{"1", "2", "0"}));
  }

  void test_reorder_toolbar_ok() {
    QToolBar toolbar;
    QLabel lable0{"0"}, lable1{"1"}, lable3{"2"};
    toolbar.addWidget(&lable0);
    toolbar.addWidget(&lable1);
    toolbar.addWidget(&lable3);
    QCOMPARE(GetLabelTextsFromToolBar(toolbar), (QStringList{"0", "1", "2"}));

    QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(0, 0, toolbar), false);  // no need reorder
    QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(0, 1, toolbar), false);  // no need reorder
    QCOMPARE(GetLabelTextsFromToolBar(toolbar), (QStringList{"0", "1", "2"}));

    QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(0, 3, toolbar), true);  // move past the end (append)
    QCOMPARE(GetLabelTextsFromToolBar(toolbar), (QStringList{"1", "2", "0"}));

    QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(0, 2, toolbar), true);  // move not past the end
    QCOMPARE(GetLabelTextsFromToolBar(toolbar), (QStringList{"2", "1", "0"}));

    QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(2, 1, toolbar), true);  // move front
    QCOMPARE(GetLabelTextsFromToolBar(toolbar), (QStringList{"2", "0", "1"}));

    QCOMPARE(MoveWidgetAtFromIndexInFrontOfDestIndex(2, 0, toolbar), true);  // move to the first (push front)
    QCOMPARE(GetLabelTextsFromToolBar(toolbar), (QStringList{"1", "2", "0"}));
  }
};

#include "WidgetReorderHelperTest.moc"
REGISTER_TEST(WidgetReorderHelperTest, false)
