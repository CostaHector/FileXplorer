#include "FullScreenableSplitter.h"
#include "PublicMacro.h"
#include "MemoryKey.h"
#include <QVBoxLayout>

FullScreenableSplitter::FullScreenableSplitter(const QString& splitterNameSavedInMemory, QWidget* parent)  //
    : FullScreenableSplitter{splitterNameSavedInMemory, Qt::Orientation::Vertical, parent} {
  CHECK_FALSE_RETURN_VOID(!splitterNameSavedInMemory.isEmpty());
}

FullScreenableSplitter::FullScreenableSplitter(const QString& splitterNameSavedInMemory, Qt::Orientation ori, QWidget* parent)
    : QSplitter{ori, parent}, mMemorySplitterStateName{splitterNameSavedInMemory} {
  CHECK_FALSE_RETURN_VOID(!splitterNameSavedInMemory.isEmpty());
}

FullScreenableSplitter::~FullScreenableSplitter() {
  CleanTempFullScreenWindow();
}

bool FullScreenableSplitter::onReqFullscreenModeChange(bool bFullScreen) {
  QWidget* subWidget = GetFullScreenableWidget();
  CHECK_NULLPTR_RETURN_FALSE(subWidget);
  if (bFullScreen) {
    Configuration().setValue(GetMemoryName() + "_STATE", mBeforeFullScreenState = saveState());
    mVideoViewOriginalIndex = indexOf(subWidget);

    mFullScreenWindow = new (std::nothrow) QWidget;
    QVBoxLayout* layout = new (std::nothrow) QVBoxLayout{mFullScreenWindow};
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(subWidget);
    mFullScreenWindow->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
    mFullScreenWindow->setWindowTitle(QString{"[Fullscreen]%1"}.arg(subWidget->windowTitle()));
    mFullScreenWindow->showFullScreen();
    return true;
  }

  if (mFullScreenWindow != nullptr) {
    subWidget->setParent(nullptr);
    insertWidget(mVideoViewOriginalIndex, subWidget);
    restoreState(mBeforeFullScreenState);

    CleanTempFullScreenWindow();
  }
  return true;
}

bool FullScreenableSplitter::saveStateInDerivedDestructor() {
  // don't call me in base class
  if (needSaveStateWhenClose()) {
    Configuration().setValue(GetMemoryName() + "_STATE", saveState());
  }
  return true;
}

bool FullScreenableSplitter::needSaveStateWhenClose() const {
  const auto* fullScreenWid = GetFullScreenableWidget();
  return fullScreenWid != nullptr && fullScreenWid->parent() == this;
}

void FullScreenableSplitter::CleanTempFullScreenWindow() {
  if (mFullScreenWindow != nullptr) {
    mFullScreenWindow->close();
    delete mFullScreenWindow;
    mFullScreenWindow = nullptr;
  }
}
