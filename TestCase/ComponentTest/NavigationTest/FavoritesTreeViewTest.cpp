#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QSignalSpy>

#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "FavoritesTreeView.h"
#include "NavigationFavorites.h"
#include "EndToExposePrivateMember.h"
#include <QInputDialog>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class FavoritesTreeViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void cleanupTestCase() {  //
    Configuration().clear();
  }

  void init() {  //
    GlobalMockObject::reset();
    Configuration().clear();
  }

  void cleanup() {  //
    GlobalMockObject::verify();
  }

  void selection_info_ok() {
    FavoritesTreeView view;
    QVERIFY(view.mFavModel->setDatas(QByteArray{}));
    QCOMPARE(view.mFavModel->rowCount(), 0);

    // 0行
    QCOMPARE(view.isExactlyOneGroupSelected(), false);
    QModelIndex rootSrcIndex;
    QCOMPARE(view.isNoSelectionOrExactlyOneGroup(&rootSrcIndex), true);
    QCOMPARE(rootSrcIndex, (QModelIndex{}));

    auto p0 = FavTreeNode::NewTreeNodeRoot();
    p0->appendRow(new FavTreeNode{TFavDataType{"group"}});
    p0->appendRow(new FavTreeNode{TFavDataType{"non_group", "path/to/non_group_folder"}});
    p0->appendRow(new FavTreeNode{TFavDataType{"group_other"}});

    view.mFavModel->setDatas(std::move(p0));
    QCOMPARE(view.mFavModel->rowCount(), 3);
    QCOMPARE(view.mFavModel->columnCount(), FavoriteItemData::COLUMN_COUNT);
    QCOMPARE(view.mFavProxyModel->rowCount(), 3);
    QCOMPARE(view.mFavProxyModel->columnCount(), FavoriteItemData::COLUMN_COUNT);

    QModelIndex grpSrcInd{view.mFavModel->index(0, 0, {})};
    QModelIndex nonGrpSrcInd{view.mFavModel->index(1, 0, {})};
    QModelIndex grpOtherSrcInd{view.mFavModel->index(2, 0, {})};
    QVERIFY(grpSrcInd.isValid());
    QVERIFY(nonGrpSrcInd.isValid());
    QVERIFY(grpOtherSrcInd.isValid());
    QVERIFY(view.mFavModel->flags(grpSrcInd).testFlag(Qt::ItemFlag::ItemIsSelectable));  // check if static used in error
    QVERIFY(view.mFavModel->flags(nonGrpSrcInd).testFlag(Qt::ItemFlag::ItemIsSelectable));
    QVERIFY(view.mFavModel->flags(grpOtherSrcInd).testFlag(Qt::ItemFlag::ItemIsSelectable));

    QModelIndex grpProxyInd{view.mFavProxyModel->mapFromSource(grpSrcInd)};
    QModelIndex nonGrpProxyInd{view.mFavProxyModel->mapFromSource(nonGrpSrcInd)};
    QModelIndex grpOtherProxyInd{view.mFavProxyModel->mapFromSource(grpOtherSrcInd)};

    QVERIFY(grpProxyInd.isValid());
    QVERIFY(nonGrpProxyInd.isValid());
    QVERIFY(grpOtherProxyInd.isValid());

    // 同时选中3行
    view.selectAll();
    QCOMPARE(view.selectionModel()->selectedRows().size(), 3);
    QCOMPARE(view.isExactlyOneGroupSelected(), false);
    QCOMPARE(view.isNoSelectionOrExactlyOneGroup(), false);

    // 只选中non-group
    view.selectionModel()->select(nonGrpProxyInd, QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
    QVERIFY(view.selectionModel()->isRowSelected(nonGrpProxyInd.row()));
    QCOMPARE(view.selectionModel()->selectedRows().size(), 1);
    QCOMPARE(view.isExactlyOneGroupSelected(), false);
    QCOMPARE(view.isNoSelectionOrExactlyOneGroup(), false);

    // 只选中一行group
    view.selectionModel()->clearSelection();
    view.selectionModel()->select(grpProxyInd, QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
    QCOMPARE(view.selectionModel()->selectedRows().size(), 1);
    QCOMPARE(view.isExactlyOneGroupSelected(), true);
    QCOMPARE(view.isNoSelectionOrExactlyOneGroup(), true);

    // 同时选中两行group
    view.selectionModel()->clearSelection();
    view.selectionModel()->select(QItemSelection{grpProxyInd, grpOtherProxyInd},
                                  QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
    QCOMPARE(view.selectionModel()->selectedRows().size(), 2);
    QCOMPARE(view.isExactlyOneGroupSelected(), false);
    QCOMPARE(view.isNoSelectionOrExactlyOneGroup(), false);
  }

  void reqIntoAPath_ok() {
    FavoritesTreeView view;
    QVERIFY(view.mFavModel->setDatas(QByteArray{}));
    QCOMPARE(view.mFavModel->rowCount(), 0);

    QSignalSpy reqIntoAPathSpy{&view, &FavoritesTreeView::reqIntoAPath};
    QCOMPARE(view.onItemClicked({}), false);
    QCOMPARE(reqIntoAPathSpy.count(), 0);

    auto p0 = FavTreeNode::NewTreeNodeRoot();
    p0->appendRow(new FavTreeNode{TFavDataType{"group"}});
    p0->appendRow(new FavTreeNode{TFavDataType{"non_group", "path/to/non_group_folder"}});
    view.mFavModel->setDatas(std::move(p0));

    QCOMPARE(view.mFavModel->rowCount(), 2);

    QModelIndex grpSrcInd{view.mFavModel->index(0, 0, {})};
    QModelIndex nonGrpSrcInd{view.mFavModel->index(1, 0, {})};

    QModelIndex grpProxyInd{view.mFavProxyModel->mapFromSource(grpSrcInd)};
    QModelIndex nonGrpProxyInd{view.mFavProxyModel->mapFromSource(nonGrpSrcInd)};

    // 直接调用函数
    // group, 不对应路径, 不会发射信号
    QCOMPARE(view.onItemClicked(grpProxyInd), false);
    QCOMPARE(reqIntoAPathSpy.count(), 0);
    // non-group, 发射信号
    QCOMPARE(view.onItemClicked(nonGrpProxyInd), true);
    QCOMPARE(reqIntoAPathSpy.count(), 1);
    QCOMPARE(reqIntoAPathSpy.takeLast(), (QVariantList{"path/to/non_group_folder", true}));

    // 通过信号, 预期和直接调用完全一致
    emit view.clicked(grpProxyInd);
    QCOMPARE(reqIntoAPathSpy.count(), 0);

    emit view.clicked(nonGrpProxyInd);
    QCOMPARE(reqIntoAPathSpy.count(), 1);
    QCOMPARE(reqIntoAPathSpy.takeLast(), (QVariantList{"path/to/non_group_folder", true}));
  }

  void onRenameDisplayRole_ok() {
    FavoritesTreeView view;
    QVERIFY(view.mFavModel->setDatas(QByteArray{}));
    QCOMPARE(view.mFavModel->rowCount(), 0);

    auto p0 = FavTreeNode::NewTreeNodeRoot();
    p0->appendRow(new FavTreeNode{TFavDataType{"group"}});
    p0->appendRow(new FavTreeNode{TFavDataType{"non_group", "path/to/non_group_folder"}});
    view.mFavModel->setDatas(std::move(p0));
    QCOMPARE(view.mFavModel->rowCount(), 2);

    MOCKER(QInputDialog::getText)
        .expects(exactly(4))                     //
        .will(returnValue(QString{}))            // 1. user cancel
        .then(returnValue(QString{"NewName"}))   // 2. node1 renamed to NewName pass
        .then(returnValue(QString{"NewName"}))   // 3. node1 try rename to NewName, skip. not pass
        .then(returnValue(QString{"NewName"}));  // 3. node2 renamed to NewName pass

    view.selectionModel()->clearSelection();
    QCOMPARE(view.onRenameDisplayRole(), false);  // 无选中
    view.selectAll();
    QCOMPARE(view.onRenameDisplayRole(), false);  // 多选

    QModelIndex grpSrcInd{view.mFavModel->index(0, 0, {})};
    QModelIndex nonGrpSrcInd{view.mFavModel->index(1, 0, {})};
    QModelIndex grpProxyInd{view.mFavProxyModel->mapFromSource(grpSrcInd)};
    QModelIndex nonGrpProxyInd{view.mFavProxyModel->mapFromSource(nonGrpSrcInd)};

    view.selectionModel()->select(grpProxyInd, QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
    QCOMPARE(view.onRenameDisplayRole(), false);  // 1. user cancel
    QCOMPARE(view.onRenameDisplayRole(), true);   // 2. node group renamed ok
    QCOMPARE(view.onRenameDisplayRole(), false);  // 3. node group already this name, skip

    view.selectionModel()->select(nonGrpProxyInd, QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
    QCOMPARE(view.onRenameDisplayRole(), true);  // non-groupName renamed ok, no check occupied

    QCOMPARE(view.mFavModel->rowCount(), 2);
  }

  void onAddAGroup_ok() {
    FavoritesTreeView view;
    QVERIFY(view.mFavModel->setDatas(QByteArray{}));
    QCOMPARE(view.mFavModel->rowCount(), 0);
    QCOMPARE(view.selectionModel()->hasSelection(), false);
    MOCKER(QInputDialog::getText)
        .expects(exactly(3))                          //
        .will(returnValue(QString{}))                 // 1. user cancel
        .then(returnValue(QString{"NewGroupName"}))   // 2.
        .then(returnValue(QString{"NewGroupName"}));  // 3. unchange

    // 没有任何选中, ok
    QCOMPARE(view.onAddAGroup(), false);  // 1. user cancel
    QCOMPARE(view.onAddAGroup(), true);   // 2. add ok
    QCOMPARE(view.mFavModel->rowCount(), 1);

    // 重置行为指定值 line=2
    auto p0 = FavTreeNode::NewTreeNodeRoot();
    p0->appendRow(new FavTreeNode{TFavDataType{"group"}});
    p0->appendRow(new FavTreeNode{TFavDataType{"non_group", "path/to/non_group_folder"}});
    view.mFavModel->setDatas(std::move(p0));
    QCOMPARE(view.mFavModel->rowCount(), 2);

    // 选中多行, nok, line=2
    view.selectAll();
    QCOMPARE(view.selectionModel()->selectedRows().size(), 2);
    QCOMPARE(view.onAddAGroup(), false);  // select no match

    // 只选中group, ok, line+=1
    QModelIndex grpSrcInd{view.mFavModel->index(0, 0, {})};
    QModelIndex grpProxyInd{view.mFavProxyModel->mapFromSource(grpSrcInd)};
    view.selectionModel()->select(grpProxyInd, QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
    QCOMPARE(view.selectionModel()->selectedRows().size(), 1);
    QCOMPARE(view.mFavModel->rowCount(), 2);
    QCOMPARE(view.mFavModel->rowCount(grpSrcInd), 0);
    QCOMPARE(view.onAddAGroup(), true);  // 3. add ok, 不care名称是否已占用
    QCOMPARE(view.mFavModel->rowCount(), 2);
    QCOMPARE(view.mFavModel->rowCount(grpSrcInd), 1);

    // 只选中non-group, nok
    QModelIndex nonGrpSrcInd{view.mFavModel->index(1, 0, {})};
    QModelIndex nonGrpProxyInd{view.mFavProxyModel->mapFromSource(nonGrpSrcInd)};
    view.selectionModel()->select(nonGrpProxyInd, QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
    QCOMPARE(view.selectionModel()->selectedRows().size(), 1);
    QCOMPARE(view.onAddAGroup(), false);  // 3. add ok, 不care名称是否已占用
    QCOMPARE(view.mFavModel->rowCount(), 2);
  }

  void onRemoveSelection_ok() {
    FavoritesTreeView view;
    QVERIFY(view.mFavModel->setDatas(QByteArray{}));
    QCOMPARE(view.mFavModel->rowCount(), 0);

    auto p0 = FavTreeNode::NewTreeNodeRoot();
    p0->appendRow(new FavTreeNode{TFavDataType{"group"}});
    p0->appendRow(new FavTreeNode{TFavDataType{"non_group", "path/to/non_group_folder"}});

    view.mFavModel->setDatas(std::move(p0));
    QCOMPARE(view.mFavModel->rowCount(), 2);

    QCOMPARE(view.onRemoveSelection(), 0);  // no row selected
    QCOMPARE(view.mFavModel->rowCount(), 2);
    view.selectAll();
    QCOMPARE(view.onRemoveSelection(), 2);  // both 2 rows selected deleted
    QCOMPARE(view.mFavModel->rowCount(), 0);
  }

  void contextMenuEvent_ok() {
    FavoritesTreeView view;
    QVERIFY(view.mFavModel->setDatas(QByteArray{}));
    view.contextMenuEvent(nullptr);  // will not crash down

    const QPoint tbCenterPnt = view.geometry().center();
    QContextMenuEvent event(QContextMenuEvent::Mouse, tbCenterPnt, view.mapToGlobal(tbCenterPnt));
    view.contextMenuEvent(&event);
    QCOMPARE(event.isAccepted(), true);
  }

  void setFilter_filter_proxy_ok() {
    FavoritesTreeView view;
    QVERIFY(view.mFavModel->setDatas(QByteArray{}));
    QCOMPARE(view.mFavModel->rowCount(), 0);
    QVERIFY(view.mFavProxyModel->m_sourceModel != nullptr);
    view.mFavProxyModel->setSourceModel(nullptr);
    QVERIFY(view.mFavProxyModel->m_sourceModel != nullptr);

    auto p0 = FavTreeNode::NewTreeNodeRoot();
    FavTreeNode* p00 = p0->appendRow(new FavTreeNode{TFavDataType{"group"}});
    FavTreeNode* p01 = p0->appendRow(new FavTreeNode{TFavDataType{"non_group", "path/to/non_group_folder"}});
    FavTreeNode* p02 = p0->appendRow(new FavTreeNode{TFavDataType{"Kaka"}});
    p01->appendRow(new FavTreeNode{TFavDataType{"Real Madrid Vacation", "Cristiano Ronaldo"}});
    p01->appendRow(new FavTreeNode{TFavDataType{"Brazil", "Neymar"}});
    view.mFavModel->setDatas(std::move(p0));
    QCOMPARE(view.mFavModel->rowCount(), 3);

    view.setFilter("folder");
    QCOMPARE(view.mFavProxyModel->rowCount(), 1);

    view.setFilter("group");
    QCOMPARE(view.mFavProxyModel->rowCount(), 2);

    view.setFilter("Cristiano Ronaldo");
    QCOMPARE(view.mFavProxyModel->rowCount(), 1);

    view.setFilter("Brazil");
    QCOMPARE(view.mFavProxyModel->rowCount(), 1);

    view.setFilter("NotExistKey");
    QCOMPARE(view.mFavProxyModel->rowCount(), 0);
  }

  void sortProxy_transfer_ok() {
    // 初始化： 访问次数降序
    FavoriteItemData::SaveInitialSortRole(FavoriteItemData::Role::ACCESS_COUNT_ROLE);
    FavoriteItemData::SaveSortOrderReverse(true);  // reverse

    auto p0 = FavTreeNode::NewTreeNodeRoot();
    FavTreeNode* p00 = p0->appendRow(new FavTreeNode{TFavDataType{"name0", "path2"}});
    p00->val.accessCount = 10;
    p00->val.lastAccess = 0;  // 1970-01-01 08:00:00 时刻访问过
    FavTreeNode* p01 = p0->appendRow(new FavTreeNode{TFavDataType{"name1", "path1"}});
    p01->val.accessCount = 44;
    p01->val.lastAccess = 60 * 1000;  // 1970-01-01 08:01:00 时刻访问过
    FavTreeNode* p02 = p0->appendRow(new FavTreeNode{TFavDataType{"name2", "path0"}});
    p02->val.accessCount = 7;
    p02->val.lastAccess = 60 * 60 * 1000;  // 1970-01-01 09:00:00 时刻访问过

    FavoritesTreeView view;
    QCOMPARE(view.mSortReverse->isChecked(), true);
    QVERIFY(view.mFavModel->setDatas(QByteArray{}));
    QCOMPARE(view.mFavProxyModel->isSortProxyInited(), true);

    const RecursiveFilterProxyTreeModel* proxyModel = view.mFavProxyModel;
    QCOMPARE(proxyModel->sortRole(), ((int)FavoriteItemData::Role::ACCESS_COUNT_ROLE));
    QCOMPARE(proxyModel->mSortOrder, Qt::SortOrder::DescendingOrder);
    QCOMPARE(view.mFavProxyModel->setSortOrder(true), false);  // unchange

    QVERIFY(view.mFavModel->setDatas(std::move(p0)));
    QCOMPARE(view.mFavModel->rowCount(), 3);

    // 初始化： 访问次数降序
    QCOMPARE(proxyModel->index(0, 0, {}).data(FavoriteItemData::Role::ACCESS_COUNT_ROLE).toInt(), 44);
    QCOMPARE(proxyModel->index(1, 0, {}).data(FavoriteItemData::Role::ACCESS_COUNT_ROLE).toInt(), 10);
    QCOMPARE(proxyModel->index(2, 0, {}).data(FavoriteItemData::Role::ACCESS_COUNT_ROLE).toInt(), 7);

    // 排序角色改为访问时间
    view.onSortRoleActionTriggered(view.mSortByLastAccessTime);
    QCOMPARE(proxyModel->index(0, 0, {}).data(FavoriteItemData::Role::LAST_ACCESS_ROLE).toInt(), 60 * 60 * 1000);
    QCOMPARE(proxyModel->index(1, 0, {}).data(FavoriteItemData::Role::LAST_ACCESS_ROLE).toInt(), 60 * 1000);
    QCOMPARE(proxyModel->index(2, 0, {}).data(FavoriteItemData::Role::LAST_ACCESS_ROLE).toInt(), 0);

    // 排序角色改为名称
    view.onSortRoleActionTriggered(view.mSortByLastAccessTime);
    QCOMPARE(proxyModel->index(0, 0, {}).data(FavoriteItemData::Role::DEF_NAME_TEXT_ROLE).toString(), "name2");
    QCOMPARE(proxyModel->index(1, 0, {}).data(FavoriteItemData::Role::DEF_NAME_TEXT_ROLE).toString(), "name1");
    QCOMPARE(proxyModel->index(2, 0, {}).data(FavoriteItemData::Role::DEF_NAME_TEXT_ROLE).toString(), "name0");

    QCOMPARE(view.mSortReverse->isChecked(), true);
    view.mSortReverse->toggle();
    QCOMPARE(view.mSortReverse->isChecked(), false);
    // 降序改为升序
    QCOMPARE(proxyModel->index(0, 0, {}).data(FavoriteItemData::Role::DEF_NAME_TEXT_ROLE).toString(), "name0");
    QCOMPARE(proxyModel->index(1, 0, {}).data(FavoriteItemData::Role::DEF_NAME_TEXT_ROLE).toString(), "name1");
    QCOMPARE(proxyModel->index(2, 0, {}).data(FavoriteItemData::Role::DEF_NAME_TEXT_ROLE).toString(), "name2");
  }

  void D0EV_and_drag_enter_move_drop_ok() {  //
    std::unique_ptr<FavoritesTreeView> pView{new FavoritesTreeView};
    QVERIFY(pView);
    pView->mFavModel->setDatas(QByteArray{});
    QCOMPARE(pView->mFavModel->rowCount(), 0);

    // not crash down
    pView->dragEnterEvent(nullptr);
    pView->dragMoveEvent(nullptr);
    pView->dropEvent(nullptr);

    QPoint dragEnterPos{pView->geometry().center()};
    QPoint dropPos{pView->geometry().bottomRight() - QPoint{1, 1}};
    {
      QMimeData emptyMimeData;
      emptyMimeData.setText("No urls");
      QDragEnterEvent ignoreDragEnter{dragEnterPos, Qt::IgnoreAction, &emptyMimeData, Qt::LeftButton, Qt::NoModifier};
      pView->dragEnterEvent(&ignoreDragEnter);  // 由父类自行处理
      QCOMPARE(ignoreDragEnter.isAccepted(), false);

      QDragMoveEvent ignoreDragMove{dragEnterPos, Qt::IgnoreAction, &emptyMimeData, Qt::LeftButton, Qt::NoModifier};
      pView->dragMoveEvent(&ignoreDragMove);  // 由父类自行处理
      QCOMPARE(ignoreDragMove.isAccepted(), false);

      QDropEvent ignoreDropEvent(dragEnterPos, Qt::IgnoreAction, &emptyMimeData, Qt::LeftButton, Qt::NoModifier);
      pView->dropEvent(&ignoreDropEvent);
      QCOMPARE(ignoreDropEvent.isAccepted(), false);  // 由父类自行处理
    }
    QCOMPARE(pView->mFavModel->rowCount(), 0);

    {  // urls ok
      QMimeData urlsMimeData;
      urlsMimeData.setText("2 urls");
      QList<QUrl> urlsList{QUrl::fromLocalFile(__FILE__), QUrl::fromLocalFile(__FILE__)};
      urlsMimeData.setUrls(urlsList);
      QDragEnterEvent dragEnter{dragEnterPos, Qt::CopyAction, &urlsMimeData, Qt::LeftButton, Qt::NoModifier};
      pView->dragEnterEvent(&dragEnter);
      QCOMPARE(dragEnter.isAccepted(), true);

      QDragMoveEvent dragMove{dragEnterPos, Qt::CopyAction, &urlsMimeData, Qt::LeftButton, Qt::NoModifier};
      pView->dragMoveEvent(&dragMove);
      QCOMPARE(dragMove.isAccepted(), true);

      QDropEvent dropInto(dragEnterPos, Qt::CopyAction, &urlsMimeData, Qt::LeftButton, Qt::NoModifier);
      pView->dropEvent(&dropInto);
      QCOMPARE(dropInto.isAccepted(), true);
    }
    QCOMPARE(pView->mFavModel->rowCount(), 2);

    {
      // move ok
      QModelIndex srcIndex = pView->mFavModel->index(0, 0, {});
      QMimeData* internalMimeData = pView->mFavModel->mimeData({srcIndex});
      QVERIFY(internalMimeData != nullptr);

      QDragEnterEvent dragEnter{dropPos, Qt::MoveAction, internalMimeData, Qt::LeftButton, Qt::NoModifier};
      pView->dragEnterEvent(&dragEnter);
      QCOMPARE(dragEnter.isAccepted(), true);

      QDragMoveEvent dragMove{dropPos, Qt::MoveAction, internalMimeData, Qt::LeftButton, Qt::NoModifier};
      pView->dragMoveEvent(&dragMove);
      QCOMPARE(dragMove.isAccepted(), true);

      QDropEvent dropInto(dropPos, Qt::MoveAction, internalMimeData, Qt::LeftButton, Qt::NoModifier);
      pView->dropEvent(&dropInto);
      QCOMPARE(dropInto.isAccepted(), true);
    }
    QCOMPARE(pView->mFavModel->rowCount(), 2);
  }

  void onStartFilter_ok() {
    NavigationFavorites naviFav;
    QVERIFY(naviFav.m_searchLineEdit != nullptr);
    QVERIFY(naviFav.view() != nullptr);

    const RecursiveFilterProxyTreeModel* proxyModel = naviFav.view()->mFavProxyModel;
    QVERIFY(proxyModel != nullptr);
    QCOMPARE(proxyModel->curFilter(), "");

    naviFav.m_searchLineEdit->setText("Kaka");
    emit naviFav.m_searchLineEdit->returnPressed();
    QCOMPARE(proxyModel->curFilter(), "Kaka");

    naviFav.m_searchLineEdit->setText("Cristinano Ronaldo");
    naviFav.m_startSearchAct->trigger();
    QCOMPARE(proxyModel->curFilter(), "Cristinano Ronaldo");

    naviFav.m_searchLineEdit->setText("");
    naviFav.onStartFilter();
    QCOMPARE(proxyModel->curFilter(), "");
  }
};

#include "FavoritesTreeViewTest.moc"
REGISTER_TEST(FavoritesTreeViewTest, false)
