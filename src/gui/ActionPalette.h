#ifndef ACTIONPALETTE_H
#define ACTIONPALETTE_H

//class QWidget;
class QAction;
class QToolButton;

#include <QSize>
#include "gui/FloatingPaletteWidget.h"

/**
 * @brief The ActionPalette class provides functions for defining a FloatingPaletteWidget with action icons. The class
 * owns the QWidget for the palette. It is a top level windows and therefore not owned by Qt.
 */
class ActionPalette : QObject
{
    Q_OBJECT

    public:

        /**
         * @brief The constuctor ActionPalette the control logic for owning and working with a FloatingPaletteWidget.
         * @param buttonSizes is the default size of all icons in the palette.
         */
        ActionPalette(int buttonSizes);
        /**
         * @brief addAction add an already existing action to the palette
         * @param action the action to be added into the palette
         */
        void addAction(QAction* action);
        /**
         * @brief createAndAddAction creates a new action and adds it to the palette
         * @param icon the icon of the action
         * @param text the tooltip shortcut of the action
         * @return the action that has been created
         */
        QAction* createAndAddAction(const QIcon &icon, const QString &text);
        /**
         * @brief shows the action palette
         */
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

        /**
         * @brief createPaletteButton creates a new tool button. Only tool buttons that have been created
         * with this function will work correctly in the ActionPalette. (If other QToolButtons are incorrectly
         * added, the drag and drop functionality of the palette will not work correctly.)
         * @param action tha action behind the tool button
         * @param parent the palette widget. Buttons that are created with createPaletteButton are intended to
         * work only in FloatingPaletteWidget.
         * @return a new QToolButton, which can only be used in FloatingPaletteWidget
         */
        QToolButton* createPaletteButton(QAction* action, FloatingPaletteWidget *parent);


};

#endif // ACTIONPALETTE_H
