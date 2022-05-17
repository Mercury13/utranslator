#pragma once

#include <QRadioButton>

class DblClickRadio : public QRadioButton {
    Q_OBJECT
    using Super = QRadioButton;
public:
    using Super::Super;
signals:
    void doubleClicked();
protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
};
