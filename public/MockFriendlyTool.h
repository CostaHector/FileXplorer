#ifndef MOCKFRIENDLYTOOL_H
#define MOCKFRIENDLYTOOL_H

#include <QtCore>
namespace MockFriendlyTool {
std::pair<int, bool> getInt(QWidget* parent,
                            const QString& title,
                            const QString& label,
                            int value = 0,
                            int minValue = -2147483647,
                            int maxValue = 2147483647,
                            int step = 1,
                            bool* ok = nullptr,
                            Qt::WindowFlags flags = Qt::WindowFlags());
}

#endif  // MOCKFRIENDLYTOOL_H
