#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "visuapplication.h"
#include "instanalog.h"
#include "instdigital.h"
#include "instlinear.h"
#include "insttimeplot.h"
#include "instled.h"
#include "instxyplot.h"
#include "visuconfigloader.h"
#include "wysiwyg/visuwidgetfactory.h"
#include <QXmlStreamReader>
#include <QTableWidget>
#include "wysiwyg/stage.h"
#include <QLabel>

#include <QLineEdit>
#include <QTableWidgetItem>
#include <QtGui>
#include <QFileDialog>
#include "visumisc.h"

MainWindow::MainWindow(QString xmlPath, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupMenu();

    QWidget* window = new QWidget();
    QVBoxLayout* windowLayout = new QVBoxLayout();
    window->setLayout(windowLayout);
    setCentralWidget(window);

    mToolbar = new QWidget(window);
    mToolbar->setObjectName("toolbar");
    mToolbar->setMinimumHeight(170);
    windowLayout->addWidget(mToolbar);
    setupToolbarWidgets(mToolbar);

    QWidget* workArea = new QWidget(window);
    windowLayout->addWidget(workArea);

    QHBoxLayout* workAreaLayout = new QHBoxLayout();
    workArea->setLayout(workAreaLayout);

    mStage = new Stage(this, workArea);
    mStage->setObjectName("stage");
    workAreaLayout->addWidget(mStage);

    mPropertiesTable = new QTableWidget(workArea);
    mPropertiesTable->setMaximumWidth(300);
    mPropertiesTable->verticalHeader()->hide();
    mPropertiesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    workAreaLayout->addWidget(mPropertiesTable);


    mStage->setStyleSheet("background-color: gray;");
#if 0
    toolbar->setStyleSheet("background-color: green;");
    workArea->setStyleSheet("background-color: gray;");
#endif

    mConfiguration = new VisuConfiguration();
    mDefaultSignal = new VisuSignal(VisuConfigLoader::getTagFromFile("system/signal.xml", "signal"));
    addSignal(mDefaultSignal, true);

    setWindowState(Qt::WindowMaximized);
    show();
}

void MainWindow::openSignalsEditor()
{
    if (editSignalWindow != nullptr)
    {
        disconnect(editSignalWindow, SIGNAL(signalAdded(VisuSignal*)), this, SLOT(addSignal(VisuSignal*, bool)));
    }

    QAction* s = static_cast<QAction*>(sender());
    int signalId = s->data().toInt();
    VisuSignal* signal = nullptr;
    if (signalId >= 0)
    {
        signal = mConfiguration->getSignal(signalId);
    }
    editSignalWindow = new EditSignal(signal);
    connect(editSignalWindow, SIGNAL(signalAdded(VisuSignal*,bool)), this, SLOT(addSignal(VisuSignal*, bool)));
}

void MainWindow::openConfiguration()
{
    QString configPath = QFileDialog::getOpenFileName(this,
                                                      tr("Open configuration"),
                                                      ".",
                                                      "Configuration files (*.xml)");
    mConfiguration = new VisuConfiguration();
    QString xml = VisuConfigLoader::loadXMLFromFile(configPath);
    mConfiguration->loadFromXML(mStage, QString(xml));
    VisuMisc::setBackgroundColor(mStage, mConfiguration->getBackgroundColor());
    mStage->setGeometry(0, 0, mConfiguration->getWidth(), mConfiguration->getHeight());

    updateMenuSignalList();
}

void MainWindow::saveConfiguration()
{
    qDebug("Saving");
    // recreate widgets tree
    QList<VisuWidget*> widgets = mStage->findChildren<VisuWidget*>();

    for (VisuWidget* widget : widgets)
    {
        //qDebug("%d", widget->getProperties()["id"].toInt());
        mapToString(widget->getProperties());
    }
}

void MainWindow::mapToString(QMap<QString, QString> properties)
{
    QString xml;
    for (auto i = properties.begin(); i != properties.end(); ++i)
    {
        xml += "<" + i.key() + ">";
        xml += i.value();
        xml += "</" + i.key() + ">\n";
    }

    qDebug(xml.toStdString().c_str());
}

void MainWindow::setupMenu()
{
    QMenu* fileMenu = ui->menuBar->addMenu(tr("&File"));

    QAction* open = new QAction(tr("&Open"), this);
    open->setShortcut(QKeySequence::Open);
    open->setStatusTip(tr("Open existing configuration"));
    fileMenu->addAction(open);
    connect(open, SIGNAL(triggered()), this, SLOT(openConfiguration()));

    QAction* save = new QAction(tr("&Save"), this);
    save->setShortcut(QKeySequence::Save);
    save->setStatusTip(tr("Save configuration"));
    fileMenu->addAction(save);
    connect(save, SIGNAL(triggered()), this, SLOT(saveConfiguration()));

    QAction* saveAs = new QAction(tr("Save &as"), this);
    saveAs->setShortcut(QKeySequence::SaveAs);
    saveAs->setStatusTip(tr("Save as new configuration"));
    fileMenu->addAction(saveAs);

    QMenu* signalsMenu = ui->menuBar->addMenu(tr("&Signals"));

    QAction* newsig = new QAction(tr("&New"), this);
    newsig->setData(QVariant(-1));
    newsig->setStatusTip(tr("Add new signal"));
    signalsMenu->addAction(newsig);
    connect(newsig, SIGNAL(triggered()), this, SLOT(openSignalsEditor()));

    mSignalsListMenu = signalsMenu->addMenu(tr("&List"));
}

void MainWindow::updateMenuSignalList()
{
    mSignalsListMenu->clear();
    auto configSignals = mConfiguration->getSignals();
    if (configSignals.size() > 0)
    {
        for (VisuSignal* sig : configSignals)
        {
            if (sig != nullptr)
            {
                QMenu* tmpSig = mSignalsListMenu->addMenu(sig->getName());

                QAction* edit = new QAction(tr("&Edit"), this);
                edit->setData(QVariant(sig->getId()));
                tmpSig->addAction(edit);
                connect(edit, SIGNAL(triggered()), this, SLOT(openSignalsEditor()));

                QAction* del = new QAction(tr("&Delete"), this);
                del->setData(QVariant(sig->getId()));
                tmpSig->addAction(del);
                connect(del, SIGNAL(triggered()), this, SLOT(deleteSignal()));
            }
        }
    }
}

bool MainWindow::dragOriginIsToolbar(QString originObjectName)
{
    return originObjectName == mToolbar->objectName();
}

void MainWindow::setupToolbarWidgets(QWidget* toolbar)
{
    QHBoxLayout *layout = new QHBoxLayout;
    toolbar->setLayout(layout);

    mDefaultSignal = new VisuSignal(VisuConfigLoader::getTagFromFile("system/signal.xml", "signal"));

    layout->addWidget(VisuWidgetFactory::createWidget(this, InstAnalog::TAG_NAME, mDefaultSignal));
    layout->addWidget(VisuWidgetFactory::createWidget(this, InstLinear::TAG_NAME, mDefaultSignal));
    layout->addWidget(VisuWidgetFactory::createWidget(this, InstTimePlot::TAG_NAME, mDefaultSignal));
    layout->addWidget(VisuWidgetFactory::createWidget(this, InstDigital::TAG_NAME, mDefaultSignal));
    layout->addWidget(VisuWidgetFactory::createWidget(this, InstLED::TAG_NAME, mDefaultSignal));
    layout->addWidget(VisuWidgetFactory::createWidget(this, InstXYPlot::TAG_NAME, mDefaultSignal));

    mDefaultSignal->initialUpdate();
}

void MainWindow::addSignal(VisuSignal* signal, bool isNewSignal)
{
    if (isNewSignal)
    {
        mConfiguration->addSignal(signal);
    }

    updateMenuSignalList();
}

void MainWindow::deleteSignal()
{
    QAction* s = static_cast<QAction*>(sender());
    int signalId = s->data().toInt();
    if (signalId >= 0)
    {
        mConfiguration->deleteSignal(signalId);
        updateMenuSignalList();
    }
}

void MainWindow::cellUpdated(int row, int col)
{
    QString key = mPropertiesTable->item(row,0)->text();
    QString value = mPropertiesTable->item(row,1)->text();

    QMap<QString, QString> properties = mActiveWidget->getProperties();
    QPoint position = mActiveWidget->pos();

    if (key == "x")
    {
        position.setX(value.toInt());
    }
    else if (key == "y")
    {
        position.setY(value.toInt());
    }
    if (key == "signalId")
    {
        // find old signal and detach instrument
        VisuInstrument* inst = static_cast<VisuInstrument*>(mActiveWidget);
        mConfiguration->detachInstrumentFromSignal(inst);
    }

    properties[key] = value;
    mActiveWidget->load(properties);
    mActiveWidget->setPosition(position);

    if (properties["signalId"].toInt() >= 0)
    {
        // actual signal
        VisuInstrument* inst = static_cast<VisuInstrument*>(mActiveWidget);
        VisuSignal* signal = mConfiguration->getSignal(inst->getSignalId());
        if (key == "signalId")
        {
            mConfiguration->attachInstrumentToSignal(inst);
        }
        signal->initialUpdate();
    }
    else
    {
        mDefaultSignal->initialUpdate();
    }
}

void MainWindow::setActiveWidget(VisuWidget* widget)
{
    QMap<QString, QString> properties = widget->getProperties();
    mActiveWidget = widget;
    disconnect(mPropertiesTable, SIGNAL(cellChanged(int,int)), this, SLOT(cellUpdated(int,int)));
    VisuMisc::updateTable(mPropertiesTable, properties);
    connect(mPropertiesTable, SIGNAL(cellChanged(int,int)), this, SLOT(cellUpdated(int,int)));
}

VisuSignal* MainWindow::getSignal()
{
    return mDefaultSignal;
}

MainWindow::~MainWindow()
{
    delete ui;
}

#include "visuwidget.h"
#include <QMouseEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>

#define DEBUG_STR(EVENT) qDebug(EVENT": %s", objectName().toStdString().c_str());

void MainWindow::mousePressEvent(QMouseEvent * event)
{
    DEBUG_STR("MAINWINDOW-mousePressEvent");
    QWidget* widget = qApp->widgetAt(event->pos());
    if (widget->objectName() == VisuWidget::OBJECT_NAME)
    {
        VisuWidget* visuWidget = dynamic_cast<VisuWidget*>(widget);
        setActiveWidget(visuWidget);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    DEBUG_STR("MAINWINDOW-mouseReleaseEvent");
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
    DEBUG_STR("MAINWINDOW-mouseDoubleClickEvent");
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    DEBUG_STR("MAINWINDOW-mouseMoveEvent");
}
