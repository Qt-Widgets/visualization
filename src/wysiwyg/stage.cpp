#include "wysiwyg/stage.h"

#include <QMimeData>
#include "instruments/instanalog.h"
#include "visuconfigloader.h"
#include "wysiwyg/visuwidgetfactory.h"

void Stage::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain"))
        event->acceptProposedAction();
}

QString getType(QString mimeDataText)
{
    int delPos = mimeDataText.indexOf("|");
    return mimeDataText.mid(0, delPos);
}

void Stage::dropEvent(QDropEvent *event)
{

    QStringList parts = event->mimeData()->text().split("|");
    QString type = parts[0];
    QString origin = parts[1];

    VisuWidget* widget;
    if (mMainWindow->dragOriginIsToolbar(origin))
    {
        // TODO :: Refactor
        QString path = QString("system/%1.xml").arg(type);
        QString xmlString = VisuConfigLoader::loadXMLFromFile(path);
        QXmlStreamReader xmlReader(xmlString);
        widget = mMainWindow->getConfiguration()->createInstrumentFromToken(xmlReader, this);

        VisuSignal* signal = mMainWindow->getSignal();
        signal->initializeInstruments();

        connect(widget, SIGNAL(widgetActivated(VisuWidget*)), this, SLOT(activateWidget(VisuWidget*)));
    }
    else
    {
        widget = mMainWindow->getActiveWidget();
    }

    VisuWidget* sourceWidget = static_cast<VisuWidget*>(event->source());

    QPoint position = getNewWidgetPosition(event->pos(),
                                           sourceWidget->getRelativeOffset(),
                                           sourceWidget->size());

    widget->setPosition(position);

    mMainWindow->setActiveWidget(widget);

    event->acceptProposedAction();
}

QPoint Stage::getNewWidgetPosition(QPoint eventPos, QPoint grabOffset, QSize instSize)
{
    QPoint position = eventPos - grabOffset;

    // corect if horizontal position outside of container
    if (position.x() < 0)
    {
        position.setX(0);
    }
    else if (position.x() + instSize.width() > width())
    {
        position.setX(width() - instSize.width());
    }

    // corect if vertical position outside of container
    if (position.y() < 0)
    {
        position.setY(0);
    }
    else if (position.y() + instSize.height() > height())
    {
        position.setY(height() - instSize.height());
    }

    return position;
}

void Stage::activateWidget(VisuWidget* widget)
{
    mMainWindow->setActiveWidget(widget);
}

void Stage::paintEvent(QPaintEvent *event)
{
    (void)event;
    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
}
