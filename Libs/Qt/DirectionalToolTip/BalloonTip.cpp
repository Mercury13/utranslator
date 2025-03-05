#include "BalloonTip.h"
#include <QBitmap>
#include <QPainter>
#include <QMouseEvent>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QScreen>
#include <QGridLayout>
#include <QApplication>
#include <QPainterPath>
#include <QMessageBox>

//#include <QWidgetTextControl>


static BalloonTip *theSolitaryBalloonTip = nullptr;

BalloonDir BalloonTip::flipDir(BalloonDir x)
{
    if (QApplication::isRightToLeft() && x != BalloonDir::BLN_12_OC) {
        return static_cast<BalloonDir>(12 - static_cast<int>(x));
    }
    return x;
}

void BalloonTip::showBalloon(
        QMessageBox::Icon icon, const QString& title,
        const QString& msg,
        const QPoint& pos, int timeout,
        BalloonDir arrowDir)
{
    showBalloonAbsDir(icon, title, msg, pos, timeout, flipDir(arrowDir));
}


void BalloonTip::showBalloonAbsDir(
        QMessageBox::Icon icon, const QString& title,
        const QString& message,
        const QPoint& pos, int timeout,
        BalloonDir arrowDir)
{
    hideBalloon();
    if (message.isEmpty() && title.isEmpty())
        return;

    theSolitaryBalloonTip = new BalloonTip(icon, title, message);
    if (timeout < 0)
        timeout = 10000; //10 s default
    theSolitaryBalloonTip->balloonAbsDir(pos, timeout, arrowDir);
}

void BalloonTip::hideBalloon()
{
    if (!theSolitaryBalloonTip)
        return;
    theSolitaryBalloonTip->hide();
    delete theSolitaryBalloonTip;
    theSolitaryBalloonTip = 0;
}

void BalloonTip::updateBalloonPosition(const QPoint& pos)
{
    if (!theSolitaryBalloonTip)
        return;
    theSolitaryBalloonTip->hide();
    theSolitaryBalloonTip->balloonAbsDir(pos, 0, theSolitaryBalloonTip->arrowDir);
}

bool BalloonTip::isBalloonVisible()
{
    return theSolitaryBalloonTip;
}

BalloonTip::BalloonTip(QMessageBox::Icon icon, const QString& title,
                         const QString& message)
    : QWidget(nullptr, Qt::Popup), timerId(-1), eventTimerId(-1), enablePressEvent(false)
{
    setAttribute(Qt::WA_DeleteOnClose);
    //QObject::connect(ti, SIGNAL(destroyed()), this, SLOT(close()));

    QLabel *titleLabel = new QLabel;
    //titleLabel->installEventFilter(this);
    titleLabel->setText(title);
    QFont f = titleLabel->font();
    f.setBold(true);
#ifdef Q_OS_WINCE
    f.setPointSize(f.pointSize() - 2);
#endif
    titleLabel->setFont(f);
    titleLabel->setTextFormat(Qt::PlainText); // to maintain compat with windows

#ifdef Q_OS_WINCE
    const int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize);
    const int closeButtonSize = style()->pixelMetric(QStyle::PM_SmallIconSize) - 2;
#else
    const int iconSize = 18;
    //const int closeButtonSize = 15;
#endif

    /*QPushButton *closeButton = new QPushButton;
    closeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    closeButton->setIconSize(QSize(closeButtonSize, closeButtonSize));
    closeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    closeButton->setFixedSize(closeButtonSize, closeButtonSize);
    QObject::connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));*/

    QLabel *msgLabel = new QLabel;
#ifdef Q_OS_WINCE
    f.setBold(false);
    msgLabel->setFont(f);
#endif
    msgLabel->installEventFilter(this);
    msgLabel->setText(message);
    msgLabel->setTextFormat(Qt::PlainText);
    msgLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    // smart size for the message label
    int limit = msgLabel->screen()->availableGeometry().size().width() / 3;

    if (msgLabel->sizeHint().width() > limit) {
        msgLabel->setWordWrap(true);
        /*if (msgLabel->sizeHint().width() > limit) {
            msgLabel->d_func()->ensureTextControl();
            if (QWidgetTextControl *control = msgLabel->d_func()->control) {
                QTextOption opt = control->document()->defaultTextOption();
                opt.setWrapMode(QTextOption::WrapAnywhere);
                control->document()->setDefaultTextOption(opt);
            }
        }*/

        // Here we allow the text being much smaller than the balloon widget
        // to emulate the weird standard windows behavior.
        msgLabel->setFixedSize(limit, msgLabel->heightForWidth(limit));
    }

    QIcon si;
    switch (icon) {
    case QMessageBox::Warning:
        si = style()->standardIcon(QStyle::SP_MessageBoxWarning);
        break;
    case QMessageBox::Critical:
        si = style()->standardIcon(QStyle::SP_MessageBoxCritical);
        break;
    case QMessageBox::Information:
        si = style()->standardIcon(QStyle::SP_MessageBoxInformation);
        break;
    case QMessageBox::NoIcon:
    default:
        break;
    }
    anyWidget = msgLabel;

    QGridLayout *layout = new QGridLayout;
    if (!si.isNull()) {
        QLabel *iconLabel = new QLabel;
        iconLabel->setPixmap(si.pixmap(iconSize, iconSize));
        iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        iconLabel->setMargin(2);
        layout->addWidget(iconLabel, 0, 0);
        layout->addWidget(titleLabel, 0, 1);
    } else {
        layout->addWidget(titleLabel, 0, 0, 1, 2);
    }

    //layout->addWidget(closeButton, 0, 2);
    layout->addWidget(msgLabel, 1, 0, 1, 3);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->setContentsMargins(3, 3, 3, 3);
    setLayout(layout);

    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(0xff, 0xff, 0xe1));
    pal.setColor(QPalette::WindowText, Qt::black);
    setPalette(pal);
}

BalloonTip::~BalloonTip()
{
    theSolitaryBalloonTip = 0;
}

void BalloonTip::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    painter.drawPixmap(rect(), pixmap);
}

void BalloonTip::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}

void BalloonTip::balloonAbsDir(const QPoint& pos, int msecs,
                         BalloonDir arrowDir)
{
    enablePressEvent = false;
    this->arrowDir = arrowDir;
    //QRect scr = QApplication::desktop()->screenGeometry(pos);
    QSize sh;
    const int border = 1;
    constexpr int ah = 18, ao = 18, aw = 18, rc = 7;

    enum class CoarseDir : unsigned char { ARR_TOP, ARR_RIGHT, ARR_LEFT, ARR_BOTTOM };
    CoarseDir coarseDir = CoarseDir::ARR_TOP;

    switch (arrowDir)
    {
    case BalloonDir::ARR_11_OC:
    case BalloonDir::ARR_12_OC:
    case BalloonDir::ARR_1_OC:
        coarseDir = CoarseDir::ARR_TOP; break;
    case BalloonDir::ARR_2_OC:
    case BalloonDir::ARR_3_OC:
    case BalloonDir::ARR_4_OC:
        coarseDir = CoarseDir::ARR_RIGHT; break;
    case BalloonDir::ARR_5_OC:
    case BalloonDir::ARR_6_OC:
    case BalloonDir::ARR_7_OC:
        coarseDir = CoarseDir::ARR_BOTTOM; break;
    case BalloonDir::ARR_8_OC:
    case BalloonDir::ARR_9_OC:
    case BalloonDir::ARR_10_OC:
        coarseDir = CoarseDir::ARR_LEFT; break;
    }

    auto ahIf = [coarseDir](CoarseDir when) {
            return (coarseDir == when) ? ah : 0;
        };

    //bool arrowAtTop =  (pos.y() + sh.height() + ah < scr.height());
    //bool arrowAtLeft = (pos.x() + sh.width() - ao < scr.width());
    setContentsMargins(border + ahIf(CoarseDir::ARR_LEFT),
                       border + ahIf(CoarseDir::ARR_TOP),
                       border + ahIf(CoarseDir::ARR_RIGHT) + 1,
                       border + ahIf(CoarseDir::ARR_BOTTOM) + 1);
    updateGeometry();
    sh  = sizeHint();
    qreal ml = 0, mr = 0, mt = 0, mb = 0;
    ml = ahIf(CoarseDir::ARR_LEFT) + 0.5;
    mt = ahIf(CoarseDir::ARR_TOP) + 0.5;
    mr = sh.width() - ahIf(CoarseDir::ARR_RIGHT) - 0.5;
    mb = sh.height() - ahIf(CoarseDir::ARR_BOTTOM) - 0.5;


    QPainterPath path;
    path.moveTo(ml + rc, mt);
    // draw TOP
    switch (arrowDir) {
    case BalloonDir::ARR_11_OC:
        path.lineTo(ml + ao, mt);
        path.lineTo(ml + ao, mt - ah);
        path.lineTo(ml + ao + aw, mt);
        move(pos.x() - ao, pos.y());
        break;
    case BalloonDir::ARR_1_OC:
        path.lineTo(mr - ao - aw, mt);
        path.lineTo(mr - ao, mt - ah);
        path.lineTo(mr - ao, mt);
        move(pos.x() - sh.width() + ao, pos.y());
        break;
    case BalloonDir::ARR_12_OC: {
            int x = (sh.width() - aw) / 2;
            path.lineTo(ml + x, mt);
            path.lineTo(ml + x + aw / 2, mt - ah);
            path.lineTo(ml + x + aw, mt);
            move(pos.x() - x - rc, pos.y());
        } break;
    default: ;
    }
    //path.lineTo(mr - rc, mt);
    path.arcTo(mr - rc*2, mt, rc*2, rc*2, 90, -90);

    switch (arrowDir) {
    case BalloonDir::ARR_2_OC:
        path.lineTo(mr, mt + ao);
        path.lineTo(mr + ah, mt + ao);
        path.lineTo(mr, mt + ao + aw);
        move(pos.x() - sh.width(), pos.y() - ao);
        break;
    case BalloonDir::ARR_4_OC:
        path.lineTo(mr, mb - ao - aw);
        path.lineTo(mr + ah, mb - ao);
        path.lineTo(mr, mb - ao);
        move(pos.x() - sh.width(), pos.y() - sh.height() + ao);
        break;
    case BalloonDir::ARR_3_OC: {
            int x = (sh.height() - aw) / 2;
            path.lineTo(mr, mt + x);
            path.lineTo(mr + ah, mt + x + aw / 2);
            path.lineTo(mr, mt + x + aw);
            move(pos.x() - sh.width(), pos.y() - x - rc);
        } break;
    default: ;
    }

    //path.lineTo(mr, mb - rc);
    path.arcTo(mr - rc*2, mb - rc*2, rc*2, rc*2, 0, -90);

    switch (arrowDir) {
    case BalloonDir::ARR_5_OC:
        path.lineTo(mr - ao, mb);
        path.lineTo(mr - ao, mb + ah);
        path.lineTo(mr - ao - aw, mb);
        move(pos.x() - sh.width() + ao,
             pos.y() - sh.height());
        break;
    case BalloonDir::ARR_7_OC:
        path.lineTo(ao + aw, mb);
        path.lineTo(ao, mb + ah);
        path.lineTo(ao, mb);
        move(pos.x() - ao, pos.y() - sh.height());
        break;
    case BalloonDir::ARR_6_OC: {
            int x = (sh.width() - aw) / 2;
            path.lineTo(mr - x, mb);
            path.lineTo(mr - x - aw / 2, mb + ah);
            path.lineTo(mr - x - aw, mb);
            move(pos.x() - x - rc, pos.y() - sh.height());
        } break;
    default: ;
    }
    //path.lineTo(ml + rc, mb);
    path.arcTo(ml, mb - rc*2, rc*2, rc*2, 270, -90);

    switch (arrowDir) {
    case BalloonDir::ARR_8_OC:
        path.lineTo(ml, mb - ao);
        path.lineTo(ml - ah, mb - ao);
        path.lineTo(ml, mb - ao - aw);
        move(pos.x(), pos.y() - sh.height() + ao);
        break;
    case BalloonDir::ARR_10_OC:
        path.lineTo(ml, mt + ao + aw);
        path.lineTo(ml - ah, mt + ao);
        path.lineTo(ml, mt + ao);
        move(pos.x(), pos.y() - ao);
        break;
    case BalloonDir::ARR_9_OC: {
            int x = (sh.height() - aw) / 2;
            path.lineTo(ml, mb - x);
            path.lineTo(ml - ah, mb - x - aw /2);
            path.lineTo(ml, mb - x - aw);
            move(pos.x(), pos.y() - x - rc);
        } break;
    default: ;
    }

    //path.lineTo(ml, mt + rc);
    path.arcTo(ml, mt, rc*2, rc*2, 180, -90);
    path.closeSubpath();

    // Set the mask
    QBitmap bitmap = QBitmap(sh);
    bitmap.fill(Qt::color0);
    QPainter painter1(&bitmap);
    painter1.setPen(QPen(Qt::color1, border));
    painter1.setBrush(QBrush(Qt::color1));
    painter1.drawPath(path);
    setMask(bitmap);

    // Draw the border
    QColor penColor = palette().color(QPalette::Window).darker(160);
    QPen pen = QPen(penColor, border);
    pixmap = QPixmap(sh);
    pixmap.fill(penColor);
    QPainter painter2(&pixmap);
    painter2.setRenderHint(QPainter::Antialiasing, true);
    painter2.setPen(pen);
    painter2.setBrush(palette().color(QPalette::Window));
    painter2.drawPath(path);

//    setMask(pixmap.mask());

    if (msecs > 0)
        timerId = startTimer(msecs);

    //eventTimerId = startTimer(500);
    show();
}

void BalloonTip::mousePressEvent(QMouseEvent *)
{
//    if (enablePressEvent)
        close();
}

void BalloonTip::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == timerId) {
        killTimer(timerId);
        if (!underMouse())
            close();
        return;
    }
//    if (e->timerId() == eventTimerId)
//        enablePressEvent = true;
    QWidget::timerEvent(e);
}
