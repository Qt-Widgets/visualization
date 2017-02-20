#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <QtGlobal>
#include <QWidget>
#include <QColor>
#include <QPixmap>
#include <QMap>
#include <QPainter>
#include <QPointer>

#include "visuhelper.h"
#include "visusignal.h"
#include "visuwidget.h"


class VisuSignal;   // forward declare Signal class

class VisuInstrument : public VisuWidget
{
    Q_OBJECT

protected:

    quint16 cSignalId;          // associated signal id
    QColor cColorBackground;    // instrument background color
    QColor cColorStatic;        // color for nonchanging parts (scales, marks, etc)
    QColor cColorForeground;    // color for changing parts (pointers, indicators, etc)
    quint8 cFontSize;           // Size of font used on labels
    QString cFontType;

    // pixmaps
    QPixmap mPixmap;        // holds instrument rendered with last received signal value
    QPixmap mPixmapStatic;  // holds prerendered pixmap generated by renderStatic()

    bool    mFirstRun;
    const VisuSignal *mSignal; // Pointer to last signal that was updated

    void paintEvent(QPaintEvent* event);
    virtual void renderStatic(QPainter*) = 0;   // Renders static parts of instrument
    virtual void renderDynamic(QPainter*) = 0;  // Renders signal value dependent parts

    void setFont(QPainter* painter);
    void setPen(QPainter* painter, QColor color, int thickness = 1);
    void setBrush(QPainter* painter, QColor color);
    void clear(QPainter* painter);
    void setup();

    QVector<QPointer<VisuSignal>> connectedSignals;

public slots:
    void signalUpdated(const VisuSignal* const mSignal);
    void initialUpdate(const VisuSignal* const signal);

public:
    virtual bool refresh(const QString& key);

public:

    explicit VisuInstrument(QWidget *parent, QMap<QString, QString> properties)
        : VisuWidget(parent, properties)
    {
        loadProperties(properties);
    }

    virtual void loadProperties(QMap<QString, QString> properties);
    void connectSignals();
    void disconnectSignals();
    void initializeInstrument();

    // Getters
    quint16 getSignalId();
    quint16 getId();
    void render();
};

#endif // INSTRUMENT_H
