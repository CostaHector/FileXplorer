#include "DuplicateVideosFinder.h"
#include "FileOpActs.h"

#include "VideoPlayerSplitter.h"
#include "VideoPlayerKey.h"
#include "SizeTool.h"
#include "Configuration.h"
#include "StyleSheet.h"

#include <QLabel>
#include <QComboBox>
#include <QKeyEvent>

using namespace DuplicateVideoDetectionCriteria;

DuplicateVideosFinder::DuplicateVideosFinder(QWidget* parent) : QSplitter{Qt::Orientation::Vertical, parent} {
  const int groupByModeInt = getConfig(VideoPlayerKey::DUPLICATE_GROUP_BY_MODE).toInt();
  const DVCriteriaE grpMode = GroupByModeInt2Enum(groupByModeInt);
  const int szDev = getConfig(VideoPlayerKey::DUPLICATE_FINDER_DEVIATION_FILESIZE).toInt();
  const int durDev = getConfig(VideoPlayerKey::DUPLICATE_FINDER_DEVIATION_DURATION).toInt();
  const bool videoPlayersEnabled = getConfig(VideoPlayerKey::DUPLICATE_VIDEO_PLAYERS_ENABLED).toBool();

  m_tb = new QToolBar{"Duplicate Videos Finder Toolbar", this};
  CHECK_NULLPTR_RETURN_VOID(m_tb);
  m_tb->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
  m_tb->setMaximumHeight(32);
  {
    // 1. Group by...
    m_GroupByCB = new (std::nothrow) QComboBox{this};
    CHECK_NULLPTR_RETURN_VOID(m_GroupByCB);
    m_GroupByCB->addItem(QIcon{":img/FILE_SIZE"},      "Size Tolerance (B)",      static_cast<int>(DVCriteriaE::SIZE));
    m_GroupByCB->addItem(QIcon{":img/VIDEO_DURATION"}, "Duration Tolerance (ms)", static_cast<int>(DVCriteriaE::DURATION));
    m_GroupByCB->addItem(QIcon{},                      "Size + Duration Tolerance", static_cast<int>(DVCriteriaE::SIZE_AND_DURATION));
    m_GroupByCB->setCurrentIndex(static_cast<int>(grpMode));
    m_GroupByCB->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Minimum);
    m_tb->addWidget(m_GroupByCB);

    sizeDevLE = new (std::nothrow) QLineEdit{QString::number(szDev), parent};
    CHECK_NULLPTR_RETURN_VOID(sizeDevLE);
    sizeDevLE->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Minimum);
    m_tb->addWidget(new QLabel{"Size"});
    m_tb->addWidget(sizeDevLE);

    durationDevLE = new (std::nothrow) QLineEdit{QString::number(durDev), parent};
    CHECK_NULLPTR_RETURN_VOID(durationDevLE);
    durationDevLE->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Minimum);
    m_tb->addWidget(new QLabel{"Duration"});
    m_tb->addWidget(durationDevLE);
  }

  {
    // selection function
    m_RecycleSelections = m_tb->addAction(QIcon{":img/MOVE_TO_TRASH_BIN"}, "Recycle");
    CHECK_NULLPTR_RETURN_VOID(m_RecycleSelections);
    m_RecycleSelections->setShortcut(QKeySequence(Qt::Key_Delete));
    m_RecycleSelections->setToolTip(QString("(%1) Move selected items to recycle bin").arg(m_RecycleSelections->shortcut().toString()));

    m_RenameSelectionsReplace = m_tb->addAction(QIcon(":img/NAME_STR_REPLACER_PATH"), "Rename Replace");
    CHECK_NULLPTR_RETURN_VOID(m_RenameSelectionsReplace);
    m_RenameSelectionsReplace->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_R));
    m_RenameSelectionsReplace->setToolTip(QString("<b>%1 (%2)</b><br/> Replace a substring in file name with another string.")
                                              .arg(m_RenameSelectionsReplace->text(), m_RenameSelectionsReplace->shortcut().toString()));
  }

  {
    // Search
    tableNameFilterLE = new (std::nothrow) QLineEdit{"", parent};
    CHECK_NULLPTR_RETURN_VOID(tableNameFilterLE);
    tableNameFilterLE->addAction(QIcon(":img/SEARCH"), QLineEdit::LeadingPosition);
    tableNameFilterLE->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
    tableNameFilterLE->setPlaceholderText("Enter table name to search here");
    m_tb->addWidget(tableNameFilterLE);
  }

  {
    // Undo/Redo
    m_tb->addActions(FileOpActs::GetInst().UNDO_REDO_RIBBONS->actions());
  }

  {
    m_showVideoPlayer = m_tb->addAction(QIcon{":/VideoPlayer/VIDEO_PLAYER_BASIC"}, "Video Players");
    m_showVideoPlayer->setCheckable(true);
    m_showVideoPlayer->setChecked(videoPlayersEnabled);
  }

  {
    // Central widget
    m_leftTables = new (std::nothrow) DuplicateVideosMetaInfoTable{this};
    CHECK_NULLPTR_RETURN_VOID(m_leftTables);

    m_RightGrpsTbl = new (std::nothrow) DuplicateVideoGroupsTable{this};
    CHECK_NULLPTR_RETURN_VOID(m_RightGrpsTbl);
    m_RightGrpsTbl->initGroupMember(grpMode, szDev, durDev);

    m_bottomDetailsTbl = new (std::nothrow) DuplicateVideoDetailsTable{this};
    CHECK_NULLPTR_RETURN_VOID(m_bottomDetailsTbl);
    m_bottomDetailsTbl->setSharedMember(&m_RightGrpsTbl->m_leftGrpModel->m_groupedVidLstArr,  //
                                        &m_RightGrpsTbl->m_leftGrpModel->m_currentDiffer);    // never forget call this one

    m_tables_group_LR_splitter = new (std::nothrow) QSplitter{Qt::Orientation::Horizontal, this};
    CHECK_NULLPTR_RETURN_VOID(m_tables_group_LR_splitter);
    m_tables_group_LR_splitter->addWidget(m_leftTables);
    m_tables_group_LR_splitter->addWidget(m_RightGrpsTbl);

    m_videoPlayers = new VideoPlayerSplitter{Qt::Orientation::Horizontal, this};

    addWidget(m_tb);
    addWidget(m_tables_group_LR_splitter);
    addWidget(m_bottomDetailsTbl);
    addWidget(m_videoPlayers);

    m_videoPlayers->onShowViewPlayer(videoPlayersEnabled);
  }

  subscribe();

  setWindowIcon(QIcon(":img/DUPLICATE_VIDEOS_FINDER"));

  m_leftTables->LoadAiMediaTableNames();

  UpdateWindowsTitleGroupInfo(m_RightGrpsTbl->GetCurrentDupVideoGroupInfo());
  UpdateWindowsTitleMetaInfo(m_leftTables->GetCurrentDupVideoMetaInfo());
  UpdateWindowsTitle();
}

DuplicateVideosFinder::~DuplicateVideosFinder() {
  setConfig(VideoPlayerKey::DUPLICATE_VIDEO_PLAYERS_ENABLED, m_showVideoPlayer->isChecked());
}

void DuplicateVideosFinder::ReadSettings() {
  m_geometryRestoredOnce = true;
  if (Configuration().contains("Geometry/DuplicateVideosFinder")) {
    m_tables_group_LR_splitter->restoreState(Configuration().value("DuplicateVideosFinder/LRSplitterState").toByteArray());
    restoreState(Configuration().value("DuplicateVideosFinder/State").toByteArray());
  } else {
    setGeometry(SizeTool::DEFAULT_GEOMETRY);
  }
}

void DuplicateVideosFinder::showEvent(QShowEvent* event) {
  if (!m_geometryRestoredOnce) {
    ReadSettings();
  }
  QSplitter::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void DuplicateVideosFinder::closeEvent(QCloseEvent* event) {
  saveSplitterStateOnClose();
  QSplitter::closeEvent(event);
}

void DuplicateVideosFinder::saveSplitterStateOnClose() {
  Configuration().setValue("DuplicateVideosFinder/LRSplitterState", m_tables_group_LR_splitter->saveState());
  Configuration().setValue("DuplicateVideosFinder/State", saveState());
}

void DuplicateVideosFinder::keyPressEvent(QKeyEvent* e) {
  if (e->modifiers() == Qt::KeyboardModifier::ControlModifier && e->key() == Qt::Key_Insert) {
    const QModelIndex& playInd = m_bottomDetailsTbl->currentIndex();
    if (!playInd.isValid()) {
      LOG_W("invalid index ignore copy everything name");
      e->ignore();
      return;
    }
    m_bottomDetailsTbl->on_effectiveNameCopiedForEverything(playInd);
    e->accept();
    return;
  } else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
    if (m_bottomDetailsTbl->hasFocus() && e->modifiers() == Qt::KeyboardModifier::NoModifier) {
      const QStringList paths = m_bottomDetailsTbl->selectedFilesFullPath();
      if (paths.isEmpty()) {
        e->ignore();
        return;
      }
      if (m_videoPlayers != nullptr) {
        m_videoPlayers->onPlayVideos(paths);
      } else {
        const QModelIndex playProxyInd = m_bottomDetailsTbl->currentIndex();
        emit m_bottomDetailsTbl->doubleClicked(playProxyInd);
      }
      e->accept();
      return;
    }
  }
  QSplitter::keyPressEvent(e);
}

void DuplicateVideosFinder::subscribe() {
  /* below will change duplicates group result. So clear left selection and right associated current left row needed */
  connect(m_leftTables, &DuplicateVideosMetaInfoTable::analyzeTablesFinished, this, &DuplicateVideosFinder::onAnalyzeDuplicatesInVideosList);
  using CURRENT_INDEX_CHANGED_MEMBER_FUNC = void (QComboBox::*)(int);
  connect(m_GroupByCB, (CURRENT_INDEX_CHANGED_MEMBER_FUNC)&QComboBox::currentIndexChanged, this,  &DuplicateVideosFinder::onDifferTypeChanged);
  connect(durationDevLE, &QLineEdit::returnPressed, this, &DuplicateVideosFinder::onChangeDurationDeviation);
  connect(sizeDevLE, &QLineEdit::returnPressed, this, &DuplicateVideosFinder::onChangeSizeDeviation);
  /* above invalidateLeftGrpTblData needed before */

  using SET_FILTER_REGULAR_EXPRESSION_FUNC = void (QSortFilterProxyModel::*)(const QString&);
  connect(tableNameFilterLE, &QLineEdit::textChanged, m_leftTables->m_sortProxy, (SET_FILTER_REGULAR_EXPRESSION_FUNC)&QSortFilterProxyModel::setFilterRegularExpression);
  connect(m_RightGrpsTbl, &DuplicateVideoGroupsTable::leftSelectionChanged, m_bottomDetailsTbl, &DuplicateVideoDetailsTable::onLeftVideoGroupsTableSelectionChanged);

  connect(m_leftTables, &DuplicateVideosMetaInfoTable::windowTitleChanged, this, &DuplicateVideosFinder::UpdateWindowsTitleMetaInfo);
  connect(m_RightGrpsTbl, &DuplicateVideoGroupsTable::windowTitleChanged, this, &DuplicateVideosFinder::UpdateWindowsTitleGroupInfo);

  connect(m_RenameSelectionsReplace, &QAction::triggered, m_bottomDetailsTbl, &DuplicateVideoDetailsTable::on_RenameReplace);
  connect(m_RecycleSelections, &QAction::triggered, m_bottomDetailsTbl, &DuplicateVideoDetailsTable::on_RecycleSelection);
  connect(m_showVideoPlayer, &QAction::toggled, m_videoPlayers, &VideoPlayerSplitter::onShowViewPlayer);
}

void DuplicateVideosFinder::invalidateLeftGrpTblData() {
  m_RightGrpsTbl->clearSelection();
}

void DuplicateVideosFinder::onAnalyzeDuplicatesInVideosList(const DupVidMetaInfoList& needAnalyzeVidLst) {
  invalidateLeftGrpTblData();
  m_RightGrpsTbl->onDuplicateVideosListChanged(needAnalyzeVidLst);
}

void DuplicateVideosFinder::onDifferTypeChanged(const int grpByIndex) {
  const int grpModeInt = m_GroupByCB->itemData(grpByIndex).toInt();
  const DVCriteriaE grpMode = GroupByModeInt2Enum(grpModeInt);
  invalidateLeftGrpTblData();
  setConfig(VideoPlayerKey::DUPLICATE_GROUP_BY_MODE, (int)grpMode);
  m_RightGrpsTbl->setDifferType(grpMode);
}

void DuplicateVideosFinder::onChangeSizeDeviation() {
  const QString& szDevStr = sizeDevLE->text();
  bool isIntValid = false;
  int dev = szDevStr.toInt(&isIntValid);
  if (!isIntValid || dev <= 0) {
    LOG_W("size str[%s] is not valid", qPrintable(szDevStr));
    return;
  }
  invalidateLeftGrpTblData();
  setConfig(VideoPlayerKey::DUPLICATE_FINDER_DEVIATION_FILESIZE, dev);
  m_RightGrpsTbl->setDeviationSize(dev);
}

void DuplicateVideosFinder::onChangeDurationDeviation() {
  const QString& durDevStr = durationDevLE->text();
  bool isIntValid = false;
  int dev = durDevStr.toInt(&isIntValid);
  if (!isIntValid || dev <= 0) {
    LOG_W("duration str[%s] is not valid", qPrintable(durDevStr));
    return;
  }
  invalidateLeftGrpTblData();
  setConfig(VideoPlayerKey::DUPLICATE_FINDER_DEVIATION_DURATION, dev);
  m_RightGrpsTbl->setDeviationDuration(dev);
}

void DuplicateVideosFinder::UpdateWindowsTitleGroupInfo(const QString& dupVidGroupInfoTitle) {
  mTitleGrpInto = dupVidGroupInfoTitle;
  UpdateWindowsTitle();
}

void DuplicateVideosFinder::UpdateWindowsTitleMetaInfo(const QString& dupVidTblMetaInfoTitle) {
  mTitleMetaInfo = dupVidTblMetaInfoTitle;
  UpdateWindowsTitle();
}

void DuplicateVideosFinder::UpdateWindowsTitle() {
  setWindowTitle("Duplicate Videos: " + mTitleMetaInfo + "|" + mTitleGrpInto);
}
