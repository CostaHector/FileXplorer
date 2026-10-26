#ifndef IMAGEVIEWERACTIONS_H
#define IMAGEVIEWERACTIONS_H

#include <QObject>
#include <QAction>
#include <QList>

class ImageViewerActions : public QObject {
public:
  static ImageViewerActions& GetInst();

  bool isNavigateIntoSubFoldersWhenIterate() const {
    return NAVIGATE_INTO_SUBFOLDERS_WHEN_ITERATE_ACT != nullptr && NAVIGATE_INTO_SUBFOLDERS_WHEN_ITERATE_ACT->isChecked();
  }
  bool isRecognizeFaceOnSelectionComplete() const {
    return RECOGNIZE_FACE_ON_SELECTION_COMPLETE_ACT != nullptr && RECOGNIZE_FACE_ON_SELECTION_COMPLETE_ACT->isChecked();
  }
  bool isAutoSaveOnSelectionComplete() const {
    return AUTO_SAVE_ON_SELECTION_COMPLETE_ACT != nullptr && AUTO_SAVE_ON_SELECTION_COMPLETE_ACT->isChecked();
  }
  bool isSilentSaveOnAutoSave() const {
    return SILENT_SAVE_ON_AUTO_SAVE_ACT != nullptr && SILENT_SAVE_ON_AUTO_SAVE_ACT->isChecked();
  }
  QAction* NAVIGATE_INTO_SUBFOLDERS_WHEN_ITERATE_ACT{nullptr};
  QAction* RECOGNIZE_FACE_ON_SELECTION_COMPLETE_ACT{nullptr};
  QAction* AUTO_SAVE_ON_SELECTION_COMPLETE_ACT{nullptr};
  QAction* SILENT_SAVE_ON_AUTO_SAVE_ACT{nullptr};

private:
  explicit ImageViewerActions(QObject* parent = nullptr);

  void subscribe();
};

#endif // IMAGEVIEWERACTIONS_H
