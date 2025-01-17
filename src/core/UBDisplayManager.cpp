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

#include "UBDisplayManager.h"

#include "frameworks/UBPlatformUtils.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "board/UBBoardView.h"
#include "board/UBBoardController.h"

#include "gui/UBBlackoutWidget.h"

#include "ui_blackoutWidget.h"

#include "core/memcheck.h"

UBDisplayManager::UBDisplayManager(QObject *parent)
    : QObject(parent)
{
    mUseMultiScreen = UBSettings::settings()->appUseMultiscreen->get().toBool();

    initScreenIndexes();

    connect(qApp, &QGuiApplication::screenAdded, this, &UBDisplayManager::addOrRemoveScreen);
    connect(qApp, &QGuiApplication::screenRemoved, this, &UBDisplayManager::addOrRemoveScreen);
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, &UBDisplayManager::addOrRemoveScreen);

    connect(UBSettings::settings()->appScreenList, &UBSetting::changed, this, &UBDisplayManager::adjustScreens);
}

UBDisplayManager::~UBDisplayManager()
{
    // NOOP
}

void UBDisplayManager::initScreenIndexes()
{
    mScreensByRole.clear();
    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    QList<QScreen*> screens = primaryScreen->virtualSiblings();

    // make sure primary screen is first element and therefore never dropped
    screens.insert(0, primaryScreen);

    // drop screens which duplicate another screen, i.e. have same geometry
    for (int i = 1; i < screens.size(); )
    {
        QRect iGeomentry = screens[i]->geometry();

        for (int j = 0; j < i; ++j)
        {
            QRect jGeometry = screens[j]->geometry();

            if (iGeomentry.contains(jGeometry) || jGeometry.contains(iGeomentry))
            {
                screens.removeAt(i);
                break;
            }
        }

        ++i;
    }

    if (screens.count() != mAvailableScreens.count())
    {
        mAvailableScreens = screens;
        emit availableScreenCountChanged(screens.count());
    }

    QStringList screenList = UBSettings::settings()->appScreenList->get().toStringList();

    if (screenList.empty())
    {
        // "old" configuration mode
        bool swapScreens = UBSettings::settings()->swapControlAndDisplayScreens->get().toBool();

        mScreensByRole[ScreenRole::Control] = screens[0];

        if (screens.count() > 1)
        {
            QScreen* controlScreen = screens[0];
            QScreen* displayScreen = screens[1];

            if (swapScreens)
            {
                std::swap(controlScreen, displayScreen);
            }

            mScreensByRole[ScreenRole::Control] = controlScreen;
            screenList << controlScreen->name();

            if (mUseMultiScreen)
            {
                mScreensByRole[ScreenRole::Display] = displayScreen;
                screenList << displayScreen->name();
                ScreenRole role(ScreenRole::Previous1);

                for (int i = 2; i < screens.count(); ++i)
                {
                    mScreensByRole[role++] = screens[i];
                    screenList << screens[i]->name();
                }
            }

            if (screenList.count() > 1)
            {
                // Convert configuration to new mode
                qDebug() << "Screen setting converted to screen list" << screenList;
                UBSettings::settings()->appScreenList->set(screenList);
            }
        }
    }
    else
    {
        // "new" configuration mode using list of screen names

        // first, create a map of screens by name
        QMap<QString,QScreen*> screenByName;

        for (QScreen* screen : mAvailableScreens)
        {
            screenByName[screen->name()] = screen;
        }

        // configure control screen
        QScreen* controlScreen = screenByName.value(screenList[0], nullptr);

        if (!controlScreen)
        {
            // by default use primary screen and remove it from the list of available screens
            controlScreen = primaryScreen;
            screenByName.remove(controlScreen->name());
        }

        mScreensByRole[ScreenRole::Control] = controlScreen;

        // configure display screen
        if (mUseMultiScreen && screenList.count() > 1)
        {
            QScreen* displayScreen = screenByName.value(screenList[1], nullptr);

            if (displayScreen)
            {
                mScreensByRole[ScreenRole::Display] = displayScreen;
            }
        }

        // configure previous screens
        ScreenRole role = ScreenRole::Previous1;

        for (int i = 2; i < screenList.count(); ++i)
        {
            QScreen* previousScreen = screenByName.value(screenList[i], nullptr);

            if (previousScreen)
            {
                mScreensByRole[role++] = previousScreen;
            }
        }
    }


    // Desktop screen is same as Control screen
    mScreensByRole[ScreenRole::Desktop] = mScreensByRole[ScreenRole::Control];
}

int UBDisplayManager::numScreens()
{
    return mAvailableScreens.count();
}


int UBDisplayManager::numPreviousViews()
{
    int previousViews = 0;
    ScreenRole role(ScreenRole::Previous1);

    for (int i = 0; i < 5; ++i)
    {
        if (mScreensByRole.contains(role++))
        {
            ++previousViews;
        }
    }

    return previousViews;
}


void UBDisplayManager::setControlWidget(QWidget* pControlWidget)
{
    if(hasControl() && pControlWidget)
        mWidgetsByRole[ScreenRole::Control] = pControlWidget;
}

void UBDisplayManager::setDesktopWidget(QWidget* pDesktopWidget )
{
    if(pDesktopWidget)
        mWidgetsByRole[ScreenRole::Desktop] = pDesktopWidget;
}

void UBDisplayManager::setDisplayWidget(QWidget* pDisplayWidget)
{
    if(pDisplayWidget)
    {
        if (mWidgetsByRole.contains(ScreenRole::Display))
        {
            mWidgetsByRole[ScreenRole::Display]->hide();
            pDisplayWidget->setGeometry(mWidgetsByRole[ScreenRole::Display]->geometry());
            pDisplayWidget->setWindowFlags(mWidgetsByRole[ScreenRole::Display]->windowFlags());
        }

        mWidgetsByRole[ScreenRole::Display] = pDisplayWidget;

        if (mScreensByRole.contains(ScreenRole::Display))
        {
            mWidgetsByRole[ScreenRole::Display]->setGeometry(mScreensByRole[ScreenRole::Display]->geometry());
            UBPlatformUtils::showFullScreen(mWidgetsByRole[ScreenRole::Display]);
        }
    }
}


void UBDisplayManager::setPreviousDisplaysWidgets(QList<UBBoardView*> pPreviousViews)
{
    ScreenRole role(ScreenRole::Previous1);

    for (int i = 0; i < pPreviousViews.size(); ++i)
    {
        mWidgetsByRole[role++] = pPreviousViews[i];
    }
}

QWidget* UBDisplayManager::widget(ScreenRole role)
{
    return mWidgetsByRole.value(role, nullptr);
}

QList<QScreen *> UBDisplayManager::availableScreens() const
{
    return mAvailableScreens;
}

void UBDisplayManager::adjustScreens()
{
    initScreenIndexes();
    positionScreens();

    emit screenLayoutChanged();
}


void UBDisplayManager::positionScreens()
{
    if(mWidgetsByRole.contains(ScreenRole::Desktop) && hasControl())
    {
        mWidgetsByRole[ScreenRole::Desktop]->hide();
        mWidgetsByRole[ScreenRole::Desktop]->setGeometry(mScreensByRole[ScreenRole::Control]->geometry());
    }
    if (mWidgetsByRole.contains(ScreenRole::Control) && hasControl())
    {
        mWidgetsByRole[ScreenRole::Control]->showNormal();
        mWidgetsByRole[ScreenRole::Control]->setGeometry(mScreensByRole[ScreenRole::Control]->geometry());
        UBPlatformUtils::showFullScreen(mWidgetsByRole[ScreenRole::Control]);
    }

    if (mWidgetsByRole.contains(ScreenRole::Display) && hasDisplay())
    {
        mWidgetsByRole[ScreenRole::Display]->showNormal();
        mWidgetsByRole[ScreenRole::Display]->setGeometry(mScreensByRole[ScreenRole::Display]->geometry());
        UBPlatformUtils::showFullScreen(mWidgetsByRole[ScreenRole::Display]);
    }
    else if(mWidgetsByRole.contains(ScreenRole::Display))
    {
        mWidgetsByRole[ScreenRole::Display]->hide();
    }

    ScreenRole role(ScreenRole::Previous1);

    for (int i = 0; i < 5; ++i)
    {
        if (mWidgetsByRole.contains(role))
        {
            if (mScreensByRole.contains(role)) {
                QWidget* previous = mWidgetsByRole[role];
                previous->setGeometry(mScreensByRole[role]->geometry());
                UBPlatformUtils::showFullScreen(previous);
            }
            else
            {
                mWidgetsByRole[role]->hide();
            }
        }

        ++role;
    }

    if (mWidgetsByRole.contains(ScreenRole::Control) && hasControl())
        mWidgetsByRole[ScreenRole::Control]->activateWindow();
}


void UBDisplayManager::blackout()
{
    for (auto screen : mScreensByRole)
    {
        UBBlackoutWidget *blackoutWidget = new UBBlackoutWidget(); //deleted in UBDisplayManager::unBlackout
        Ui::BlackoutWidget *blackoutUi = new Ui::BlackoutWidget();
        blackoutUi->setupUi(blackoutWidget);

        connect(blackoutUi->iconButton, SIGNAL(pressed()), blackoutWidget, SLOT(doActivity()));
        connect(blackoutWidget, SIGNAL(activity()), this, SLOT(unBlackout()));

        // display Uniboard logo on main screen
        bool isControlScreen = screen == mScreensByRole[ScreenRole::Control];
        blackoutUi->iconButton->setVisible(isControlScreen);
        blackoutUi->labelClickToReturn->setVisible(isControlScreen);

        blackoutWidget->setGeometry(screen->geometry());

        mBlackoutWidgets << blackoutWidget;
    }

    UBPlatformUtils::fadeDisplayOut();

    foreach(UBBlackoutWidget *blackoutWidget, mBlackoutWidgets)
    {
        UBPlatformUtils::showFullScreen(blackoutWidget);
    }
}

void UBDisplayManager::unBlackout()
{
    while (!mBlackoutWidgets.isEmpty())
    {
        // the widget is also destroyed thanks to its Qt::WA_DeleteOnClose attribute
        mBlackoutWidgets.takeFirst()->close();
    }

    UBPlatformUtils::fadeDisplayIn();

    UBApplication::boardController->freezeW3CWidgets(false);

}


void UBDisplayManager::addOrRemoveScreen(QScreen *screen)
{
    Q_UNUSED(screen);
    // adjustment must be delayed, because OS also tries to position the widgets
    QTimer::singleShot(3000, [this](){ adjustScreens(); } );
}


void UBDisplayManager::setUseMultiScreen(bool pUse)
{
    mUseMultiScreen = pUse;
}

QSize UBDisplayManager::screenSize(ScreenRole role) const
{
    QScreen* screen = mScreensByRole.value(role, nullptr);
    return screen ? screen->size() : QSize();
}

QSize UBDisplayManager::availableScreenSize(ScreenRole role) const
{
    QScreen* screen = mScreensByRole.value(role, nullptr);
    return screen ? screen->availableSize() : QSize();
}

QRect UBDisplayManager::screenGeometry(ScreenRole role) const
{
    QScreen* screen = mScreensByRole.value(role, nullptr);
    return screen ? screen->geometry() : QRect();
}

qreal UBDisplayManager::physicalDpi(ScreenRole role) const
{
    QScreen* screen = mScreensByRole.value(role, nullptr);
    return screen ? screen->physicalDotsPerInch() : 96.;
}

qreal UBDisplayManager::logicalDpi(ScreenRole role) const
{
    QScreen* screen = mScreensByRole.value(role, nullptr);
    return screen ? screen->logicalDotsPerInch() : 96.;
}

QPixmap UBDisplayManager::grab(ScreenRole role, QRect rect) const
{
    QScreen* screen = mScreensByRole.value(role, nullptr);

    if (screen)
    {
        // see https://doc.qt.io/qt-6.2/qtwidgets-desktop-screenshot-example.html
        // for using window id 0
        return screen->grabWindow(0, rect.x(), rect.y(), rect.width(), rect.height());
    }

    return QPixmap();
}

QPixmap UBDisplayManager::grabGlobal(QRect rect) const
{
    QScreen* screen = QGuiApplication::screenAt(rect.topLeft());

    if (screen) {
        rect.translate(-screen->geometry().topLeft());
        return screen->grabWindow(0, rect.x(), rect.y(), rect.width(), rect.height());
    }

    return QPixmap();
}


ScreenRole &operator++(ScreenRole &role)
{
    role = ScreenRole(int(role) + 1);
    return role;
}

ScreenRole operator++(ScreenRole &role, int)
{
    ScreenRole old = role;  // copy old value
    ++role;                 // prefix increment
    return old;             // return old value
}
