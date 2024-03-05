#include "ToolButtonFileSystemTypeFilter.h"
#include "PublicVariable.h"

#include <QDebug>
#include <QSplitter>

const QDir::Filters ToolButtonFileSystemTypeFilter::DEFAULT_FILTER_FLAG =
    QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::Drives | QDir::Filter::NoDotAndDotDot;

ToolButtonFileSystemTypeFilter::ToolButtonFileSystemTypeFilter(QWidget* parent)
    : QToolButton(parent),
      m_flagWhenFilterEnabled{PreferenceSettings().value("FILE_SYSTEM_FLAG_WHEN_FILTER_ENABLED", int(DEFAULT_FILTER_FLAG)).toInt()} {
  FILTER_SWITCH->setCheckable(true);
  FILTER_SWITCH->setChecked(PreferenceSettings().value("FILE_SYSTEM_IS_FILTER_SWITCH_ON_RESTORED", true).toBool());

  qDebug() << PreferenceSettings().value("FILE_SYSTEM_GRAY_IF_FILTERED", false).toBool();
  GRAY_IF_FILTERED->setCheckable(true);
  GRAY_IF_FILTERED->setChecked(true);
  GRAY_IF_FILTERED->setToolTip("Gray items meet rule when enabled. hide when Disabled");

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
  fileTypeFilterMenu->addAction(GRAY_IF_FILTERED);
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

void ToolButtonFileSystemTypeFilter::BindFileSystemModel(QAbstractItemModel* newModel) {
  if (_model != nullptr) {
    qWarning("skip. model is already not nullptr. don't rebind");
    return;
  }
  if (newModel == nullptr) {
    qWarning("skip. don't try to bind a nullptr");
    return;
  }
  _model = _fsmModel = dynamic_cast<QFileSystemModel*>(newModel);
  if (_fsmModel != nullptr) {
    m_modelType = MODEL_TYPE::FILE_SYSTEM_MODEL;
  } else {
    _model = _searchModel = dynamic_cast<MySearchModel*>(newModel);
    if (_searchModel != nullptr) {
      m_modelType = MODEL_TYPE::SEARCH_MODEL;
    } else {
      qDebug("BindFileSystemModel failed. _model cannot converted QFileSystemModel* or MySearchModel*");
      return;
    }
  }
  if (FILTER_SWITCH->isChecked()) {
    setFilterAgent(m_flagWhenFilterEnabled);
  } else {
    setFilterAgent(DEFAULT_FILTER_FLAG);
  }
  setNameFilterDisablesAgent(GRAY_IF_FILTERED->isChecked());
  connect(fileTypeFilterMenu, &QMenu::triggered, this, &ToolButtonFileSystemTypeFilter::onTypeChecked);
  connect(FILTER_SWITCH, &QAction::triggered, this, &ToolButtonFileSystemTypeFilter::onSwitchChanged);
  connect(GRAY_IF_FILTERED, &QAction::triggered, this, &ToolButtonFileSystemTypeFilter::onGrayOrHideChanged);
}

void ToolButtonFileSystemTypeFilter::onSwitchChanged(bool isOn) {
  PreferenceSettings().setValue("FILE_SYSTEM_IS_FILTER_SWITCH_ON_RESTORED", isOn);
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

void ToolButtonFileSystemTypeFilter::onGrayOrHideChanged(bool isGray) {
  PreferenceSettings().setValue("FILE_SYSTEM_GRAY_IF_FILTERED", isGray);
  if (m_flagWhenFilterEnabled == INVALID_MODEL) {
    return;
  }
  setNameFilterDisablesAgent(isGray);
}

void ToolButtonFileSystemTypeFilter::onTypeChecked(QAction* act) {
  if (act->isChecked()) {
    m_flagWhenFilterEnabled |= m_text2FilterFlag[act->text()];
  } else {
    m_flagWhenFilterEnabled &= ~m_text2FilterFlag[act->text()];
  }
  PreferenceSettings().setValue("FILE_SYSTEM_FLAG_WHEN_FILTER_ENABLED", int(m_flagWhenFilterEnabled));
  if (FILTER_SWITCH->isChecked()) {  // take into effect only switch is on. Record changes even switch is off
    setFilterAgent(m_flagWhenFilterEnabled);
  }
  qDebug() << "Keep switch on: " << FILTER_SWITCH->isChecked() << ". Save flags:" << m_flagWhenFilterEnabled;
}

QDir::Filters ToolButtonFileSystemTypeFilter::filterAgent() const {
  switch (m_modelType) {
    case FILE_SYSTEM_MODEL:
      return _fsmModel->filter();
    case SEARCH_MODEL:
      return _searchModel->filter();
    default:
      return QDir::Filter::NoFilter;
  }
}

void ToolButtonFileSystemTypeFilter::setFilterAgent(QDir::Filters filters) {
  switch (m_modelType) {
    case FILE_SYSTEM_MODEL:
      _fsmModel->setFilter(filters);
      return;
    case SEARCH_MODEL:
      _searchModel->setFilter(filters);
      return;
    default:
      return;
  }
}

void ToolButtonFileSystemTypeFilter::setNameFilterDisablesAgent(bool enable) {
  switch (m_modelType) {
    case FILE_SYSTEM_MODEL:
      _fsmModel->setNameFilterDisables(enable);
      return;
    case SEARCH_MODEL:
      _searchModel->setNameFilterDisables(enable);
      return;
    default:
      return;
  }
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

    auto* fileTypeFilterButton = new ToolButtonFileSystemTypeFilter;
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
