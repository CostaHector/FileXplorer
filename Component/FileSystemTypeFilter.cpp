#include "FileSystemTypeFilter.h"
#include "PublicVariable.h"
#include "MemoryKey.h"

#include <QDebug>
#include <QSplitter>

const QDir::Filters FileSystemTypeFilter::DEFAULT_FILTER_FLAG{MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.v.toInt()};

FileSystemTypeFilter::FileSystemTypeFilter(QWidget* parent)
  : QToolButton{parent},
  m_flagWhenFilterEnabled{Configuration().value(MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.name, MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.v).toInt()},
  m_isIncludingSubdirectory{Configuration().value(MemoryKey::SEARCH_INCLUDING_SUBDIRECTORIES.name, MemoryKey::SEARCH_INCLUDING_SUBDIRECTORIES.v).toBool()} {
  FILTER_SWITCH->setCheckable(true);
  FILTER_SWITCH->setChecked(Configuration().value("FILE_SYSTEM_IS_FILTER_SWITCH_ON_RESTORED", true).toBool());
  FILTER_SWITCH->setToolTip(
      "1. QFileSystemModel->setFilter()\n"
      "2. SearchModel->setFilter()");

  DISABLE_ENTRIES_DONT_PASS_FILTER->setCheckable(true);
  DISABLE_ENTRIES_DONT_PASS_FILTER->setChecked(
      Configuration().value(MemoryKey::DISABLE_ENTRIES_DONT_PASS_FILTER.name, MemoryKey::DISABLE_ENTRIES_DONT_PASS_FILTER.v).toBool());
  DISABLE_ENTRIES_DONT_PASS_FILTER->setToolTip(
      "This property holds whether files that don't pass the name filter are disable(true) or hide(false)\n"
      "This property is true disable by default.\n"
      "1. QFileSystemModel->setNameFilters()\n"
      "2. SearchModel->setFilter()");

  INCLUDING_SUBDIRECTORIES->setCheckable(true);
  INCLUDING_SUBDIRECTORIES->setChecked(m_isIncludingSubdirectory);
  INCLUDING_SUBDIRECTORIES->setToolTip(
      "List entries inside all subdirectories as well.\n"
      "1. Skip for QFileSystemModel\n"
      "2. In searchModel QDirIterator(flags=QDirIterator::IteratorFlag::Subdirectories)");

  m_FILTER_FLAG_AGS->addAction(FILES);
  m_FILTER_FLAG_AGS->addAction(DIRS);
  m_FILTER_FLAG_AGS->addAction(DRIVES);
  m_FILTER_FLAG_AGS->addAction(HIDDEN);
  m_FILTER_FLAG_AGS->addAction(NO_DOT);
  m_FILTER_FLAG_AGS->addAction(NO_DOT_DOT);
  m_FILTER_FLAG_AGS->addAction(CASE_SENSITIVE);
  m_FILTER_FLAG_AGS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  fileTypeFilterMenu->addActions(m_FILTER_FLAG_AGS->actions());
  fileTypeFilterMenu->addSeparator();
  fileTypeFilterMenu->addAction(DISABLE_ENTRIES_DONT_PASS_FILTER);
  fileTypeFilterMenu->addSeparator();
  fileTypeFilterMenu->addAction(INCLUDING_SUBDIRECTORIES);

  fileTypeFilterMenu->setToolTipsVisible(true);

  qDebug() << "Stored switch on: " << FILTER_SWITCH->isChecked() << ", flags:" << m_flagWhenFilterEnabled;
  // restore Checkable / checked / Enabled;
  for (QAction* typeAct : m_FILTER_FLAG_AGS->actions()) {
    typeAct->setCheckable(true);
    typeAct->setChecked(m_text2FilterFlag[typeAct->text()] & m_flagWhenFilterEnabled);
  }
  setDefaultAction(FILTER_SWITCH);
  setPopupMode(QToolButton::ToolButtonPopupMode::MenuButtonPopup);
  setMenu(fileTypeFilterMenu);
  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
}

void FileSystemTypeFilter::BindFileSystemModel(QFileSystemModel* newModel) {
  if (newModel == nullptr) {
    qWarning("skip. don't try to bind a nullptr");
    return;
  }
  if (_fsmModel != nullptr) {
    qWarning("skip. don't try to rebind");
  }
  m_modelType = MODEL_TYPE::FILE_SYSTEM_MODEL;
  _fsmModel = newModel;
  if (FILTER_SWITCH->isChecked()) {
    initFilterAgent(m_flagWhenFilterEnabled);
  } else {
    initFilterAgent(DEFAULT_FILTER_FLAG);
  }
  initNameFilterDisablesAgent(DISABLE_ENTRIES_DONT_PASS_FILTER->isChecked());
  connect(fileTypeFilterMenu, &QMenu::triggered, this, &FileSystemTypeFilter::onTypeChecked);
  connect(FILTER_SWITCH, &QAction::triggered, this, &FileSystemTypeFilter::onSwitchChanged);
  connect(DISABLE_ENTRIES_DONT_PASS_FILTER, &QAction::triggered, this, &FileSystemTypeFilter::onGrayOrHideChanged);
  connect(INCLUDING_SUBDIRECTORIES, &QAction::triggered, this, &FileSystemTypeFilter::changeSearchModelIteratorFlagAgent);
}

void FileSystemTypeFilter::BindFileSystemModel(AdvanceSearchModel* newModel, SearchProxyModel* newProxyModel) {
  if (newModel == nullptr || newProxyModel == nullptr) {
    qWarning("skip. newModel is nullptr");
    return;
  }
  if (_searchSourceModel != nullptr || _searchProxyModel != nullptr) {
    qWarning("Skip. reject rebind.");
    return;
  }
  m_modelType = MODEL_TYPE::SEARCH_MODEL;
  _searchSourceModel = newModel;
  _searchProxyModel = newProxyModel;

  if (FILTER_SWITCH->isChecked()) {
    initFilterAgent(m_flagWhenFilterEnabled);
  } else {
    initFilterAgent(DEFAULT_FILTER_FLAG);
  }
  initNameFilterDisablesAgent(DISABLE_ENTRIES_DONT_PASS_FILTER->isChecked());

  connect(fileTypeFilterMenu, &QMenu::triggered, this, &FileSystemTypeFilter::onTypeChecked);
  connect(FILTER_SWITCH, &QAction::triggered, this, &FileSystemTypeFilter::onSwitchChanged);
  connect(DISABLE_ENTRIES_DONT_PASS_FILTER, &QAction::triggered, this, &FileSystemTypeFilter::onGrayOrHideChanged);
  connect(INCLUDING_SUBDIRECTORIES, &QAction::triggered, this, &FileSystemTypeFilter::changeSearchModelIteratorFlagAgent);
}

void FileSystemTypeFilter::onSwitchChanged(bool isOn) {
  Configuration().setValue("FILE_SYSTEM_IS_FILTER_SWITCH_ON_RESTORED", isOn);
  if (m_flagWhenFilterEnabled == INVALID_MODEL) {
    return;
  }
  if (not isOn) {
    m_flagWhenFilterEnabled = filterAgent();
    setFilterAgent(DEFAULT_FILTER_FLAG);
  } else {  // recover last flag
    setFilterAgent(m_flagWhenFilterEnabled);
  }
  qDebug() << "Save switch on: " << isOn << ". Keep flags:" << m_flagWhenFilterEnabled;
}

void FileSystemTypeFilter::onGrayOrHideChanged(bool isGray) {
  Configuration().setValue(MemoryKey::DISABLE_ENTRIES_DONT_PASS_FILTER.name, isGray);
  if (m_flagWhenFilterEnabled == INVALID_MODEL) {
    return;
  }
  setNameFilterDisablesAgent(isGray);
}

void FileSystemTypeFilter::onTypeChecked(QAction* act) {
  if (act->isChecked()) {
    m_flagWhenFilterEnabled |= m_text2FilterFlag[act->text()];
  } else {
    m_flagWhenFilterEnabled &= ~m_text2FilterFlag[act->text()];
  }
  Configuration().setValue(MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.name, int(m_flagWhenFilterEnabled));
  if (FILTER_SWITCH->isChecked()) {  // take into effect only switch is on. Record changes even switch is off
    setFilterAgent(m_flagWhenFilterEnabled);
  }
  qDebug() << "Keep switch on: " << FILTER_SWITCH->isChecked() << ". Save flags:" << m_flagWhenFilterEnabled;
}

QDir::Filters FileSystemTypeFilter::filterAgent() const {
  switch (m_modelType) {
    case FILE_SYSTEM_MODEL:
      return _fsmModel->filter();
    case SEARCH_MODEL:
      return _searchSourceModel->filter();
    default:
      qDebug("invalid Model. return default filter");
      return QDir::Filter::NoFilter;
  }
}

void FileSystemTypeFilter::setFilterAgent(QDir::Filters filters) {
  switch (m_modelType) {
    case FILE_SYSTEM_MODEL:
      _fsmModel->setFilter(filters);
      return;
    case SEARCH_MODEL:
      _searchSourceModel->setFilter(filters);
      return;
    default:
      qDebug("invalid Model. not setFilterAgent");
      return;
  }
}

void FileSystemTypeFilter::initFilterAgent(QDir::Filters filters) {
  switch (m_modelType) {
    case FILE_SYSTEM_MODEL:
      // Todo: may cause double load
      _fsmModel->setFilter(filters);
      return;
    case SEARCH_MODEL:
      _searchSourceModel->initFilter(filters);
      return;
    default:
      qDebug("invalid Model. not initFilterAgent");
      return;
  }
}

void FileSystemTypeFilter::initNameFilterDisablesAgent(bool enable) {
  switch (m_modelType) {
    case FILE_SYSTEM_MODEL:
      _fsmModel->setNameFilterDisables(enable);
      return;
    case SEARCH_MODEL:
      _searchProxyModel->initNameFilterDisables(enable);
      return;
    default:
      qDebug("invalid Model. not setNameFilterDisable");
      return;
  }
}

void FileSystemTypeFilter::setNameFilterDisablesAgent(bool enable) {
  switch (m_modelType) {
    case FILE_SYSTEM_MODEL:
      _fsmModel->setNameFilterDisables(enable);
      return;
    case SEARCH_MODEL:
      _searchProxyModel->setNameFilterDisables(enable);
      return;
    default:
      qDebug("invalid Model. not setNameFilterDisable");
      return;
  }
}

void FileSystemTypeFilter::initSearchModelIteratorFlagAgent() {
  if (m_modelType != SEARCH_MODEL) {
    return;
  }
  auto iteratorFlag = m_isIncludingSubdirectory ? QDirIterator::IteratorFlag::Subdirectories : QDirIterator::IteratorFlag::NoIteratorFlags;
  _searchSourceModel->initIteratorFlag(iteratorFlag);  // only change value not set
}

void FileSystemTypeFilter::changeSearchModelIteratorFlagAgent(const bool including) {
  Configuration().setValue(MemoryKey::SEARCH_INCLUDING_SUBDIRECTORIES.name, including);
  m_isIncludingSubdirectory = including;
  if (m_modelType != SEARCH_MODEL) {
    return;
  }
  auto iteratorFlag = m_isIncludingSubdirectory ? QDirIterator::IteratorFlag::Subdirectories : QDirIterator::IteratorFlag::NoIteratorFlags;
  _searchSourceModel->setIteratorFlag(iteratorFlag);  // change value and set
}

#include <QTableView>
class FileSystemFilter : public QSplitter {
public:
  FileSystemFilter(QWidget* parent = nullptr) : QSplitter(parent) {
    auto* m_tv = new QTableView(this);
    auto* m_model = new QFileSystemModel(this);
    m_model->setNameFilterDisables(false);
    m_tv->setModel(m_model);
    m_tv->setRootIndex(m_model->setRootPath("./"));

    auto* fileTypeFilterButton = new FileSystemTypeFilter;
    fileTypeFilterButton->BindFileSystemModel(m_model);

    addWidget(m_tv);
    addWidget(fileTypeFilterButton);
  }

  QSize sizeHint() const { return QSize(1024, 768); }
};

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  FileSystemFilter fsf;
  fsf.show();
  a.exec();
  return 0;
}
#endif
