#ifndef FMNEW_H
#define FMNEW_H

#include <QDialog>

// Qt ex
#include "QtMultiRadio.h"

// Project
#include "TrDefines.h"

class QStackedWidget;

class WizardManager : public QObject
{
    Q_OBJECT
public:
    WizardManager(
            QStackedWidget* aStack, QWidget* aStartingPage,
            QAbstractButton* abtBack, QAbstractButton* abtNext,
            const QString& asFinish);
    void goNext(QWidget* newPage, bool hasNext);
    bool hasNext() const { return fHasNext; }
    bool hasBack() const { return !history.empty(); }
    size_t index() const { return history.size(); }
    QWidget* page() const;
public slots:
    void start();
    void goBack();
private:
    QStackedWidget* stack;
    QWidget* startingPage;
    QAbstractButton *btBack, *btNext;
    QString sNext, sFinish;
    std::vector<QWidget*> history;
    bool fHasNext = false;
    void updateButtons();
};

namespace Ui {
class FmNew;
}

class FmNew : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
    using This = FmNew;
public:
    explicit FmNew(QWidget *parent = nullptr);
    ~FmNew() override;
    std::unique_ptr<tr::PrjInfo> exec(int dummy=0);
protected:
private:
    Ui::FmNew *ui;
    EcRadio<tr::PrjType> radioType;

    std::unique_ptr<WizardManager> wizard;

    using Super::exec;
    void copyTo(tr::PrjInfo&);
private slots:
    void nextPressed();
};

#endif // FMNEW_H
