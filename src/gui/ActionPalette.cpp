#include "ActionPalette.h"

#include <QToolButton>
#include <QLayout>
#include <QAction>
#include "FloatingPaletteWidget.h"

ActionPalette::ActionPalette(int buttonSizes)
    : mButtonSize(QSize(buttonSizes, buttonSizes))
   // _mWidget is constructed by its own. It does not require a parent, so there is nothing to pass to the constructor.
{
    new QVBoxLayout(&_mWidget);
}

void ActionPalette::show(){
    _mWidget.show();
}


void ActionPalette::addAction(QAction* action){
    QToolButton* button = createPaletteButton(action, &_mWidget);
    _mWidget.layout()->addWidget(button);
    _mWidget.layout()->setContentsMargins (12 / 2  + 5,
                                           12 / 2  + 5,
                                           12 / 2  + 5,
                                           12 / 2  + 5);
}


QAction* ActionPalette::createAndAddAction(const QIcon &icon, const QString &text){
    QAction* toReturn = new QAction(icon, text, &_mWidget);
    addAction(toReturn);
    return toReturn;
}

QToolButton* ActionPalette::createPaletteButton(QAction* action, FloatingPaletteWidget* parent)
{
    /**
     * @brief The TalkativeToolButton class is a type of QToolButton that works like a normal QToolButton
     * but in addition forwards mouse events to the parent widget, so that the floating function works
     * comfortably.
     */
    class TalkativeToolButton : public QToolButton{
        public:
            TalkativeToolButton(FloatingPaletteWidget* parent) : QToolButton(parent){}
            /**
             * @brief mousePressEvent calls the normal mousePressEvent of QToolButton so that QToolButton
             * works as usual. But it also forwards them to the parent widget, so that it can be moved around.
             * @param e
             */
            void mousePressEvent(QMouseEvent *e) override{
                QAbstractButton::mousePressEvent(e);
                // Do a static cast, because normally mousePressEvent is protected. But in FloatingPaletteWidget
                // it is public so that we can only call it after the cast. TalkativeToolButton does not want
                // to store it as FloatingPaletteWidget because it is a QWidget. But the only constructor
                // for TalkativeToolButton only accepts FloatingPaletteWidget, so we can be sure the static
                // cast works.
                (static_cast<FloatingPaletteWidget*>(parentWidget()))->mousePressEvent(e);
            }
            /**
             * @brief mouseMoveEvent calls the normal mouseMoveEvent of QToolButton so that QToolButton
             * works as usual. But it also forwards them to the parent widget, so that it can be moved around.
             * @param e
             */
            void mouseMoveEvent(QMouseEvent *e) override{
                QAbstractButton::mouseMoveEvent(e);
                // see comment above
                (static_cast<FloatingPaletteWidget*>(parentWidget()))->mouseMoveEvent(e);
            }
    };

    QToolButton* button = new TalkativeToolButton(parent);
    button->setIconSize(mButtonSize);
    button->setDefaultAction(action);
    button->setFocusPolicy(Qt::NoFocus);
    button->setStyleSheet(QString("QToolButton {color: white; font-weight: bold; font-family: Arial; background-color: transparent; border: none}"));
    button->setObjectName("ubActionPaletteButton");
    return button;
}
