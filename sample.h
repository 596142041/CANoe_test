#ifndef SAMPLE_H
#define SAMPLE_H

#include <QDialog>

namespace Ui {
        class sample;
    }

class sample : public QDialog
    {
        Q_OBJECT

    public:
        explicit sample(QWidget *parent = 0);
        ~sample();

    private:
        Ui::sample *ui;
    };

#endif // SAMPLE_H
