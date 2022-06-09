#include "circle_eyetrack.h"
#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>
#include <QDebug>
#include <math.h>

CircleEyeTrack::CircleEyeTrack(qreal radius,QWidget *parent) : QWidget(parent)
    ,brush(Qt::red),minSize(QSize(10, 10)),radius(radius)
{

}

void CircleEyeTrack::paintEvent(QPaintEvent *e) {
    (void)e;
    QPainter p(this);

    // Draw background
    p.setPen(Qt::NoPen);
    p.setBrush(brush);
    p.drawEllipse(radius,radius,radius,radius);
}

QSize CircleEyeTrack::minimumSizeHint() const
{
    return minSize;
}
