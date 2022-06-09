#ifndef CIRCLE_EYETRACK_H
#define CIRCLE_EYETRACK_H

#include <QWidget>

class CircleEyeTrack : public QWidget
{
    Q_OBJECT
public:
    explicit CircleEyeTrack(qreal radius, QWidget *parent = nullptr);
    void setBrush(QBrush b) {brush = b; update();}
    void setMinSize(QSize s) {minSize = s;}
    void setRadius(qreal r) {radius = r;}

    QSize minimumSizeHint() const override;

signals:

public slots:

protected:
    void paintEvent(QPaintEvent*) override;

private:
    QBrush brush;
    QSize minSize;
    qreal radius;
};

#endif // CIRCLEEYETRACK_H
