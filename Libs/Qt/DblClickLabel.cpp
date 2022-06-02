#include "DblClickLabel.h"

///// DblClickLabel ////////////////////////////////////////////////////////////

void DblClickLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    Super::mouseDoubleClickEvent(event);
    emit doubleClicked();
}
