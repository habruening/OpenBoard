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
};

#endif // CONTROLDESKTOPPALETTE_H
