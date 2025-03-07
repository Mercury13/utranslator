#ifndef FMDECODER_H
#define FMDECODER_H

#include <QDialog>

namespace Ui {
class FmDecoder;
}

class QstrObject;

class FmDecoder : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
    using This = FmDecoder;
public:
    explicit FmDecoder(QWidget *parent = nullptr);
    ~FmDecoder();
    bool exec(QstrObject* obj);
private slots:
    void decodeCpp();
    void decodeBr();
private:
    Ui::FmDecoder *ui;
    template <class Intermed, class Body> void decodeMemo(const Body& body);
    using Super::exec;
};

#endif // FMDECODER_H
