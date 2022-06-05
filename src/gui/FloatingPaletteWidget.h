
#ifndef FLOATINGPALETTEWIDGET_H
#define FLOATINGPALETTEWIDGET_H

#include <QWidget>
#include <QTimer>
class QMouseEvent;

/**
 * @brief The FloatingPaletteWidget class is the basis for all floating palettes. It has the basic functionality
 * of being a widget that can be moved around. All the contents of it is not managed by this class. Therefore
 * composition is required.
 * FloatingPaletteWidget is used as a separate window. This is for the desktop mode where we want only the palette
 * want to see and nothing else of OpenBoard. This is like the Qt Shaped Clock Example, which you quickly find
 * in Internet. A mask is not even required because in this use case the Palette is a top level window
 * (with no parent).
 * FloatingPaletteWidget inherits from QWidget. But it is not a reusable Qt widget. FloatingPaletteWidget is not a
 * normal widget that can be used like all other widgets. For example you should not use it in other widget
 * containers, group it in layouts or add it to dock widgets. It inherits from QWidget only because top level
 * windows must be widgets.
 */
class FloatingPaletteWidget : public QWidget
{
    Q_OBJECT

    public:

        explicit FloatingPaletteWidget(QWidget *parent = nullptr);

        void mouseMoveEvent(QMouseEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void closeEvent(QCloseEvent *event) override;
        void paintEvent(QPaintEvent *) override;

        /**
         * @brief widget_was_moved_after_mouse_press_event tells if FloatingPaletteWidget consumed
         * mouse move events.
         * @return true if FloatingPaletteWidget was moved after the last mouse press event.
         */
        bool widget_was_moved_after_mouse_press_event() const;

    private:
        QPoint mousePressPosition, dragPosition;
        bool was_dragged;

signals:

};

#endif // FLOATINGPALETTEWIDGET_H
