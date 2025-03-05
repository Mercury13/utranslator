#ifndef BALLOONTIP_H
#define BALLOONTIP_H

#include <QWidget>
#include <QMessageBox>
#include <QPixmap>
#include <QTimerEvent>

enum class BalloonDir : unsigned char {
    // Just clock face: balloon faces 12 o’clock
    BLN_1_OC = 1, BLN_2_OC, BLN_3_OC, BLN_4_OC, BLN_5_OC, BLN_6_OC,
    BLN_7_OC, BLN_8_OC, BLN_9_OC, BLN_10_OC, BLN_11_OC, BLN_12_OC,
    // Balloon faces 12 o’clock → arrow faces 6 o’clock
    ARR_1_OC  = BLN_7_OC,  ARR_2_OC  = BLN_8_OC,  ARR_3_OC  = BLN_9_OC,
    ARR_4_OC  = BLN_10_OC, ARR_5_OC  = BLN_11_OC, ARR_6_OC  = BLN_12_OC,
    ARR_7_OC  = BLN_1_OC,  ARR_8_OC  = BLN_2_OC,  ARR_9_OC  = BLN_3_OC,
    ARR_10_OC = BLN_4_OC,  ARR_11_OC = BLN_5_OC,  ARR_12_OC = BLN_6_OC,
};

class BalloonTip : public QWidget
{
    Q_OBJECT
public:
    // Turns relative direction to absolute, and vice-versa
    static BalloonDir flipDir(BalloonDir x);
    static void showBalloon(QMessageBox::Icon icon, const QString& title,
                            const QString& msg,
                            const QPoint& pos, int timeout,
                            BalloonDir arrowDir = BalloonDir::BLN_12_OC);
    static void hideBalloon();
    static bool isBalloonVisible();
    static void updateBalloonPosition(const QPoint& pos);

private:
    static void showBalloonAbsDir(QMessageBox::Icon icon, const QString& title,
                            const QString& msg,
                            const QPoint& pos, int timeout,
                            BalloonDir arrowDir = BalloonDir::BLN_12_OC);
    BalloonTip(QMessageBox::Icon icon, const QString& title,
                const QString& msg);
    ~BalloonTip();
    void balloonAbsDir(const QPoint&, int, BalloonDir arrowDir);

protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE;

private:
    QPixmap pixmap;
    QWidget* anyWidget;
    int timerId;
    int eventTimerId;
    bool enablePressEvent;
    BalloonDir arrowDir;
};

#endif // BALLOONTIP_H
