#ifndef DBLCLICKSVGWIDGET_H
#define DBLCLICKSVGWIDGET_H

#include <QSvgWidget>

class DblClickSvgWidget : public QSvgWidget {
    Q_OBJECT
    using Super = QSvgWidget;
public:
    using Super::Super;
signals:
    void doubleClicked();
protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
};

#endif // DBLCLICKSVGWIDGET_H
