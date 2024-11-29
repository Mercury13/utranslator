#include "FmTranslateWithOriginal.h"
#include "ui_FmTranslateWithOriginal.h"

FmTranslateWithOriginal::FmTranslateWithOriginal(QWidget *parent) :
    Super(parent),
    ui(new Ui::FmTranslateWithOriginal)
{
    ui->setupUi(this);
}

FmTranslateWithOriginal::~FmTranslateWithOriginal()
{
    delete ui;
}

std::optional<tr::tw::Sets> FmTranslateWithOriginal::exec(int)
{
    if (!Super::exec())
        return std::nullopt;
    /// @todo [urgent] exec
}
