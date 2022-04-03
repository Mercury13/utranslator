#ifndef FMDECODER_H
#define FMDECODER_H

#include <QDialog>

namespace Ui {
class FmDecoder;
}

class FmDecoder : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
    using This = FmDecoder;
public:
    explicit FmDecoder(QWidget *parent = nullptr);
    ~FmDecoder();
    int exec() override;
private slots:
    void decodeCpp();
private:
    Ui::FmDecoder *ui;
};

#endif // FMDECODER_H
