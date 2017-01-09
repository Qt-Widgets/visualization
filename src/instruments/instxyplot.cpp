#include "instxyplot.h"

const QString InstXYPlot::TAG_NAME = "XY_PLOT";

void InstXYPlot::renderAxis(QPainter* painter)
{
    int dx = cWidth / cMajorCnt;
    int dy = cHeight / cMajorCnt;
    int x = 0;
    int y = 0;
    for (int i=0; i<cMajorCnt + 1; ++i)
    {
        painter->drawLine(x, mCenterY-cMajorLen, x, mCenterY+cMajorLen);
        painter->drawLine(mCenterX-cMajorLen, y, mCenterX+cMajorLen, y);
        x += dx;
        y += dy;
    }
}

void InstXYPlot::renderBall(QPainter* painter)
{
    setPen(painter, cColorStatic);
    setBrush(painter, cColorForeground);
    QRect rect(mLastValX * (cWidth-cBallSize), mLastValY * (cHeight-cBallSize), cBallSize, cBallSize);
    painter->drawEllipse(rect);
}

void InstXYPlot::renderStatic(QPainter *painter)
{
    setPen(painter, cColorStatic);
    clear(painter);

    mCenterX = cWidth / 2;
    mCenterY = cHeight / 2;

    painter->drawLine(0, mCenterY, cWidth, mCenterY);
    painter->drawLine(mCenterX, 0, mCenterX, cHeight);

    renderAxis(painter);

    painter->drawText(0, mCenterY-cMajorLen, cLabelX);
    painter->drawText(mCenterX+cMajorLen, cHeight, cLabelY);
}

void InstXYPlot::renderDynamic(QPainter *painter)
{
    if (mSignal->getId() == cSignalId)
    {
        mLastValX = mSignal->getNormalizedValue();   // primary signal shown on X axis
    }
    else
    {
        mLastValY = mSignal->getNormalizedValue();   // additional signal shown on Y axis
    }

    renderBall(painter);
}

quint16 InstXYPlot::getSignalYId()
{
    return cSignalYId;
}
