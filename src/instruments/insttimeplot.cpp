#include "insttimeplot.h"

const QString InstTimePlot::TAG_NAME = "TIME_PLOT";

int InstTimePlot::getFontHeight()
{
    QFont font;
    font.setPointSize(fontSize);
    QFontMetrics fm(font);
    return fm.height();
}

void InstTimePlot::setTimestampRect(int fontHeight)
{
    mTimestampRect.setX(width - 100);
    mTimestampRect.setWidth(90);
    mTimestampRect.setY(height - fontHeight);
    mTimestampRect.setHeight(fontHeight);
}

void InstTimePlot::init()
{
    int fontHeight = getFontHeight();
    setTimestampRect(fontHeight);
}

void InstTimePlot::setPen(QPainter* painter, QColor color)
{
    QPen pen;
    pen.setWidth(lineThickness);
    pen.setColor(color);
    painter->setPen(pen);
}

void InstTimePlot::setLabelMaxWidth(QPainter* painter)
{
    QFontMetrics font_metrics = painter->fontMetrics();
    int sig_cur = mSignal->getMin();
    int label_width;
    QString label;
    mMaxLabelWidth = 0;
    for (int i=0; i<=majorCnt; ++i) {
        label = getLabel(sig_cur);
        sig_cur += mSigStep;
        label_width = font_metrics.width(label);
        mMaxLabelWidth = mMaxLabelWidth < label_width ? label_width : mMaxLabelWidth;
    }
}

QString InstTimePlot::getLabel(double value)
{
    return QString::number(value, 'f', decimals);
}

void InstTimePlot::renderLabel(QPainter* painter, double sigCur, qint32 yPos)
{
    QString label = getLabel(sigCur);
    QFontMetrics fontMetrics = painter->fontMetrics();
    int labelWidth = fontMetrics.width(label);
    int labelHeight = fontMetrics.height();
    painter->drawText(mMaxLabelWidth - labelWidth, yPos + labelHeight / 2, label);
}

quint16 InstTimePlot::renderLabelsAndMajors(QPainter* painter)
{
    double sigCur = mSignal->getMin();
    qint32 yPos = mPlotStartY;
    quint16 yStep = (height - 2 * mMargin) / majorCnt;

    for (int i=0; i<=majorCnt; ++i)
    {
        renderLabel(painter, sigCur, yPos);
        painter->drawLine(mMaxLabelWidth + mMargin / 2, yPos, width - mMargin, yPos);
        yPos -= yStep;
        sigCur += mSigStep;
    }

    return yPos + yStep;
}

void InstTimePlot::renderStatic(QPainter* painter)
{
    painter->fillRect(0, 0, width, height, colorBackground);

    setPen(painter, colorStatic);
    setFont(painter, fontSize);

    // make sure we make enough space so bottom line does not hit the timestamp display
    mMargin = mTimestampRect.height() + 2;
    mSigStep = (mSignal->getMax() - mSignal->getMin()) / (majorCnt);

    setLabelMaxWidth(painter);

    mPlotStartX = mMargin + mMaxLabelWidth;
    mPlotStartY = height - mMargin;
    mPlotEndX = width - mMargin;
    mPlotEndY = renderLabelsAndMajors(painter);
    mPlotRangeX = mPlotEndX - mPlotStartX;
    mPlotRangeY = mPlotStartY - mPlotEndY;

    // render signal name
    painter->drawText(width/2,
                      mPlotEndY-PADDING,
                      QString("%1 (%2)").arg(mSignal->getName()).arg(mSignal->getUnit()));

    // init values
    mLastUpdateX = mPlotStartX;
    mLastUpdateY = mPlotStartY;
    mLastMarkerTime = 0;

    mGraphPixmap = QPixmap(width, height);
    mGraphPixmap.fill(Qt::transparent);
    this->setAttribute(Qt::WA_TranslucentBackground);
    mGraphPainter = new QPainter(&mGraphPixmap);
    mGraphPainter->setRenderHint(QPainter::Antialiasing);
}

QString InstTimePlot::getDisplayTime(int ticks, QString format)
{
    return QTime(0, 0, 0).addSecs(ticks / ticksInSecond).toString(format);
}

void InstTimePlot::renderMarker(QPainter* painter, quint64 timestamp)
{
    int markerTime = timestamp - (timestamp % markerDt) + markerDt;   // round up
    double cor = ((double)markerTime - timestamp - markerDt) * mPlotRangeX / timespan;
    double markerX = mNewUpdateX + cor;

    if (markerX > mPlotStartX && markerX < mPlotEndX)
    {
        setPen(mGraphPainter, colorStatic);
        painter->drawLine(markerX, mPlotStartY, markerX, mPlotEndY);

        setFont(mGraphPainter, fontSize);
        painter->drawText(markerX,
                                height,
                                getDisplayTime(mLastMarkerTime, divisionFormat));
    }
    mLastMarkerTime = timestamp;
}

bool InstTimePlot::shouldRenderMarker(quint64 timestamp)
{
    return (timestamp >= mLastMarkerTime + markerDt);
}

void InstTimePlot::renderTimeLabel(QPainter* painter)
{
    setPen(painter, colorStatic);
    setFont(painter, fontSize);
    quint64 timestamp = mSignal->getTimestamp();
    painter->drawText(mPlotStartX,
                      mPlotEndY - 5,
                      "Time " + getDisplayTime(timestamp, masterTimeFormat));
}

void InstTimePlot::renderGraphSegment(QPainter* painter)
{
    setPen(mGraphPainter, colorForeground);
    mGraphPainter->drawLine(mLastUpdateX, mLastUpdateY, mNewUpdateX, mNewUpdateY);
    painter->drawPixmap(0, 0, mGraphPixmap);
}

void InstTimePlot::resetPlotToStart()
{
    mGraphPixmap.fill(Qt::transparent);
    mNewUpdateX = mPlotStartX + (mNewUpdateX - mLastUpdateX);
    mLastUpdateX = mPlotStartX;
}

bool InstTimePlot::noSpaceLeftOnRight()
{
    return (mNewUpdateX > mPlotEndX);
}

void InstTimePlot::renderDynamic(QPainter* painter)
{
    double value = mSignal->getNormalizedValue();
    quint64 timestamp = mSignal->getTimestamp();
    quint64 dt = timestamp > mLastUpdateTime ? (timestamp - mLastUpdateTime) : 0;
    double dx = mPlotRangeX * dt / (timespan);

    mLastUpdateTime = timestamp;
    mNewUpdateX = mLastUpdateX + dx;
    mNewUpdateY = mPlotStartY - mPlotRangeY * value;

    if (noSpaceLeftOnRight())
    {
        resetPlotToStart();
    }

    if (shouldRenderMarker(timestamp))
    {
        renderMarker(mGraphPainter, timestamp);
    }

    renderTimeLabel(painter);
    renderGraphSegment(painter);

    mLastUpdateX = mNewUpdateX;
    mLastUpdateY = mNewUpdateY;
}

