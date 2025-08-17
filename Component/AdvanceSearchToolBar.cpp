#include "AdvanceSearchToolBar.h"
#include "PublicMacro.h"
#include "PublicVariable.h"
#include "MemoryKey.h"
#include <QLayout>

AdvanceSearchToolBar::AdvanceSearchToolBar(const QString& title, QWidget* parent)  //
    : QToolBar{title, parent}                                                      //
{
  m_nameFilter = new (std::nothrow) QLineEdit{""};
  CHECK_NULLPTR_RETURN_VOID(m_nameFilter)
  m_nameFilter->addAction(QIcon(":img/SEARCH"), QLineEdit::LeadingPosition);
  m_nameFilter->setClearButtonEnabled(true);
  m_nameFilter->setPlaceholderText("Search file names here");

  m_nameFilterCB = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_nameFilterCB)
  m_nameFilterCB->setInsertPolicy(QComboBox::InsertPolicy::InsertAtTop);
  m_nameFilterCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  m_nameFilterCB->setLineEdit(m_nameFilter);
  m_nameFilterCB->addItem(PreferenceSettings().value(MemoryKey::ADVANCE_SEARCH_LINEEDIT_VALUE.name, MemoryKey::ADVANCE_SEARCH_LINEEDIT_VALUE.v).toString());
  m_nameFilterCB->addItem("\\.xltd$");
  m_nameFilterCB->addItem("\\.torrent$");
  m_nameFilterCB->addItem("\\.!ut$");
  m_nameFilterCB->addItem("\\.html$");
  m_nameFilterCB->addItem("\\.txt$");
  m_nameFilterCB->addItem("\\.json$");
  m_nameFilterCB->addItem("\\.scn$");
  m_nameFilterCB->addItem("\\.md$");
  m_nameFilterCB->addItem("\\.pjson$");

  m_typeFilterButton = new FileSystemTypeFilter{this};
  CHECK_NULLPTR_RETURN_VOID(m_typeFilterButton)
  m_typeFilterButton->setToolTip("Filter file types by extension");

  m_searchModeComboBox = new SearchModeComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_searchModeComboBox)
  m_searchModeComboBox->setToolTip("Search Mode:\n1. full match\n2. regex\n3. regex+file name");

  m_searchCaseButton = new SearchCaseMatterToolButton{this};
  CHECK_NULLPTR_RETURN_VOID(m_searchCaseButton)
  m_searchCaseButton->setToolTip("Match Case Switch");

  m_contentCB = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_contentCB)
  m_contentCB->setEditable(true);
  m_contentCB->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
  m_contentCB->setToolTip("Search plain text file contents here");
  m_contentCB->addItem("nonporn");
  m_contentCB->addItem(PreferenceSettings().value(MemoryKey::ADVANCE_SEARCH_CONTENTS_LINEEDIT_VALUE.name, MemoryKey::ADVANCE_SEARCH_CONTENTS_LINEEDIT_VALUE.v).toString());

  addWidget(m_nameFilterCB);
  addWidget(m_typeFilterButton);
  addSeparator();
  addWidget(m_searchModeComboBox);
  addWidget(m_searchCaseButton);
  addWidget(m_contentCB);
  onSearchModeChanged(m_searchModeComboBox->currentText());

  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);
}

void AdvanceSearchToolBar::BindSearchAllModel(SearchProxyModel* searchProxyModel, AdvanceSearchModel* searchSourceModel) {
  // independant bind
  BindSearchProxyModel(searchProxyModel);
  BindSearchSourceModel(searchSourceModel);
  // interative bind
  m_typeFilterButton->BindFileSystemModel(_searchSourceModel, _searchProxyModel);
}

void AdvanceSearchToolBar::BindSearchProxyModel(SearchProxyModel* searchProxyModel) {
  if (searchProxyModel == nullptr) {
    qDebug("don't try bind nullptr SearchProxyModel*");
    return;
  }
  if (_searchProxyModel != nullptr) {
    qDebug("don't try rebind SearchProxyModel*");
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
    qDebug("don't try bind nullptr MySearchModel*");
    return;
  }
  if (_searchSourceModel != nullptr) {
    qDebug("don't try rebind MySearchModel*");
    return;
  }
  _searchSourceModel = searchSourceModel;
}

void AdvanceSearchToolBar::onSearchTextChanges() {
  if (_searchProxyModel == nullptr) {
    return;
  }
  _searchProxyModel->startFilterWhenTextChanges(m_nameFilterCB->currentText(), m_contentCB->currentText());
}

void AdvanceSearchToolBar::onSearchEnterAndApply() {
  PreferenceSettings().setValue(MemoryKey::ADVANCE_SEARCH_LINEEDIT_VALUE.name, m_nameFilterCB->currentText());
  PreferenceSettings().setValue(MemoryKey::ADVANCE_SEARCH_CONTENTS_LINEEDIT_VALUE.name, m_contentCB->currentText());
  if (_searchProxyModel == nullptr) {
    return;
  }
  _searchProxyModel->startFilterWhenTextChanged(m_nameFilterCB->currentText(), m_contentCB->currentText());
}

void AdvanceSearchToolBar::onSearchModeChanged(const QString& newSearchModeText) {
  using namespace SearchTools;
  SEARCH_MODE newSearchMode = GetSearchModeEnum(newSearchModeText);
  m_contentCB->setEnabled(newSearchMode == SEARCH_MODE::FILE_CONTENTS);
}
