/*
 * Maintainer: Peng Hui<penghui@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QObject>

#include "main_window.h"
//#include "eventcontainer.h"

class Screenshot : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "com.deepin.ScreenRecorder")
public:
    Screenshot(QObject *parent = nullptr);
    ~Screenshot();

    void setConfigThemeType(int themeType);
public slots:
    void startScreenshot();
    void delayScreenshot(double num);
    void fullscreenScreenshot();
    void topWindowScreenshot();
    void noNotifyScreenshot();
    void savePathScreenshot(const QString &path);
    void initLaunchMode(const QString &launchmode);
    Q_SCRIPTABLE void stopRecord();
    Q_SCRIPTABLE void waylandRecordOver();

private:
    void initUI();

//    EventContainer *m_eventContainer = nullptr;
    QString m_launchMode;
    MainWindow *m_window = nullptr;

};

#endif // SCREENSHOT_H
