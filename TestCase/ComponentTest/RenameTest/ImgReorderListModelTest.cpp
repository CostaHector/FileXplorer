#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ImgReorderListModel.h"
#include "EndToExposePrivateMember.h"

class ImgReorderListModelTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void default_ok() {
    ImgReorderListModel reorderModel;
    QCOMPARE(reorderModel.rowCount(), 0);
    QCOMPARE(reorderModel.data({}, Qt::DisplayRole).isValid(), false);
    QCOMPARE(reorderModel.setData({}, 0, Qt::ItemDataRole::EditRole), false);
    QCOMPARE(reorderModel.flags({}).testFlag(Qt::ItemFlag::ItemIsDropEnabled), true);
    QCOMPARE(reorderModel.mimeTypes().count(), 1);
    QCOMPARE(reorderModel.getOrderedNames().isEmpty(), true);
    QCOMPARE(reorderModel.mimeData({}), nullptr);
    QCOMPARE(reorderModel.mimeData({QModelIndex{}, QModelIndex{}}), nullptr);
    QCOMPARE(reorderModel.supportedDropActions().testFlag(Qt::MoveAction), true);

    QCOMPARE(reorderModel.dropMimeData(nullptr, Qt::DropAction::MoveAction, 0, 0, {}), false);

    // no mimeType field
    QMimeData mimeData;
    QCOMPARE(reorderModel.dropMimeData(&mimeData, Qt::DropAction::MoveAction, 0, 0, {}), false);

    // ignored action
    mimeData.setData(ImgReorderListModel::MIME_TYPE, QByteArray{});
    QCOMPARE(reorderModel.dropMimeData(&mimeData, Qt::DropAction::IgnoreAction, 0, 0, {}), false);

    // mimeType value empty
    mimeData.setData(ImgReorderListModel::MIME_TYPE, QByteArray{});
    QCOMPARE(reorderModel.dropMimeData(&mimeData, Qt::DropAction::MoveAction, 0, 0, {}), false);
  }

  void dropMimeData_ok() {
    const QStringList filesMixedWithImages{"/Ricardo Leite.jpg", "/Cristiano Ronaldo.txt", "/Robert Lewandowski.png"};
    const QString baseName{"Kaka"};
    const int startNo{9};
    const QString namePattern{" %1"};

    ImgReorderListModel reorderModel;
    QCOMPARE(reorderModel.setImagesToReorder(filesMixedWithImages, baseName, startNo, "invalid pattern not contain format percentage1"),
             false);
    QCOMPARE(reorderModel.setImagesToReorder(filesMixedWithImages, baseName, startNo, namePattern), true);
    QCOMPARE(reorderModel.rowCount(), 3);
    QCOMPARE(reorderModel.m_baseName, baseName);
    QCOMPARE(reorderModel.m_startNo, 9);
    QCOMPARE(reorderModel.m_namePattern, namePattern);

    QModelIndex ricardoIndex{reorderModel.index(0)};
    QModelIndex cr7Index{reorderModel.index(1)};
    QModelIndex lewanIndex{reorderModel.index(2)};
    QCOMPARE(reorderModel.data(ricardoIndex, Qt::DisplayRole).toInt(), 0);
    QCOMPARE(reorderModel.data(ricardoIndex, Qt::EditRole).toInt(), 0);
    QCOMPARE(reorderModel.data(cr7Index, Qt::DisplayRole).toInt(), 1);
    QCOMPARE(reorderModel.data(cr7Index, Qt::EditRole).toInt(), 1);
    QCOMPARE(reorderModel.data(lewanIndex, Qt::DisplayRole).toInt(), 2);
    QCOMPARE(reorderModel.data(lewanIndex, Qt::EditRole).toInt(), 2);

    QCOMPARE(reorderModel.data(lewanIndex, Qt::ForegroundRole), (QVariant()));

    const QVariant pxmap = reorderModel.data(cr7Index, Qt::DecorationRole);
    QVERIFY(pxmap.canConvert<QPixmap>());
    const QPixmap mpFromIconProvider = pxmap.value<QPixmap>();
    QCOMPARE(mpFromIconProvider.isNull(), false);

    const QStringList expectsNames{"Kaka 09", "Kaka 10", "Kaka 11"};
    QCOMPARE(reorderModel.getOrderedNames(), expectsNames);

    {
      // mimeData ok
      QModelIndexList all3indexes{ricardoIndex, cr7Index, lewanIndex};
      QMimeData* p3RowMimeData = reorderModel.mimeData(all3indexes);
      QVERIFY(p3RowMimeData != nullptr);
      QCOMPARE(p3RowMimeData->hasFormat(ImgReorderListModel::MIME_TYPE), true);

      // dropMimeData
      // 1. all line selected no need move, OrderedNames unchange
      QCOMPARE(reorderModel.dropMimeData(p3RowMimeData, Qt::DropAction::MoveAction, 0, 0, {}), false);
      QCOMPARE(reorderModel.dropMimeData(p3RowMimeData, Qt::DropAction::MoveAction, 1, 0, {}), false);
      QCOMPARE(reorderModel.dropMimeData(p3RowMimeData, Qt::DropAction::MoveAction, 2, 0, {}), false);
      QCOMPARE(reorderModel.getOrderedNames(), expectsNames);

      // 2. "Kaka 10", "Kaka 11" move to index 0
      QModelIndexList last2indexes{cr7Index, lewanIndex};
      QMimeData* p2RowMimeData = reorderModel.mimeData(last2indexes);
      QCOMPARE(reorderModel.dropMimeData(p2RowMimeData, Qt::DropAction::MoveAction, 0, 0, {}), true);
      QCOMPARE(reorderModel.getOrderedNames(), (QStringList{"Kaka 10", "Kaka 11", "Kaka 09"}));

      // 3. "Kaka 10" move to the after end index 3
      QModelIndexList first1indexes{ricardoIndex};
      QMimeData* p1RowMimeData = reorderModel.mimeData(first1indexes);
      QCOMPARE(reorderModel.dropMimeData(p1RowMimeData, Qt::DropAction::MoveAction, 3, 0, {}), true);
      QCOMPARE(reorderModel.getOrderedNames(), (QStringList{"Kaka 11", "Kaka 09", "Kaka 10"}));

      // 4. "Kaka 11" move to the index 2
      QModelIndexList first1indexes2{ricardoIndex};
      QMimeData* p1RowMimeData2 = reorderModel.mimeData(first1indexes2);
      QCOMPARE(reorderModel.dropMimeData(p1RowMimeData2, Qt::DropAction::MoveAction, -1, 0, lewanIndex), true);
      QCOMPARE(reorderModel.getOrderedNames(), (QStringList{"Kaka 09", "Kaka 11", "Kaka 10"}));

      // 5. "Kaka 09" move to the after end index 3
      QModelIndexList first1indexes3{ricardoIndex};
      QMimeData* p1RowMimeData3 = reorderModel.mimeData(first1indexes3);
      QCOMPARE(reorderModel.dropMimeData(p1RowMimeData3, Qt::DropAction::MoveAction, -1, 0, {}), true);
      QCOMPARE(reorderModel.getOrderedNames(), (QStringList{"Kaka 11", "Kaka 10", "Kaka 09"}));
    }

    // clear all rows
    QCOMPARE(reorderModel.setImagesToReorder(QStringList{}, QString{}, 0, " (%1)"), true);
    QCOMPARE(reorderModel.rowCount(), 0);
  }

  void setData_ok() {
    const QStringList filesMixedWithImages{"/Ricardo Leite.jpg", "/Cristiano Ronaldo.png"};
    const QString baseName{"Kaka"};
    const int startNo{90};
    const QString namePattern{" %1"};

    ImgReorderListModel reorderModel;
    QCOMPARE(reorderModel.setImagesToReorder(filesMixedWithImages, baseName, startNo, namePattern), true);
    QCOMPARE(reorderModel.rowCount(), 2);
    QCOMPARE(reorderModel.getOrderedNames(), (QStringList{"Kaka 90", "Kaka 91"}));
    QModelIndex kakaIndex{reorderModel.index(0)};
    QModelIndex cr7Index{reorderModel.index(1)};

    // invalid index
    QCOMPARE(reorderModel.setData({}, 157, Qt::EditRole), false);
    // role not accept
    QCOMPARE(reorderModel.setData(kakaIndex, 0, Qt::DisplayRole), false);
    // value not change
    QCOMPARE(reorderModel.setData(kakaIndex, 0, Qt::EditRole), false);
    // value already occupied(0, 1)
    QCOMPARE(reorderModel.setData(kakaIndex, 1, Qt::EditRole), false);
    // value changed ok
    QCOMPARE(reorderModel.setData(kakaIndex, 2, Qt::EditRole), true);
    QCOMPARE(reorderModel.data(kakaIndex).toInt(), 2);
    QCOMPARE(reorderModel.getOrderedNames(), (QStringList{"Kaka 92", "Kaka 91"}));

    // value not change
    QCOMPARE(reorderModel.setData(cr7Index, 1, Qt::EditRole), false);
    // value occupied(0, 1)
    QCOMPARE(reorderModel.setData(cr7Index, 0, Qt::EditRole), false);
    // value changed ok
    QCOMPARE(reorderModel.setData(cr7Index, 3, Qt::EditRole), true);
    QCOMPARE(reorderModel.data(cr7Index).toInt(), 3);
    QCOMPARE(reorderModel.getOrderedNames(), (QStringList{"Kaka 92", "Kaka 93"}));
  }
};

#include "ImgReorderListModelTest.moc"
REGISTER_TEST(ImgReorderListModelTest, false)
