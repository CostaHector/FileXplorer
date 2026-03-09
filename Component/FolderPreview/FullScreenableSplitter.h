#ifndef FULLSCREENABLESPLITTER_H
#define FULLSCREENABLESPLITTER_H

#include <QSplitter>

class FullScreenableSplitter : public QSplitter {
 public:
  explicit FullScreenableSplitter(const QString& splitterNameSavedInMemory, QWidget* parent = nullptr);
  explicit FullScreenableSplitter(const QString& splitterNameSavedInMemory, Qt::Orientation ori, QWidget* parent = nullptr);
  virtual ~FullScreenableSplitter();
  const QString GetMemoryName() const { return mMemorySplitterStateName; }

 protected:
  bool onReqFullscreenModeChange(bool bFullScreen);
  virtual QWidget* GetFullScreenableWidget() const = 0;
  bool saveStateInDerivedDestructor();

 private:
  bool needSaveStateWhenClose() const;
  void CleanTempFullScreenWindow();

  const QString mMemorySplitterStateName;
  QWidget* mFullScreenWindow{nullptr};
  QByteArray mBeforeFullScreenState;
  int mVideoViewOriginalIndex{0};
};


#endif  // FULLSCREENABLESPLITTER_H
