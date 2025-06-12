# FileXplorer - Ultimate File Management Suite

## Overview

FileXplorer is a cross-platform professional file management system engineered for photographers and videographers handling large-scale media files, offering advanced file operations across both Windows and Linux environments.

![FileXplorer](bin/FileXplorer_Dark.png)

## Core Features

1. File/Folder Navigation

        - Dual-pane explorer interface
        - Thumbnail/preview for 100+ file formats
        - Dark/light theme support

2. Advanced Batch Renaming

        - Plain Text Insert/replace/Delete
        - Regex String Insert/replace/Delete
        - Case operations: UPPER/lower/Title/toggleCase
        - Segment swapping (e.g., "Marvil - S01E02 - 2012" → "Marvil - 2012 - S01E02")
        - Auto-numbering with customizable templates

3. Professional Tools

        - Media Management:
        - Duplicate image detection
        - Batch video duration viewer
        - Media metadata to database export
        - Cast and their films database by media database export 

4. Developer Features:

        - HAR file previewer
        - JSON batch editor (build media databases)
        - MD5 checksum comparator

4. Smart Operations

        - F1 quick-search across files
        - One-click file organization by patterns
        - Multi-path synchronization

5. Content-aware search

        - Search by filename
        - Search by file content
        - Search by filename + file content

6. File/Folder Operation Recoverable

        - Each operation will be saved in memory. Operation can be undo(recovered) by shortcut `Ctrl+Z` and Redo by shortcut `Ctrl+Y`.

        Attention:
                - All undo history purges upon application termination/restart. So operation cannot be recovered after restart;
                - Permanent deletion operations cannot be recovered;

7. Dark/Light theme switch support
![FileXplorer](bin/FileXplorer_Light.png)



## Coding Style

variable of class data memeber:

1. class itself has ownership, start with "m_" or start with "m";
2. only transfer, no ownership, start with "m_" or start with "m" and endwith "_";

## Devoloping Settings
we suggest you to do following setting in git bash
```md
git update-index --assume-unchange FileXplorer.pro.user
git update-index --assume-unchange FileExplorerTest.pro.user
git update-index --assume-unchange bin/logs_info.log
git update-index --assume-unchange bin/AKA_PERFORMERS.txt
git update-index --assume-unchange bin/PERFORMERS_TABLE.txt
git update-index --assume-unchange bin/STANDARD_STUDIO_NAME.txt

git update-index --no-assume-unchange FileXplorer.pro.user
git update-index --no-assume-unchange FileExplorerTest.pro.user
git update-index --no-assume-unchange bin/logs_info.log
```

Edit right click menu in windows register edit:
Computer\HKEY_CLASSES_ROOT\Directory\Background\Shell\FileExplorerDesktop\command
Modify and set Value data to
"C:\home\Ariel\qtcpp\FileXplorer\build\Desktop_Qt_5_15_2_MinGW_64_bit-Release\release\FileExplorerReadOnly.exe" "%V"

"C:\home\Ariel\qtcpp\build-FileExplorerReadOnly-Desktop_Qt_5_15_2_MinGW_64_bit-release\release\FileXplorer.exe" "%V"


```md
git filter-branch --force --index-filter   "git rm --cached --ignore-unmatch bin/AKA_PERFORMERS.txt"   --prune-empty --tag-name-filter cat -- --all  
rm -rf .git/refs/original/
git reflog expire --expire=now --all
git gc --prune=now --aggressive

git remote set-url git@github.com:CostaHector/FileXplorer.git
git remote -v
git remote remove origin
git remote add origin git@github.com:CostaHector/FileXplorer.git
git push -u origin fileXplor:master -f
git remote add origin git@github.com:CostaHector/FileExplorerReadOnly.git
```

## New Feature
1. Logs Control

### Log Control
Interactive function:
1. Open latest log file
2. Open logs folder
3. Set log level(default: error), Attention:
    - This log level only control release edition.
4. Aging log file if size >= 20MiB

a log line example:
> `hh:mm:ss.zzz E functionName msg [fileName:fileNo]`



## How QTextEdit Show image from ByteArray

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

## Testcase
### Table 1.0 Expected Behavior of rename Functions
```cpp
RETURN_TYPE rename(const QString& srcPath, const QString& oldCompleteName, const QString& newCompleteName)
```
| srcPath | oldCompleteName | newCompleteName | exist items in srcPath | result |
|---------|-----------------|-----------------|---------------------------------------------------|--------|
| home | a | A | {a} | OK |
| home | a | A | {a, A} | In windows platform no need consider this one as system may prevent two items(Only differ in case) place/create/moved in one folder;<br/>Linux return DST_FILE_OR_PATH_ALREADY_EXIST |
| home | a | a | {a} | SKIP |
| home | a | b | {a} | OK |
| home | a | b | {a,b} | windows/linux return DST_FILE_OR_PATH_ALREADY_EXIST|
| home | a | B | {a,B} | windows/linux return DST_FILE_OR_PATH_ALREADY_EXIST|
| home | a | b | {a,B} | windows return DST_FILE_OR_PATH_ALREADY_EXIST;<br/>Linux return OK|
| home | a | B | {a,b} | windows return DST_FILE_OR_PATH_ALREADY_EXIST;<br/>Linux return OK|

### Table 1.1 Expected Behavior of mv Functions
```cpp
RETURN_TYPE mv(const QString& srcPath, const QString& relToItem, const QString& dstPath)`
```
| srcPath | relToItem | dstPath | exist items in dstPath | result |
|---------|-----------|---------|---------------------------------------------------|--------|
| home | any1 | home | {any1} | OK, SKIP |
| home | any1 | HOME | {any1} | OK, SKIP. It is not recommend to create two folder only differ in name case in Linux platform. |
| home | a | bin | {} | OK |
| home | a | bin | {a} | windows/linux return DST_FILE_OR_PATH_ALREADY_EXIST |
| home | a | bin | {A} | windows return DST_FILE_OR_PATH_ALREADY_EXIST;<br/>linux return OK |
| home | path/to/a | bin | {} | OK |
| home | path/to/a | bin | {path/to} | OK |
| home | path/to/a | bin | {path/to/a} | windows/linux return DST_FILE_OR_PATH_ALREADY_EXIST |
| home | path/to/a.txt | bin | {} | OK |

### Table 1.2 rmpath Behavior of mv Functions
```cpp
RETURN_TYPE rmpath(const QString& pre, const QString& dirPath)
```
| srcPath | relToItem | exist items in srcPath | result |
|---------|-----------|---------------------------------------------------|--------|
| home | a/a1 | {a/a1, a/a1/a2.txt} | CANNOT_REMOVE_DIR |
| home | a/a1 | {a/a1} | OK |
| home | a | {a/a.txt} | CANNOT_REMOVE_DIR |
| home | a | {a} | OK |

### Table 1.3 mkpath Behavior of mv Functions
```cpp
RETURN_TYPE mkpath(const QString& pre, const QString& dirPath)
```
| srcPath | relToItem | exist items in srcPath | result |
|---------|-----------|---------------------------------------------------|--------|
| home | a/a1 | srcPath not exist | DST_DIR_INEXIST |
| home | a/a1 | {} | OK |
| home | a/a1 | {a} | OK |
| home | a/a1 | {a/a1} | OK |
| home | a | {} | OK |