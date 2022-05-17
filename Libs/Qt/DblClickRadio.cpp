#include "DblClickRadio.h"

///// DblClickRadio ////////////////////////////////////////////////////////////

void DblClickRadio::mouseDoubleClickEvent(QMouseEvent *event)
{
    Super::mouseDoubleClickEvent(event);
    emit doubleClicked();
}
