#ifndef CONTROLDESKTOPPALETTE_H
#define CONTROLDESKTOPPALETTE_H

#include <QObject>

#include "gui/ActionPalette.h"
#include "frameworks/UBPlatformUtils.h"


class ControlDesktopPalette : QObject
{
    Q_OBJECT

    public:
        ControlDesktopPalette();
        ActionPalette mNewDesktopPalette;
        void show();

    signals:
        void closeDesktopMode();
        void selectPen();
        void selectEraser();
        void selectMarker();
        void selectCursor();
        void selectLaser();
        void toggleVirtualKeyboard();
        void takeScreenshotRectangle();
        void takeFullScreenshot();
        void lookIntoOtherScreen(bool eyes_open);
};

#endif // CONTROLDESKTOPPALETTE_H
