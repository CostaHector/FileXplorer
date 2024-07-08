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


## 2. How QTextEdit Show image from ByteArray

```cpp
if (not qzPath.toLower().endsWith(".qz")) {
  qWarning("Not a qz file");
  setWindowTitle(QString("ArchiveFilesPreview | [%1] not a qz file").arg(qzPath));
  return false;
}

ArchiveFiles af(qzPath, ArchiveFiles::ONLY_IMAGE);
static constexpr int K = 4;
// Todo, loading more button
QStringList paths;
QList<QByteArray> datas;
paths.reserve(K);
datas.reserve(K);

if (not af.ReadFirstKItemsOut(K, paths, datas)) {
  setWindowTitle(QString("ArchiveFilesPreview | [%1] read failed").arg(qzPath));
  return false;
}
setWindowTitle(QString("ArchiveFilesPreview | %1 item(s)").arg(paths.size()));

QTextDocument* textDocument = document();
for (int i = 0; i < paths.size(); ++i) {
  QUrl url{paths[i]};
  textDocument->addResource(QTextDocument::ImageResource, url, QVariant(datas[i]));
  QTextImageFormat imageFormat;
  imageFormat.setName(url.toString());

  QTextCursor cursor = textCursor();
  cursor.insertText(url.toString());
  cursor.insertText("\n");
  cursor.insertImage(imageFormat);
  cursor.insertText("\n");
}
```

## 需求文档:
### 文件整理功能增强:
#### 要求:

1.1 视频文件
1. 模式类似`name scene \d.vid`

    -- 期望归入文件夹`name scene \d`
2. 其他`name.vid`

    -- 期望归入文件夹`name`

1.2 图片文件
1. 模式类似`name scene \d \d.img`

    -- 期望归入文件夹`name scene \d`
2. 模式类似`name \d.img`

    -- 期望归入文件夹`name`

须知!
Notice!











