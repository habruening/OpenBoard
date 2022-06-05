#include "FloatingPaletteWidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include "core/UBSettings.h"

FloatingPaletteWidget::FloatingPaletteWidget(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint)
    , was_dragged(false)
{

}

void FloatingPaletteWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    mousePressPosition = event->globalPos();
    dragPosition = event->globalPos() - frameGeometry().topLeft();
    was_dragged = false;
    event->accept();
}

void FloatingPaletteWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;

    auto motion = event->globalPos() - mousePressPosition;
    if(motion.manhattanLength() <= 3)
        return;

    move(event->globalPos() - dragPosition);
    was_dragged = true;
    event->accept();
}

bool FloatingPaletteWidget::widget_was_moved_after_mouse_press_event() const{
    return was_dragged;
}

void FloatingPaletteWidget::closeEvent(QCloseEvent *event)
{
    event->ignore();
}

void FloatingPaletteWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(QColor(170, 170 ,170)));
    QPainterPath borderPath;
    borderPath.addRoundedRect(0, 0, width(), height(), 10, 10);
    borderPath.addRoundedRect(5, 5, width() - 2 * 5, height() - 2 * 5, 10, 10);
    painter.drawPath(borderPath);
    painter.setBrush(UBSettings::settings()->opaquePaletteColor);
    painter.drawRoundedRect(5, 5, width() - 2 * 5, height() - 2 * 5, 10, 10);
}
