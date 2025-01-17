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




#ifndef UBUNINOTESWINDOW_H_
#define UBUNINOTESWINDOW_H_
#include <QtGui>
#include <QShowEvent>
#include <QHideEvent>

#include "gui/UBActionPalette.h"
#include "gui/UBRightPalette.h"

#define PROPERTY_PALETTE_TIMER      1000

/**
 * The uninotes window. This window is controlled by UBUninotesWindowController.
 */
class UBDesktopPalette : public UBActionPalette
{
    Q_OBJECT

    public:
        UBDesktopPalette(QWidget *parent, UBRightPalette* rightPalette);
        virtual ~UBDesktopPalette();

        void disappearForCapture();
        void appear();

        std::function<QPoint()> penButtonPos;
        std::function<QPoint()> eraserButtonPos;
        std::function<QPoint()> markerButtonPos;


 // The following actions are owned by UBDesktopPalette and therefore we have to produce signals for them.
        QAction *mActionUniboard;
        QAction *mActionCustomSelect;
        QAction *mDisplaySelectAction;

    signals:
        void uniboardClick();
        void customClick();
        void screenClick();

// The following actions are owned by UBDesktopPalette. But they are handled internally in UBDesktopPalette.
// Therefore slots exist. UBDesktopPalette does not emit signals for them.

    private:

        QAction *mMaximizeAction;
        QAction *mShowHideAction;

    public slots:

        void showHideClick(bool checked);
        void maximizeMe();

    signals:
//#ifdef Q_OS_LINUX //TODO: check why this produces an error on linux if uncommented
        void refreshMask();
//#endif

    public slots:

        void updateShowHideState(bool pShowEnabled);
        void setShowHideButtonVisible(bool visible);
        void setDisplaySelectButtonVisible(bool show);

        void setArrowsForPenMarkerErasor(bool showAarrows);

    private:

        eMinimizedLocation mMinimizedLocation;


        virtual void minimizePalette(const QPoint& pos);
        virtual void minimizeMe();

        QAction *pendingButton;
        QTime mButtonHoldTimer;

        std::function<QPoint()> addActionAndConnectWithPressedReleasedEvent(QAction* action, int stylusTool = 0, bool connectPressedEvent = false);
        void createAndConnectButtons();

        void actionPressed(QToolButton* button, QAction* action, int stylusTool);
        void actionReleased(QAction* action);

signals:
        void hideOtherPalettes(QAction *);
        void togglePropertyPalette(QAction *);
        void switchCursor(QAction *);


        void stylusToolChanged(int tool);

};

#endif /* UBUNINOTESWINDOW_H_ */
