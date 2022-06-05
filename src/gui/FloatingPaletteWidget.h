
#ifndef FLOATINGPALETTEWIDGET_H
#define FLOATINGPALETTEWIDGET_H

#include <QWidget>
class QMouseEvent;

/**
 * @brief The FloatingPaletteWidget class is the basis for all floating palettes. It has the basic functionality
 * of being a widget that can be moved around. All the contents of it is not managed by this class. Therefore
 * composition is required.
 * There are two ways how FloatingPaletteWidget can be used:
 * FloatingPaletteWidget can be used as a separate window. This is for the desktop mode where we want only the palette
 * want to see and nothing else of OpenBoard. This is like the Qt Shaped Clock Example, which you quickly find
 * in Internet. A mask is not even required because in this use case the Palette is a top level window
 * (with no parent).
 * TODO: Update this text. Not clear if it will be with parent.
 * The other use case is within the application (with a parent).
 * FloatingPaletteWidget inherits from QWidget not because of being a reusable Qt widget. FloatingPaletteWidget is not a
 * normal widget that can be used like all other widgets. For example you should not use it in other widget
 * containers, group it in layouts or add it to dock widgets. It inherits from QWidget only because top level
 * windows must be widgets.
 * It is weird having FloatingPaletteWidget for both use cases. The (probably cleaner) alternative would be to
 * have two classes IntegratedFloatingPaletteWidget and ToplevelFloatingPaletteWidget. But that is at the moment
 * not worth the effort because both approaches have the majority of the implementation identical.
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

    private:
        QPoint dragPosition;

signals:

};

#endif // FLOATINGPALETTEWIDGET_H
