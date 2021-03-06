#include "wysiwyg/editconfiguration.h"
#include "wysiwyg/visupropertieshelper.h"
#include "visuconfigloader.h"

void EditConfiguration::setup(QPointer<VisuConfiguration> configuration)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Configuration parameters"));

    resize(mWidth, mHeight);

    mConfiguration = configuration;

    QLayout* vlayout = new QVBoxLayout();
    setLayout(vlayout);

    mTable = new QTableWidget();
    VisuPropertiesHelper::updateTable(mTable,
                          mConfiguration->getProperties(),
                          mConfiguration->getPropertiesMeta(),
                          std::make_pair(this, SLOT(propertyChange())));
    mTable->setMaximumWidth(mWidth);
    mTable->verticalHeader()->hide();
    mTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    vlayout->addWidget(mTable);

    QWidget* buttons = new QWidget();
    QLayout* buttonsLayout = new QHBoxLayout();
    buttons->setLayout(buttonsLayout);

    QPushButton* saveButton = new QPushButton(tr("&Save"));
    buttonsLayout->addWidget(saveButton);

    QPushButton* cancelButton = new QPushButton(tr("&Cancel"));
    buttonsLayout->addWidget(cancelButton);

    buttons->setMaximumHeight(saveButton->height());
    vlayout->addWidget(buttons);

    show();
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(updateConfigParams()));
    connect(mTable, SIGNAL(cellChanged(int,int)), this, SLOT(cellUpdated(int,int)));
}

void EditConfiguration::updateConfigParams()
{
    emit(configParamsUpdated());
    close();
}

void EditConfiguration::cellUpdated(int row, int col)
{
    (void)col;
    QString key = VisuPropertiesHelper::getKeyString(mTable, row);
    QString value = VisuPropertiesHelper::getValueString(mTable, row);
    mConfiguration->updateProperties(key, value);
    VisuPropertiesHelper::updateWidgetsState(mTable,
                                             mConfiguration->getProperties(),
                                             mConfiguration->getPropertiesMeta());
}

void EditConfiguration::propertyChange()
{
    int row = VisuPropertiesHelper::updateWidgetProperty(sender(), this);
    cellUpdated(row, 1);
}
