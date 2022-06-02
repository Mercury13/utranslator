#pragma once

#include <QLabel>

class DblClickLabel : public QLabel {
    Q_OBJECT
    using Super = QLabel;
public:
    using Super::Super;
signals:
    void doubleClicked();
protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
};
