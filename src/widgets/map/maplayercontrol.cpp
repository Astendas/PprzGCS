#include "maplayercontrol.h"
#include <QPixmap>
#include <QPainter>
#include <QPaintEvent>


MapLayerControl::MapLayerControl(QString name, QPixmap pixmap, bool initialState, int z, QWidget *parent) : QWidget(parent),
    _name(name), showState(initialState), z_value(z)
{
    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(0);

    nameLabel = new QLabel(name);
    verticalLayout->addWidget(nameLabel);
    verticalLayout->setAlignment(nameLabel, Qt::AlignHCenter);
    nameLabel->setMargin(5);

    imageLabel = new QLabel(this);
    verticalLayout->addWidget(imageLabel);
    verticalLayout->setAlignment(imageLabel, Qt::AlignHCenter);
    imageLabel->setPixmap(pixmap);

    QIcon icon(":/pictures/displayed.svg");
    if(showState) {
        icon = QIcon(":/pictures/hidden.svg");
    }

    show_button = new ImageButton(icon, QSize(60, 60), true, imageLabel);
    QVBoxLayout* image_layout = new QVBoxLayout(imageLabel);
    image_layout->addWidget(show_button);
    image_layout->setAlignment(show_button, Qt::AlignHCenter);
    connect(
        show_button, &QPushButton::clicked,
        [=]() {
            toggleShowState();
        });

    opacitySlider = new QSlider(Qt::Horizontal, this);
    opacitySlider->setValue(opacitySlider->maximum());
    verticalLayout->addWidget(opacitySlider);
    //verticalLayout->setAlignment(opacitySlider, Qt::AlignHCenter);
    //opacitySlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(
        opacitySlider, &QSlider::valueChanged,
        [=](int value) {
            (void) value;
            emit(layerOpacityChanged(opacity()));
        });

}


void MapLayerControl::setShowState(bool state) {
    showState = state;
    if(showState) {
        show_button->setNormalIcon(QIcon(":/pictures/hidden.svg"));
    } else {
        show_button->setNormalIcon(QIcon(":/pictures/displayed.svg"));
    }
}

void MapLayerControl::setOpacitySlider(qreal opacity) {
    opacity = qMax(0., qMin(opacity, 1.));
    int value = static_cast<int>(opacity * opacitySlider->maximum());
    opacitySlider->setValue(value);
}

void MapLayerControl::toggleShowState() {
    if(showState) {
        setShowState(false);
    } else {
        setShowState(true);
    }
    emit(showLayer(showState));
}

void MapLayerControl::setZValue(int z) {
    z_value = z;
    emit(zValueChanged(z));
}

