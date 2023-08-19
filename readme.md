# properties
## DockWidget recommend initial size


```cpp
QSize sizeHint() const override;
QSize FolderPreviewWidget::sizeHint() const
{
    auto w = PreferenceSettings().value("mainWindowWidth", DOCKER_DEFAULT_SIZE.width()).toInt();
    auto h = PreferenceSettings().value("mainWindowHeight", DOCKER_DEFAULT_SIZE.height()).toInt();
    return QSize(w, h);
}
```
