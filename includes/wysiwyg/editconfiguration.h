#ifndef EDITCLASS_H
#define EDITCLASS_H

#include <QHeaderView>
#include <QWidget>
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QMap>
#include <QPointer>
#include "visuconfiguration.h"
#include "visumisc.h"

class EditConfiguration : public QWidget
{
    Q_OBJECT

public:
    EditConfiguration(QWidget* parent, QPointer<VisuConfiguration> configuration) : QWidget(parent)
    {
        setWindowFlags(Qt::Dialog);
        setup(configuration);
    }

signals:
    void configParamsUpdated();

public slots:
    void propertyChange();

private slots:
    void updateConfigParams();
    void cellUpdated(int row, int col);

private:
    static const int mWidth = 400;
    static const int mHeight = 400;

    QPointer<VisuConfiguration> mConfiguration;
    QPointer<QTableWidget> mTable;

    void setup(QPointer<VisuConfiguration> configuration);
};

#endif // EDITCLASS_H
