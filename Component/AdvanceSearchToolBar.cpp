#include "AdvanceSearchToolBar.h"
#include "PublicMacro.h"
#include "PublicVariable.h"
#include "MemoryKey.h"
#include "FileBasicOperationsActions.h"
#include <QLayout>

AdvanceSearchToolBar::AdvanceSearchToolBar(const QString& title, QWidget* parent)  //
  : QToolBar{title, parent}                                                      //
{
  // if its height does not same as others. check if parent is provided the same as others
  CHECK_NULLPTR_RETURN_VOID(parent)

  m_nameFilterCB = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_nameFilterCB)
  m_nameFilterCB->setInsertPolicy(QComboBox::InsertPolicy::InsertAtTop);
  m_nameFilterCB->setEditable(true);
  m_nameFilterCB->lineEdit()->addAction(QIcon(":img/FILE_SYSTEM_FILTER"), QLineEdit::LeadingPosition);
  m_nameFilterCB->lineEdit()->setClearButtonEnabled(true);
  m_nameFilterCB->addItem(Configuration().value(MemoryKey::ADVANCE_SEARCH_LINEEDIT_VALUE.name, MemoryKey::ADVANCE_SEARCH_LINEEDIT_VALUE.v).toString());
  m_nameFilterCB->addItem("\\.xltd$");
  m_nameFilterCB->addItem("\\.torrent$");
  m_nameFilterCB->addItem("\\.!ut$");
  m_nameFilterCB->addItem("\\.html$");
  m_nameFilterCB->addItem("\\.txt$");
  m_nameFilterCB->addItem("\\.json$");
  m_nameFilterCB->addItem("\\.scn$");
  m_nameFilterCB->addItem("\\.md$");
  m_nameFilterCB->addItem("\\.pson$");
  m_nameFilterCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

  m_searchFilterButton = new (std::nothrow) TypeFilterButton{ModelFilterE::ADVANCE_SEARCH, this};
  CHECK_NULLPTR_RETURN_VOID(m_searchFilterButton)
  m_searchFilterButton->setToolTip("Filter file types by extension");

  m_searchModeComboBox = new (std::nothrow) SearchModeComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_searchModeComboBox)
  m_searchModeComboBox->setToolTip("Search Mode:\n1. full match\n2. regex\n3. regex+file name");

  m_searchCaseButton = new SearchCaseMatterToolButton{this};
  CHECK_NULLPTR_RETURN_VOID(m_searchCaseButton)
  m_searchCaseButton->setToolTip("Match Case Switch");

  m_contentCB = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_contentCB)
  m_contentCB->setEditable(true);
  m_contentCB->addItem("nonporn");
  m_contentCB->addItem(QString{50, QChar{' '}});
  m_contentCB->addItem(Configuration().value(MemoryKey::ADVANCE_SEARCH_CONTENTS_LINEEDIT_VALUE.name, MemoryKey::ADVANCE_SEARCH_CONTENTS_LINEEDIT_VALUE.v).toString());
  m_contentCB->lineEdit()->addAction(QIcon(":img/FILE_SYSTEM_FILTER"), QLineEdit::LeadingPosition);
  m_contentCB->lineEdit()->setClearButtonEnabled(true);
  m_contentCB->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
  m_contentCB->setToolTip("Search plain text file contents here");

  addWidget(m_nameFilterCB);
  addAction(g_fileBasicOperationsActions()._FORCE_RESEARCH);
  addWidget(m_searchFilterButton);
  addWidget(m_searchModeComboBox);
  addWidget(m_searchCaseButton);
  addWidget(m_contentCB);

  onSearchModeChanged(m_searchModeComboBox->currentText());

  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);
}

void AdvanceSearchToolBar::BindSearchAllModel(SearchProxyModel* searchProxyModel, AdvanceSearchModel* searchSourceModel) {
  CHECK_NULLPTR_RETURN_VOID(searchProxyModel);
  CHECK_NULLPTR_RETURN_VOID(searchSourceModel);
  // independant bind
  BindSearchProxyModel(searchProxyModel);
  BindSearchSourceModel(searchSourceModel);

  // initial
  _searchSourceModel->initFilter(m_searchFilterButton->curDirFilters());
  _searchProxyModel->initNameFilterDisables(m_searchFilterButton->curGrayOrHideUnpassItem());
  _searchSourceModel->initIteratorFlag(m_searchFilterButton->curIteratorFlag());

  // subscribe
  connect(m_searchFilterButton, &TypeFilterButton::filterChanged, _searchSourceModel, &AdvanceSearchModel::setFilter);
  connect(m_searchFilterButton, &TypeFilterButton::nameFilterDisablesChanged, _searchProxyModel, &SearchProxyModel::setNameFilterDisables);
  connect(m_searchFilterButton, &TypeFilterButton::includingSubdirectoryChanged, _searchSourceModel, &AdvanceSearchModel::setIteratorFlag);
}

void AdvanceSearchToolBar::BindSearchProxyModel(SearchProxyModel* searchProxyModel) {
  if (searchProxyModel == nullptr) {
    LOG_D("don't try bind nullptr SearchProxyModel*");
    return;
  }
  if (_searchProxyModel != nullptr) {
    LOG_D("don't try rebind SearchProxyModel*");
    return;
  }
  _searchProxyModel = searchProxyModel;
  connect(m_nameFilterCB, &QComboBox::currentTextChanged, this, &AdvanceSearchToolBar::onSearchTextChanges);
  connect(m_nameFilterCB->lineEdit(), &QLineEdit::returnPressed, this, &AdvanceSearchToolBar::onSearchEnterAndApply);
  connect(m_contentCB, &QComboBox::currentTextChanged, this, &AdvanceSearchToolBar::onSearchTextChanges);
  connect(m_contentCB->lineEdit(), &QLineEdit::returnPressed, this, &AdvanceSearchToolBar::onSearchEnterAndApply);
  connect(m_searchModeComboBox, &QComboBox::currentTextChanged, this, &AdvanceSearchToolBar::onSearchModeChanged);

  m_searchModeComboBox->BindSearchModel(_searchProxyModel);
  m_searchCaseButton->BindSearchModel(_searchProxyModel);
}

void AdvanceSearchToolBar::BindSearchSourceModel(AdvanceSearchModel* searchSourceModel) {
  if (searchSourceModel == nullptr) {
    LOG_D("don't try bind nullptr MySearchModel*");
    return;
  }
  if (_searchSourceModel != nullptr) {
    LOG_D("don't try rebind MySearchModel*");
    return;
  }
  _searchSourceModel = searchSourceModel;
}

void AdvanceSearchToolBar::onSearchTextChanges() {
  if (_searchProxyModel == nullptr) {
    return;
  }
  // _searchProxyModel->startFilterWhenTextChanges(m_nameFilterCB->currentText(), m_contentCB->currentText());
}

void AdvanceSearchToolBar::onSearchEnterAndApply() {
  Configuration().setValue(MemoryKey::ADVANCE_SEARCH_LINEEDIT_VALUE.name, m_nameFilterCB->currentText());
  Configuration().setValue(MemoryKey::ADVANCE_SEARCH_CONTENTS_LINEEDIT_VALUE.name, m_contentCB->currentText());
  if (_searchProxyModel == nullptr) {
    return;
  }
  _searchProxyModel->startFilterWhenTextChanged(m_nameFilterCB->currentText(), m_contentCB->currentText());
  _searchProxyModel->PrintRegexDebugMessage();
}

void AdvanceSearchToolBar::onSearchModeChanged(const QString& newSearchModeText) {
  using namespace SearchTools;
  SEARCH_MODE newSearchMode = GetSearchModeEnum(newSearchModeText);
  m_contentCB->setEnabled(newSearchMode == SEARCH_MODE::FILE_CONTENTS);
}
