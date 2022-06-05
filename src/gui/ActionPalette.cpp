#include "ActionPalette.h"

#include <QToolButton>
#include <QLayout>
#include <QAction>
#include <QMouseEvent>
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
            TalkativeToolButton(FloatingPaletteWidget* parent)
                : QToolButton(parent)
                , mousePressEvent_consumed_by_FloatingPaletteWidget(false){}
            /**
             * @brief mousePressEvent calls the normal mousePressEvent of QToolButton so that QToolButton
             * works as usual. But it also forwards them to the parent widget, so that it can be moved around.
             * @param e
             */
            void mousePressEvent(QMouseEvent *e) override{
                QAbstractButton::mousePressEvent(e);
                // We want also to propagate the signal to the FloatingPaletteWidget so that we can
                // comfortably move it.
                // QAbstractButton::mousePressEvent already accepted it. But we set it back to ignore.
                e->ignore();
            }
            /**
             * @brief mouseMoveEvent calls the normal mouseMoveEvent of QToolButton so that QToolButton
             * works as usual. But it also forwards them to the parent widget, so that it can be moved around.
             * @param e
             */
            void mouseMoveEvent(QMouseEvent *e) override{
                QToolButton::mouseMoveEvent(e);
                e->ignore();
                // We want to know if the FloatingPaletteWidget was moved. Because in this case we don't use
                // the last mouse click event any more. We need a static cast to ask FloatingPaletteWidget.
                // This is the reason why we only accept FloatingPaletteWidget in the constructor. The
                // static cast therefore is safe.
                auto parent_as_FloatingPaletteWidget = static_cast<FloatingPaletteWidget*>(parentWidget());
                mousePressEvent_consumed_by_FloatingPaletteWidget = parent_as_FloatingPaletteWidget->widget_was_moved_after_mouse_press_event();
            }
            void mouseReleaseEvent(QMouseEvent *e) override{
                if(!mousePressEvent_consumed_by_FloatingPaletteWidget)
                    QToolButton::mouseReleaseEvent(e);
                mousePressEvent_consumed_by_FloatingPaletteWidget = false;
                // TODO: I cannot see if this is reliable. Clarify if no other mouseReleaseEvents
                //       can trigger the button.
            }
        private:
            bool mousePressEvent_consumed_by_FloatingPaletteWidget;
    };

    QToolButton* button = new TalkativeToolButton(parent);
    button->setIconSize(mButtonSize);
    button->setDefaultAction(action);
    button->setFocusPolicy(Qt::NoFocus);
    button->setStyleSheet(QString("QToolButton {color: white; font-weight: bold; font-family: Arial; background-color: transparent; border: none}"));
    button->setObjectName("ubActionPaletteButton");
    return button;
}
