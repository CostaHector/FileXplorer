#include "FolderPreviewSwitcher.h"
#include "FolderPreviewActions.h"
#include "PublicMacro.h"

FolderPreviewSwitcher::FolderPreviewSwitcher(SelectionPreviewer* folderPreview, QObject* parent)//
  : QObject{parent}, _folderPreview{folderPreview} //
{
  auto* checkedPreviewType = g_folderPreviewActions().PREVIEW_AG->checkedAction();
  onSwitchByViewAction(checkedPreviewType);
}

void FolderPreviewSwitcher::onSwitchByViewType(const QString& viewType) {
  CHECK_NULLPTR_RETURN_VOID(_folderPreview)
  if (viewType == g_folderPreviewActions().LISTS->text()) {
    if (_folderPreview->m_fileFolderPreviewStackedWid == nullptr) {
      _folderPreview->m_fileFolderPreviewStackedWid = new (std::nothrow) FileFolderPreviewer{"DockerList", _folderPreview};
      CHECK_NULLPTR_RETURN_VOID(_folderPreview->m_fileFolderPreviewStackedWid)
      _folderPreview->AddView(viewType, _folderPreview->m_fileFolderPreviewStackedWid);
    }
    _folderPreview->m_fileFolderPreviewStackedWid->operator()(_folderPreview->GetCurPath());
  } else if (viewType == g_folderPreviewActions().BROWSER->text()) {
    if (_folderPreview->m_imgInFolderBrowser == nullptr) {
      _folderPreview->m_imgInFolderBrowser = new (std::nothrow) ImagesInFolderBrowser{_folderPreview};
      CHECK_NULLPTR_RETURN_VOID(_folderPreview->m_imgInFolderBrowser)
      _folderPreview->AddView(viewType, _folderPreview->m_imgInFolderBrowser);
    }
    _folderPreview->m_imgInFolderBrowser->operator()(_folderPreview->GetCurPath());
  } else if (viewType == g_folderPreviewActions().LABELS->text()) {
    if (_folderPreview->m_imgInFolderLabels == nullptr) {
      _folderPreview->m_imgInFolderLabels = new (std::nothrow) ImagesInFolderSlider{_folderPreview};
      CHECK_NULLPTR_RETURN_VOID(_folderPreview->m_imgInFolderLabels)
      _folderPreview->AddView(viewType, _folderPreview->m_imgInFolderLabels);
    }
    _folderPreview->m_imgInFolderLabels->operator()(_folderPreview->GetCurPath());
  } else {
    qWarning("previewType[%s] not support now.", qPrintable(viewType));
    return;
  }
  int viewIndex = _folderPreview->m_name2PreviewIndex[viewType];
  _folderPreview->setCurrentIndex(viewIndex);
}

void FolderPreviewSwitcher::onSwitchByViewAction(const QAction* activatedAction) {
  if (activatedAction == nullptr) {
    qDebug("Nothing preview type selected, skip preview");
    return;
  }
  const QString& viewType = activatedAction->text();
  onSwitchByViewType(viewType);
}
