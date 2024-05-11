#include "FolderPreviewSwitcher.h"
#include "Actions/FolderPreviewActions.h"

FolderPreviewSwitcher::FolderPreviewSwitcher(PreviewFolder* folderPreview, QWidget* parentDocker, QObject* parent)
    : QObject{parent}, _folderPreview{folderPreview}, _parentDocker{parentDocker} {
  auto* checkedPreviewType = g_folderPreviewActions().PREVIEW_AG->checkedAction();
  onSwitchByViewAction(checkedPreviewType);
}

void FolderPreviewSwitcher::onSwitchByViewType(const QString& viewType) {
  if (_folderPreview == nullptr) {
    qWarning("_folderPreview cannot be empty");
    return;
  }
  if (viewType == g_folderPreviewActions().LISTS->text()) {
    if (_folderPreview->m_lists == nullptr) {
      _folderPreview->m_lists = new PreviewLists{_parentDocker};
      _folderPreview->AddView(viewType, _folderPreview->m_lists);
    }
    _folderPreview->m_lists->operator()(_folderPreview->GetCurPath());
  } else if (viewType == g_folderPreviewActions().BROWSER->text()) {
    if (_folderPreview->m_browser == nullptr) {
      _folderPreview->m_browser = new PreviewBrowser{_parentDocker};
      _folderPreview->AddView(viewType, _folderPreview->m_browser);
    }
    _folderPreview->m_browser->operator()(_folderPreview->GetCurPath());
  } else if (viewType == g_folderPreviewActions().LABELS->text()) {
    if (_folderPreview->m_labels == nullptr) {
      _folderPreview->m_labels = new PreviewLabels{_parentDocker};
      _folderPreview->AddView(viewType, _folderPreview->m_labels);
    }
    _folderPreview->m_labels->operator()(_folderPreview->GetCurPath());
  } else {
    qWarning("previewType[%s] not support now.", qPrintable(viewType));
    return;
  }
  int viewIndex = _folderPreview->m_name2PreviewIndex[viewType];
  _folderPreview->setCurrentIndex(viewIndex);
}

void FolderPreviewSwitcher::onSwitchByViewAction(const QAction* activatedAction) {
  const QString& viewType = activatedAction->text();
  onSwitchByViewType(viewType);
}
