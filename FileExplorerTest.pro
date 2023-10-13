QT += testlib
QT += gui
CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app


SOURCES += TestCase/SomeTestTemplate.cpp \
    FileOperation/FileOperation.cpp \
    TestCase/FileOperationTest.cpp

HEADERS += \
    FileOperation/FileOperation.h

DISTFILES += \
    FileOperation/test/FileOperationTestEnv/DONT_CHANGE/a.txt \
    FileOperation/test/FileOperationTestEnv/DONT_CHANGE/a/a1.txt \
    FileOperation/test/FileOperationTestEnv/DONT_CHANGE/a/a1/a2.txt \
    FileOperation/test/FileOperationTestEnv/DONT_CHANGE/a/a1/a2/a3.txt \
    FileOperation/test/FileOperationTestEnv/DONT_CHANGE/b.txt \
    FileOperation/test/FileOperationTestEnv/DONT_CHANGE/b/b1.txt \
    FileOperation/test/FileOperationTestEnv/DONT_CHANGE/b/b1/b2.txt \
    FileOperation/test/FilesNameBatchStandardizerTestEnv/DONT_CHANGE - Copy/- C.txt \
    FileOperation/test/FilesNameBatchStandardizerTestEnv/DONT_CHANGE - Copy/A B/C - A/C - A.txt \
    FileOperation/test/FilesNameBatchStandardizerTestEnv/DONT_CHANGE - Copy/A.txt \
    FileOperation/test/FilesNameBatchStandardizerTestEnv/DONT_CHANGE/---C.txt \
    FileOperation/test/FilesNameBatchStandardizerTestEnv/DONT_CHANGE/A       B/C [A/C (A.txt \
    FileOperation/test/FilesNameBatchStandardizerTestEnv/DONT_CHANGE/A - .txt \
    FileOperation/test/TestEnv_FileFolderMover/DONT_CHANGE/FOLDER_SRC/SubFolder/SubFolderFile.txt \
    FileOperation/test/TestEnv_FileFolderMover/DONT_CHANGE/FOLDER_SRC2/file2.txt \
    FileOperation/test/TestEnv_FolderMerger/CHANGEABLE/MixedFileFolderConflicts/XXLFolder - MovieName - 1999/4.jpg \
    FileOperation/test/TestEnv_FolderMerger/CHANGEABLE/MixedFileFolderConflicts/XXLFolder - MovieName - 1999/5.json \
    FileOperation/test/TestEnv_FolderMerger/CHANGEABLE/MixedFileFolderConflicts/XXLFolder - MovieName - 1999/Performer/4.jpg \
    FileOperation/test/TestEnv_FolderMerger/CHANGEABLE/MixedFileFolderConflicts/XXLFolder - MovieName - 1999/Performer/all/1.jpg \
    FileOperation/test/TestEnv_FolderMerger/CHANGEABLE/MixedFileFolderConflicts/XXLFolder - MovieName - 1999/Performer/all/2.jpg \
    FileOperation/test/TestEnv_FolderMerger/CHANGEABLE/MixedFileFolderConflicts/XXLFolder - MovieName - 1999/Performer/all/4.jpg \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/MixedFileFolderConflicts/MovieName/5.json \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/MixedFileFolderConflicts/MovieName/Performer/4.jpg \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/MixedFileFolderConflicts/MovieName/Performer/all/1.jpg \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/MixedFileFolderConflicts/MovieName/Performer/all/4.jpg \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/MixedFileFolderConflicts/XXLFolder - MovieName - 1999/4.jpg \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/MixedFileFolderConflicts/XXLFolder - MovieName - 1999/5.json \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/MixedFileFolderConflicts/XXLFolder - MovieName - 1999/Performer/all/1.jpg \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/MixedFileFolderConflicts/XXLFolder - MovieName - 1999/Performer/all/2.jpg \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/MixedFileFolderConflicts_rightSide/MovieName/5.json \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/MixedFileFolderConflicts_rightSide/MovieName/Performer/4.jpg \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/MixedFileFolderConflicts_rightSide/MovieName/Performer/all/1.jpg \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/MixedFileFolderConflicts_rightSide/MovieName/Performer/all/4.jpg \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/MixedFileFolderConflicts_rightSide/XXLFolder - MovieName - 1999/5.json \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/MixedFileFolderConflicts_rightSide/XXLFolder - MovieName - 1999/Performer/4.jpg \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/MixedFileFolderConflicts_rightSide/XXLFolder - MovieName - 1999/Performer/all/1.jpg \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/MixedFileFolderConflicts_rightSide/XXLFolder - MovieName - 1999/Performer/all/2.jpg \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/OneFileConflict/MovieName/1 720.json \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/OneFileConflict/MovieName/1 720.mp4 \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/OneFileConflict/MovieName/1.jpg \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/OneFileConflict/XXLFolder - MovieName - 1999/1.jpg \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/OneFileConflict/XXLFolder - MovieName - 1999/1.json \
    FileOperation/test/TestEnv_FolderMerger/DONT_CHANGE/OneFileConflict/XXLFolder - MovieName - 1999/1.mp4 \
    FileOperation/test/TestEnv_MyQFileSystemModel/DONT_CHANGE/DONT_CHANGE.json \
    FileOperation/test/TestEnv_MyQFileSystemModel/Microsoft Edge.lnk \
    FileOperation/test/TestEnv_NameStringNumerizer/DONT_CHANGE/A.txt \
    FileOperation/test/TestEnv_NameStringNumerizer/DONT_CHANGE/B.txt \
    FileOperation/test/TestEnv_NameStringNumerizer/DONT_CHANGE/C.txt \
    FileOperation/test/TestEnv_NameStringNumerizer/DONT_CHANGE/D.png \
    FileOperation/test/TestEnv_NameStringReplacer/DONT_CHANGE/a single file BB.txt \
    FileOperation/test/TestEnv_NameStringReplacer/DONT_CHANGE/folder BB/direct file BB.txt \
    FileOperation/test/TestEnv_NameStringReplacer/DONT_CHANGE/snippingtool PNG figure.PNG \
    FileOperation/test/TestEnv_TextMerger/DONT_CHANGE_MultiTextFile/file1.txt \
    FileOperation/test/TestEnv_TextMerger/DONT_CHANGE_MultiTextFile/file2.txt \
    FileOperation/test/TestEnv_TextMerger/DONT_CHANGE_MultiTextFile/file3.txt \
    FileOperation/test/TestEnv_TextMerger/test_output/out.txt \
    FileOperation/test/TextEnv_NameStringCase/DONT_CHANGE/FC club, CR7, kaka, USB Wi-Fi.txt \
    FileOperation/test/TextEnv_NameStringCase/DONT_CHANGE/top/NBA is.txt \
    FileOperation/test/TextEnv_NameStringSectionSwapper/DONT_CHANGE/A-B-C.txt \
    FileOperation/test/TextEnv_NameStringSectionSwapper/DONT_CHANGE/A/A - B - C - D - 0.txt \
    FileOperation/test/TextEnv_NameStringSectionSwapper/DONT_CHANGE/A/A - B - C - D 1.txt \
    FileOperation/test/TextEnv_NameStringSectionSwapper/DONT_CHANGE/A/A.txt
