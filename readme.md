# FileExplorerReadyOnly
## Introduction

An File Explorer that can be used in windows and linux.

## Component

### ContentPanel
1 Table/Tree/List View:
1. abstract view double clicked


onActionAndViewNavigate
onActionAndViewNavigate

### NavigationAndAddressBar

3 Actions and 1 LineEdit:
1. UpTo
2. BackTo
3. ForwardTo
4. Search LineEdit Text changed
5. Search LineEdit Return Pressed


#### AddressELineEdit

1 Action and 1 LineEdit:
1. path action
2. return pressed

1. CustomTableView + QAbstractTableModel + QSortProxyModel + AdvanceSearchToolBar => SearchPane
#? merge QAbstractTableModel into QSortProxyModel? and discard the former
        adv: no more mapToSource, mapSelectionToSource
# ?? use or not Signal of QSortProxyModel filterCaseSensitivityChanged connect to startfilter
        a. when switch view to "search" 
                i. QAbstractTableModel 
                        1) setRootPathAndFilter(path, typeFilter)
                        2) call QDirIterator it(path, typeFilter, iteratorFlag)
        b. when searchLineEdit text changed
                i. QSortProxyModel
                        1) --- change m_text
                        2) --- startFilter(m_text and m_searchMode)
                                a) setFilterFixedString
                                b) setFilterRegularExpression
                                c) setFilterWildcard
                                d) setFilterCustomNameAndContent
        c. when typeToolButton nameFilterDisables triggered
                i. QSortProxyModel 
                        1) m_isNameFilterDisable
                        2) --- PostReturn() 
                                a) QAbstractTableModel
                                        i) m_disablesList.append
        d. when searchModeComboBox mode changed
                i. QSortProxyModel
                        1) --- change m_searchMode
                        2) --- startFilter(m_text and m_searchMode)
        e. searchCaseToolButton contents/name --- filterAcceptRow
                i. QSortProxyModel
                        1) --- change m_nameCaseSensitive, m_contentCaseSensitive
                        2) --- setIgnoreCase
                        3) --- startFilter(m_text and m_searchMode)
        f. GetSelection
                i. CustomTableView
                        1) selectionIndexes: mapToSource, mapSelectionToSource
2. CustomTableView + QSqlModel => PerformersPane, TorrPane
3. CustomTableView + QFileSystemModel => FileSystemPane
4. ListView + QFileSystemModel =>  FileSystemPane
5. TreeView + QFileSystemModel  => FileSystemPane

Extract AdvanceSearchToolBar out
level0: name/contentsLineEdit e.g., QSqlTableModel
level1: typeFilterToolButton FileSystemModel
level2: searchModeComboBox, CaseSensitiveToolButton, searchColumnComboBox, e.g., SortFilterMode


# Install Suggestion

## 1. fsearch

Install in app store:

https://github.com/cboxdoerfer/fsearch/wiki/Snap-is-no-longer-officially-supported
