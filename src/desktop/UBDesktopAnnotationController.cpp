/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#include "UBDesktopAnnotationController.h"

#include "frameworks/UBPlatformUtils.h"

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBDisplayManager.h"
#include "core/UBSettings.h"

#include "web/UBWebController.h"

#include "gui/UBMainWindow.h"

#include "board/UBBoardView.h"
#include "board/UBDrawingController.h"
#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"

#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsPolygonItem.h"

#include "UBCustomCaptureWindow.h"
#include "UBDesktopPalette.h"
#include "UBDesktopPropertyPalette.h"

#include "gui/UBKeyboardPalette.h"
#include "gui/UBResources.h"

#include "core/memcheck.h"

UBDesktopAnnotationController::UBDesktopAnnotationController(QObject *parent, UBRightPalette* rightPalette)
        : QObject(parent)
        , mTransparentDrawingView(0)
        , mTransparentDrawingScene(0)
        , mDesktopPalette(NULL)
        , mDesktopPenPalette(NULL)
        , mDesktopMarkerPalette(NULL)
        , mDesktopEraserPalette(NULL)
        , mRightPalette(rightPalette)
        , mWindowPositionInitialized(false)
        , mIsFullyTransparent(false)
        , mDesktopToolsPalettePositioned(false)
        , mBoardStylusTool(UBDrawingController::drawingController()->stylusTool())
        , mDesktopStylusTool(UBDrawingController::drawingController()->stylusTool())
{

    mTransparentDrawingView = new UBBoardView(UBApplication::boardController, static_cast<QWidget*>(0), false, true); // deleted in UBDesktopAnnotationController::destructor
    mTransparentDrawingView->setAttribute(Qt::WA_TranslucentBackground, true);
#ifdef Q_OS_OSX
    mTransparentDrawingView->setAttribute(Qt::WA_MacNoShadow, true);
#endif //Q_OS_OSX

    mTransparentDrawingView->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Window | Qt::NoDropShadowWindowHint | Qt::X11BypassWindowManagerHint);
    mTransparentDrawingView->setCacheMode(QGraphicsView::CacheNone);
    mTransparentDrawingView->resize(UBApplication::displayManager->screenSize(ScreenRole::Desktop));

    mTransparentDrawingView->setMouseTracking(true);

    mTransparentDrawingView->setAcceptDrops(true);

    QString backgroundStyle = "QWidget {background-color: rgba(127, 127, 127, 0)}";
    mTransparentDrawingView->setStyleSheet(backgroundStyle);

    mTransparentDrawingScene = new UBGraphicsScene(0, false);
    updateColors();

    mTransparentDrawingView->setScene(mTransparentDrawingScene);
    mTransparentDrawingScene->setDrawingMode(true);

    mDesktopPalette = new UBDesktopPalette(mTransparentDrawingView, rightPalette); 
    // This was not fix, parent reverted
    // FIX #633: The palette must be 'floating' in order to stay on top of the library palette

    if (UBPlatformUtils::hasVirtualKeyboard())
    {
        connect( UBApplication::boardController->paletteManager()->mKeyboardPalette, SIGNAL(keyboardActivated(bool)), 
                 mTransparentDrawingView, SLOT(virtualKeyboardActivated(bool)));
    }

    connect(mDesktopPalette, &UBDesktopPalette::uniboardClick, [](){ UBApplication::applicationController->showBoard(); });
    connect(mDesktopPalette, SIGNAL(customClick()), this, SLOT(customCapture()));
    connect(mDesktopPalette, SIGNAL(screenClick()), this, SLOT(screenCapture()));
    connect(mDesktopPalette, SIGNAL(mouseEntered()), mTransparentDrawingScene, SLOT(hideTool()));
    connect(mRightPalette, SIGNAL(mouseEntered()), mTransparentDrawingScene, SLOT(hideTool()));
    connect(mRightPalette, SIGNAL(pageSelectionChangedRequired()), this, SLOT(updateBackground()));

    connect(mDesktopPalette, SIGNAL(hideOtherPalettes(QAction *)), this, SLOT(hideOtherPalettes(QAction *)));
    connect(mDesktopPalette, SIGNAL(togglePropertyPalette(QAction *)), this, SLOT(togglePropertyPalette(QAction *)));
    connect(mDesktopPalette, SIGNAL(switchCursor(QAction *)), this, SLOT(switchCursor(QAction *)));


    connect(mTransparentDrawingView, SIGNAL(resized(QResizeEvent*)), this, SLOT(onTransparentWidgetResized()));


    connect(UBDrawingController::drawingController(), SIGNAL(stylusToolChanged(int)), this, SLOT(stylusToolChanged(int)));

    // Add the desktop associated palettes
    mDesktopPenPalette = new UBDesktopPenPalette(mTransparentDrawingView, rightPalette); 

    connect(mDesktopPalette, SIGNAL(maximized()), mDesktopPenPalette, SLOT(onParentMaximized()));

    mDesktopMarkerPalette = new UBDesktopMarkerPalette(mTransparentDrawingView, rightPalette);
    mDesktopEraserPalette = new UBDesktopEraserPalette(mTransparentDrawingView, rightPalette);

    mDesktopPalette->setBackgroundBrush(UBSettings::settings()->opaquePaletteColor);
    mDesktopPenPalette->setBackgroundBrush(UBSettings::settings()->opaquePaletteColor);
    mDesktopMarkerPalette->setBackgroundBrush(UBSettings::settings()->opaquePaletteColor);
    mDesktopEraserPalette->setBackgroundBrush(UBSettings::settings()->opaquePaletteColor);


    // Hack : the size of the property palettes is computed the first time the palette is visible
    //        In order to prevent palette overlap on if the desktop palette is on the right of the
    //        screen, a setVisible(true) followed by a setVisible(false) is done.
    mDesktopPenPalette->setVisible(true);
    mDesktopMarkerPalette->setVisible(true);
    mDesktopEraserPalette->setVisible(true);
    mDesktopPenPalette->setVisible(false);
    mDesktopMarkerPalette->setVisible(false);
    mDesktopEraserPalette->setVisible(false);

    connect(UBApplication::mainWindow->actionEraseDesktopAnnotations, SIGNAL(triggered()), this, SLOT(eraseDesktopAnnotations()));
    connect(UBApplication::boardController, SIGNAL(backgroundChanged()), this, SLOT(updateColors()));
    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(updateColors()));

    // FIX #633: Ensure that these palettes stay on top of the other elements
    //mDesktopEraserPalette->raise();
    //mDesktopMarkerPalette->raise();
    //mDesktopPenPalette->raise();
}

UBDesktopAnnotationController::~UBDesktopAnnotationController()
{
    delete mTransparentDrawingScene;
    delete mTransparentDrawingView;
}

void UBDesktopAnnotationController::updateColors(){
    if(UBApplication::boardController->activeScene()->isDarkBackground()){
        mTransparentDrawingScene->setBackground(true, UBPageBackground::plain);
    }else{
        mTransparentDrawingScene->setBackground(false, UBPageBackground::plain);
    }
}

UBDesktopPalette* UBDesktopAnnotationController::desktopPalette()
{
    return mDesktopPalette;
}

QPainterPath UBDesktopAnnotationController::desktopPalettePath() const
{
    QPainterPath result;
    if (mDesktopPalette && mDesktopPalette->isVisible()) {
        result.addRect(mDesktopPalette->geometry());
    }
    if (mDesktopPenPalette && mDesktopPenPalette->isVisible()) {
        result.addRect(mDesktopPenPalette->geometry());
    }
    if (mDesktopMarkerPalette && mDesktopMarkerPalette->isVisible()) {
        result.addRect(mDesktopMarkerPalette->geometry());
    }
    if (mDesktopEraserPalette && mDesktopEraserPalette->isVisible()) {
        result.addRect(mDesktopEraserPalette->geometry());
    }

    return result;
}

void UBDesktopAnnotationController::desktopPropertyActionToggled(UBDesktopPropertyPalette* palette, QPoint pos){
    setAssociatedPalettePosition(palette, pos);
    mDesktopEraserPalette->setVisible(palette == mDesktopEraserPalette ? !palette->isVisible() : false);
    mDesktopPenPalette->setVisible(palette == mDesktopPenPalette ? !palette->isVisible() : false);
    mDesktopMarkerPalette->setVisible(palette == mDesktopMarkerPalette ? !palette->isVisible() : false);
}

/**
 * \brief Set the location of the properties palette
 * @param palette as the palette
 * @param actionName as the name of the related action
 */
void UBDesktopAnnotationController::setAssociatedPalettePosition(UBActionPalette *palette, QPoint pos)
{
    QPoint desktopPalettePos = mDesktopPalette->geometry().topLeft();
    desktopPalettePos += pos;
    if(desktopPalettePos.x() <= (mTransparentDrawingView->width() - (palette->width() + mDesktopPalette->width() + mRightPalette->width() + 20))){ // we take a small margin of 20 pixels
       desktopPalettePos += QPoint(mDesktopPalette->width() - 18, 0);
    }
    else{
       desktopPalettePos += QPoint(0 - palette->width() - 4, 0);
    }
    palette->setCustomPosition(true);
    palette->move(desktopPalettePos);
}

void UBDesktopAnnotationController::eraseDesktopAnnotations()
{
    if (mTransparentDrawingScene)
    {
        mTransparentDrawingScene->clearContent(UBGraphicsScene::clearAnnotations);
    }
}


UBBoardView* UBDesktopAnnotationController::drawingView()
{
    return mTransparentDrawingView;
}


void UBDesktopAnnotationController::showWindow()
{
    mDesktopPalette->setDisplaySelectButtonVisible(true);

    connect(UBApplication::applicationController, SIGNAL(desktopMode(bool))
            , mDesktopPalette, SLOT(setDisplaySelectButtonVisible(bool)));

    mDesktopPalette->show();
    mDesktopPalette->setArrowsForPenMarkerErasor(true);

    bool showDisplay = UBSettings::settings()->webShowPageImmediatelyOnMirroredScreen->get().toBool();

    mDesktopPalette->showHideClick(showDisplay);
    mDesktopPalette->updateShowHideState(showDisplay);

    if (!mWindowPositionInitialized)
    {
        QRect desktopRect = UBApplication::displayManager->screenGeometry(ScreenRole::Desktop);

        mDesktopPalette->move(5, desktopRect.top() + 150);

        mWindowPositionInitialized = true;
        mDesktopPalette->maximizeMe();
    }

    updateBackground();

    mBoardStylusTool = UBDrawingController::drawingController()->stylusTool();

    UBDrawingController::drawingController()->setStylusTool(mDesktopStylusTool);

#ifndef Q_OS_LINUX
    UBPlatformUtils::showFullScreen(mTransparentDrawingView);
#else
    // this is necessary to avoid hiding the panels on Unity and Cinnamon
    // if finer control is necessary, use qgetenv("XDG_CURRENT_DESKTOP")
    mTransparentDrawingView->show();
#endif
    UBPlatformUtils::setDesktopMode(true);

    mDesktopPalette->appear();
}

void UBDesktopAnnotationController::stylusToolChanged(int tool)
{
    Q_UNUSED(tool);
#ifdef Q_OS_OSX
    /* no longer needed
     if (UBDrawingController::drawingController()->isInDesktopMode())
     {
         UBStylusTool::Enum eTool = (UBStylusTool::Enum)tool;
         if(eTool == UBStylusTool::Selector)
         {
             UBPlatformUtils::toggleFinder(true);
         }
         else
         {
             UBPlatformUtils::toggleFinder(false);
         }
     }*/
#endif

    updateBackground();
}


void UBDesktopAnnotationController::updateBackground()
{
    QBrush newBrush;

    if (mIsFullyTransparent
            || UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Selector)
    {
        newBrush = QBrush(Qt::transparent);
    }
    else
    {
#if defined(Q_OS_OSX)
        newBrush = QBrush(QColor(127, 127, 127, 15));
#else
        newBrush = QBrush(QColor(127, 127, 127, 1));
#endif
    }

    if (mTransparentDrawingScene && mTransparentDrawingScene->backgroundBrush() != newBrush)
        mTransparentDrawingScene->setBackgroundBrush(newBrush);
}


void UBDesktopAnnotationController::hideWindow()
{
    if (mTransparentDrawingView)
        mTransparentDrawingView->hide();

    mDesktopPalette->hide();
    mDesktopPalette->setArrowsForPenMarkerErasor(false);


    mDesktopStylusTool = UBDrawingController::drawingController()->stylusTool();
    UBDrawingController::drawingController()->setStylusTool(mBoardStylusTool);
}

void UBDesktopAnnotationController::customCapture()
{
    hideOtherPalettes(nullptr);
    mIsFullyTransparent = true;
    updateBackground();

    mDesktopPalette->disappearForCapture();
    UBCustomCaptureWindow customCaptureWindow(mDesktopPalette);
    // need to show the window before execute it to avoid some glitch on windows.

#ifndef Q_OS_WIN // Working only without this call on win32 desktop mode
    UBPlatformUtils::showFullScreen(&customCaptureWindow);
#endif

    if (customCaptureWindow.execute(getScreenPixmap()) == QDialog::Accepted)
    {
        QPixmap selectedPixmap = customCaptureWindow.getSelectedPixmap();
        emit imageCaptured(selectedPixmap, false);
    }

    mDesktopPalette->appear();

    mIsFullyTransparent = false;
    updateBackground();
}


void UBDesktopAnnotationController::screenCapture()
{
    hideOtherPalettes(nullptr);
    mIsFullyTransparent = true;
    updateBackground();

    mDesktopPalette->disappearForCapture();

    QPixmap originalPixmap = getScreenPixmap();

    mDesktopPalette->appear();

    emit imageCaptured(originalPixmap, false);

    mIsFullyTransparent = false;

    updateBackground();
}


QPixmap UBDesktopAnnotationController::getScreenPixmap()
{
    return UBApplication::displayManager->grab(ScreenRole::Control);
}


void UBDesktopAnnotationController::updateShowHideState(bool pEnabled)
{
    mDesktopPalette->updateShowHideState(pEnabled);
}


void UBDesktopAnnotationController::screenLayoutChanged()
{
    if (UBApplication::applicationController &&
            UBApplication::displayManager &&
            UBApplication::displayManager->hasDisplay())
    {
        mDesktopPalette->setShowHideButtonVisible(true);
    }
    else
    {
        mDesktopPalette->setShowHideButtonVisible(false);
    }
}

void UBDesktopAnnotationController::switchCursor(const int tool)
{
    mTransparentDrawingScene->setToolCursor(tool);
    mTransparentDrawingView->setToolCursor(tool);
}

void UBDesktopAnnotationController::TransparentWidgetResized()
{
    onTransparentWidgetResized();
}

/**
 * \brief Resize the library palette.
 */
void UBDesktopAnnotationController::onTransparentWidgetResized()
{
    int rW = UBApplication::boardController->paletteManager()->rightPalette()->width();
    int lW = UBApplication::boardController->paletteManager()->leftPalette()->width();

    int rH = mTransparentDrawingView->height();

    UBApplication::boardController->paletteManager()->rightPalette()->resize(rW+1, rH);
    UBApplication::boardController->paletteManager()->rightPalette()->resize(rW, rH);

    UBApplication::boardController->paletteManager()->leftPalette()->resize(lW+1, rH);
    UBApplication::boardController->paletteManager()->leftPalette()->resize(lW, rH);
}

void UBDesktopAnnotationController::hideOtherPalettes(QAction *action){
    if(action != UBApplication::mainWindow->actionPen)
        mDesktopPenPalette->hide();
    if(action != UBApplication::mainWindow->actionEraser)
        mDesktopEraserPalette->hide();
    if(action != UBApplication::mainWindow->actionMarker)
        mDesktopMarkerPalette->hide();
}
void UBDesktopAnnotationController::togglePropertyPalette(QAction * action){
    if(action == UBApplication::mainWindow->actionPen)
        desktopPropertyActionToggled(mDesktopPenPalette, mDesktopPalette->penButtonPos());
    if(action == UBApplication::mainWindow->actionEraser)
        desktopPropertyActionToggled(mDesktopEraserPalette, mDesktopPalette->eraserButtonPos());
    if(action == UBApplication::mainWindow->actionMarker)
        desktopPropertyActionToggled(mDesktopMarkerPalette, mDesktopPalette->markerButtonPos());
}
void UBDesktopAnnotationController::switchCursor(QAction * action){
    if(action == UBApplication::mainWindow->actionPen)
        switchCursor(UBStylusTool::Pen);
    if(action == UBApplication::mainWindow->actionEraser)
        switchCursor(UBStylusTool::Eraser);
    if(action == UBApplication::mainWindow->actionMarker)
        switchCursor(UBStylusTool::Marker);
    if(action == UBApplication::mainWindow->actionSelector)
        switchCursor(UBStylusTool::Selector);
    if(action == UBApplication::mainWindow->actionPointer)
        switchCursor(UBStylusTool::Pointer);
}
