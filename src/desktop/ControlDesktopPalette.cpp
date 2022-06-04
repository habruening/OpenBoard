#include "ControlDesktopPalette.h"

#include <QIcon>
#include <QAction>

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "gui/UBMainWindow.h"

ControlDesktopPalette::ControlDesktopPalette()
    : mNewDesktopPalette(42)
{

    mNewDesktopPalette.createAndAddAction(QIcon(":/images/toolbar/board.png"), tr("Show OpenBoard"));
    mNewDesktopPalette.addAction(UBApplication::mainWindow->actionPen);
    mNewDesktopPalette.addAction(UBApplication::mainWindow->actionEraser);
    mNewDesktopPalette.addAction(UBApplication::mainWindow->actionMarker);
    mNewDesktopPalette.addAction(UBApplication::mainWindow->actionSelector);
    mNewDesktopPalette.addAction(UBApplication::mainWindow->actionPointer);

    if (UBPlatformUtils::hasVirtualKeyboard())
        mNewDesktopPalette.addAction(UBApplication::mainWindow->actionVirtualKeyboard);

    mNewDesktopPalette.createAndAddAction(QIcon(":/images/toolbar/captureArea.png"), tr("Capture Part of the Screen"));

    mNewDesktopPalette.createAndAddAction(QIcon(":/images/toolbar/captureScreen.png"), tr("Capture the Screen"));

    QIcon showHideIcon;
    showHideIcon.addPixmap(QPixmap(":/images/toolbar/eyeOpened.png"), QIcon::Normal , QIcon::On);
    showHideIcon.addPixmap(QPixmap(":/images/toolbar/eyeClosed.png"), QIcon::Normal , QIcon::Off);
    QAction* mShowHideAction = mNewDesktopPalette.createAndAddAction(showHideIcon, tr("Show Other Screen"));
    mShowHideAction->setCheckable(true);

}

void ControlDesktopPalette::show(){
    mNewDesktopPalette.show();
}
