#include "WiFind.h"
#include "ui_WiFind.h"

WiFind::WiFind(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WiFind)
{
    ui->setupUi(this);
    connect(ui->btBack, &QAbstractButton::clicked, this, &This::goBack);
    connect(ui->btNext, &QAbstractButton::clicked, this, &This::goNext);
    connect(ui->btClose, &QAbstractButton::clicked, this, &This::close);
    connect(ui->spinIndex, &QSpinBox::valueChanged, this, &This::spinChanged);
    connect(ui->btAgain, &QAbstractButton::clicked, this, &This::repeated);
}

WiFind::~WiFind()
{
    delete ui;
}

void WiFind::goBack()
{
    if (isHidden())
        return;
    auto index1 = index0();
    ui->spinIndex->stepDown();
    auto index2 = index0();
    if (index2 != index1)
        emit indexChanged(index2);
}

void WiFind::goNext()
{
    if (isHidden())
        return;
    auto index1 = index0();
    ui->spinIndex->stepUp();
    auto index2 = index0();
    if (index2 != index1)
        emit indexChanged(index2);
}

void WiFind::close()
{
    if (!isHidden()) {
        hide();
        emit closed();
    }
}

void WiFind::startSearch(const QString& caption, size_t aCount)
{
    if (aCount == 0) {
        close();
    } else {
        ui->lbCaption->setText(caption);
        show();
        char buf[40];
        snprintf(buf, std::size(buf), "/%llu",
                 static_cast<unsigned long long>(aCount));
        ui->lbCount->setText(buf);
        isProgrammatic = true;
        count = aCount;
        ui->spinIndex->setMinimum(1);
        ui->spinIndex->setMaximum(count);
        ui->spinIndex->setValue(1);
        isProgrammatic = false;
        emit indexChanged(index0());
    }
}

size_t WiFind::index0() const
{
    auto i = ui->spinIndex->value() - 1;
    if (i < 0)
        return 0;   // count == 0 → still return 0!
    if (static_cast<size_t>(i) >= count)
        i = count - 1;
    return i;
}

void WiFind::setIndexQuietly(size_t index0)
{
    if (isHidden())
        return;
    if (index0 < count) {
        isProgrammatic = true;
        ui->spinIndex->setValue(index0 + 1);
        isProgrammatic = false;
    }
}


void WiFind::spinChanged()
{
    if (!isProgrammatic)
        emit indexChanged(index0());
}
