#include "ActionPalette.h"

#include <QToolButton>
#include <QLayout>
#include <QAction>
#include "FloatingPaletteWidget.h"

ActionPalette::ActionPalette(int buttonSizes)
    : mButtonSize(QSize(buttonSizes, buttonSizes))
  //  , _mWidget(new FloatingPaletteWidget)
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

QToolButton* ActionPalette::createPaletteButton(QAction* action, QWidget *parent)
{
    QToolButton* button = new QToolButton(parent);
    button->setIconSize(mButtonSize);
    button->setDefaultAction(action);
    button->setFocusPolicy(Qt::NoFocus);
    button->setStyleSheet(QString("QToolButton {color: white; font-weight: bold; font-family: Arial; background-color: transparent; border: none}"));
    button->setObjectName("ubActionPaletteButton");
    return button;
}
