## Usage

### Create environment for a new laptop
Create file `AKA_PERFORMERS.txt`, `PERFORMERS_TABLE.txt` and folder `runlog` in the `QDir::homePath()`. 
e.g., `QDir::homepath()` can be get by 

```bash
echo "$PWD"
/home/ariel/FileExplorerReadOnly
```

Assure that you are in this path. execute the following commands:
```bash
touch AKA_PERFORMERS.txt
touch PERFORMERS_TABLE.txt
mkdir runlog
```

# properties
## DockWidget recommend initial size


```cpp
QSize sizeHint() const override;
QSize FolderPreviewWidget::sizeHint() const
{
    auto w = PreferenceSettings().value("dockerWidgetWidth", DOCKER_DEFAULT_SIZE.width()).toInt();
    auto h = PreferenceSettings().value("dockerWidgetHeight", DOCKER_DEFAULT_SIZE.height()).toInt();
    return QSize(w, h);
}
```

### isRoot

"C:/" is Root; "C:" is not Root; "" is not Root

```cpp
qDebug() << QDir("C:/").isRoot() << QDir("C:").isRoot();
const auto& itemsInEmptyStringList = QDir("").entryList(QDir::AllEntries|QDir::NoDotAndDotDot);
qDebug() << "item(s) count under [\"\"]" << itemsInEmptyStringList.size() << "is Root" << QDir("").isRoot();
for (const auto& s: itemsInEmptyStringList){
    qDebug() << s;
}
```


```sh
".qmake.stash"
".qtc_clangd"
"debug"
"Makefile"
"Makefile.Debug"
"Makefile.Release"
"object_script.FileExplorerReadOnly.Debug"
"object_script.FileExplorerReadOnly.Release"
"release"
"usersarielappdatalocaltemptmpak6fpk"
"usersarielappdatalocaltemptmpblg1yx"
"usersarielappdatalocaltemptmpdjsdut"
"usersarielappdatalocaltemptmper1v9e"
"usersarielappdatalocaltemptmpgxht4p"
"usersarielappdatalocaltemptmpikucbd"
"usersarielappdatalocaltemptmpnpgs_q"
"usersarielappdatalocaltemptmprmcuqo"
"usersarielappdatalocaltemptmptc2pwk"
```


