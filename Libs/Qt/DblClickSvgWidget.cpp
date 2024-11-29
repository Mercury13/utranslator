// My header
#include "DblClickSvgWidget.h"

///// DblClickSvgWidget ////////////////////////////////////////////////////////

void DblClickSvgWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Super::mouseDoubleClickEvent(event);
    emit doubleClicked();
}
