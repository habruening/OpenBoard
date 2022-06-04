#ifndef ACTIONPALETTE_H
#define ACTIONPALETTE_H

//class QWidget;
class QAction;
class QToolButton;

#include <QSize>
#include "gui/FloatingPaletteWidget.h"

class ActionPalette
{
    public:

        ActionPalette(int buttonSizes);
        void addAction(QAction* action);
        QAction* createAndAddAction(const QIcon &icon, const QString &text);
        void show();

    private:

        /**
         * @brief mButtonSize is the default size for the buttons. It would be nice to determine it at
         * compile time with templates. But then we would have to do everything in the header files. This
         * is to much a deviation from the rest of the Open Board source code.
         */
        QSize mButtonSize;
        /**
         * @brief _mWidget is the Qt widget of the action palette. This is no pointer because it is a top
         * level window. So ActionPalette is responsible for it. Qt does does not delete it for us. Our
         * destructor does.
         */
        FloatingPaletteWidget _mWidget;

        QToolButton* createPaletteButton(QAction* action, QWidget *parent);


};

#endif // ACTIONPALETTE_H
