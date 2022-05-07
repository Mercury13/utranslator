#ifndef WIFIND_H
#define WIFIND_H

#include <QWidget>

namespace Ui {
class WiFind;
}

class WiFind : public QWidget
{
    Q_OBJECT
    using This = WiFind;
public:
    explicit WiFind(QWidget *parent = nullptr);
    ~WiFind();

    void startSearch(const QString& caption, size_t aCount);

    // index, 0-based
    size_t index0() const;

signals:
    void closed();
    void indexChanged(size_t index0);
private slots:
    void spinChanged();
public slots:
    void goBack();
    void goNext();
    void close();
    /// Sets search index w/o changing anything
    void setIndexQuietly(size_t index0);
private:
    Ui::WiFind *ui;
    size_t count = 1;
    bool isProgrammatic = false;
};

#endif // WIFIND_H
