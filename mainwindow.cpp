#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QLabel"
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QRgb>
#include <QFileDialog>
#include "filter.h"
#include <QBuffer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_useBilinearInterpolation(false),
    m_sobelMinThreshold(0),
    m_sobelMaxThreshold(255),
    m_prewittMinThreshold(0),
    m_prewittMaxThreshold(255),
    m_fourierOp(NotSelected)
{
    ui->setupUi(this);
    ui->radius2label->hide();
    ui->horizontalSlider_2->hide();
    this->showMaximized();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::crazyFilter()
{
    QImage image = ui->label->pixmap()->toImage();

    int width = image.width();
    int height = image.height();

    qDebug() << width;
    qDebug() << height;

    for(int i = 0; i < height; ++i)
        for(int j = 0; j < width; ++j) {

            if((j < width ) && (i < height)) {
                QRgb color = image.pixel(j,i);
                color/=7;
                image.setPixel(j, i, color);
            }


        }

    ui->label->setPixmap(QPixmap::fromImage(image));
    ui->label->show();

}

void MainWindow::on_anglelSlider_sliderMoved(int position)
{
   QImage rotatedImage = Filter::rotationTransform(position, m_modifiedImage, m_useBilinearInterpolation);

    ui->label->setPixmap(QPixmap::fromImage(rotatedImage));
    ui->label->show();
}

void MainWindow::on_checkBox_clicked(bool checked)
{
    m_useBilinearInterpolation = checked;
}


void MainWindow::on_sobelFilterButton_clicked()
{
    ui->label->setPixmap(QPixmap::fromImage(Filter::sobelFilter(m_modifiedImage, m_sobelMinThreshold, m_sobelMaxThreshold)));
    ui->label->show();

}

void MainWindow::on_prewittButton_clicked()
{
    ui->label->setPixmap(QPixmap::fromImage(Filter::prewittFilter(m_modifiedImage, m_prewittMinThreshold, m_prewittMaxThreshold)));
    ui->label->show();
}

void MainWindow::on_blurButton_clicked()
{
    ui->label->setPixmap(QPixmap::fromImage(Filter::grayBlurFilter(ui->label->pixmap()->toImage())));
    ui->label->show();

}

void MainWindow::on_anglelSlider_valueChanged(int value)
{
    on_anglelSlider_sliderMoved(value);
}

void MainWindow::on_saveButton_clicked()
{
    auto fileName = QFileDialog::getSaveFileName(this,
        tr("Open Image"), "", tr("Image Files (*.png)"));


    if(fileName.isEmpty()) {
        qDebug() << "Filename is empty";
        return;
    }

    QFile imageFile(fileName);

    imageFile.open(QIODevice::WriteOnly);
    QImage image = m_modifiedImage;

    image.save(&imageFile, "PNG");

}

void MainWindow::on_loadImageButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image files (*.png)"));

    if(fileName.isEmpty()) {
        qDebug() << "File is empty...";
        return;
    }

    if(!m_originalImage.load(fileName)) {
        qDebug() << "Failed on loading image. Exiting...";
        QCoreApplication::exit();
    }

    m_modifiedImage = m_originalImage;
    QPixmap img = QPixmap::fromImage(m_originalImage);
    ui->imageWidget->resize(img.width(), img.height());
    ui->label->setGeometry(0,0,img.width(), img.height());
    ui->label->setPixmap(img);
    show();

}

void MainWindow::on_resetButton_clicked()
{

    ui->anglelSlider->setValue(0);
    ui->horizontalSlider->setValue(0);
    ui->horizontalSlider_2->setValue(100);
    ui->spinBox->setValue(0);
    m_sobelMinThreshold = 0;
    m_sobelMaxThreshold = 255;
    m_prewittMinThreshold = 0;
    m_prewittMaxThreshold = 255;

    ui->sobelMinSpinBox->setValue(m_sobelMinThreshold);
    ui->sobelMaxSpinBox->setValue(m_sobelMaxThreshold);
    ui->prewittMinSpinBox->setValue(m_prewittMinThreshold);
    ui->prewittMaxSpinBox->setValue(m_prewittMaxThreshold);
    m_modifiedImage = m_originalImage;
    QPixmap img = QPixmap::fromImage(m_originalImage);
    ui->label->setPixmap(img);

    show();

}

void MainWindow::on_sobelMinSpinBox_valueChanged(int arg1)
{
    m_sobelMinThreshold = arg1;
    on_sobelFilterButton_clicked();
}

void MainWindow::on_sobelMaxSpinBox_valueChanged(int arg1)
{
    m_sobelMaxThreshold = arg1;
    on_sobelFilterButton_clicked();

}

void MainWindow::on_prewittMinSpinBox_valueChanged(int arg1)
{
    m_prewittMinThreshold = arg1;
    on_prewittButton_clicked();

}

void MainWindow::on_prewittMaxSpinBox_valueChanged(int arg1)
{
    m_prewittMaxThreshold = arg1;
    on_prewittButton_clicked();

}

void MainWindow::on_lowPassRadioButton_clicked()
{
    m_fourierOp = LowPass;
}

void MainWindow::on_highPassRadioButton_clicked()
{
    m_fourierOp = HighPass;
}

void MainWindow::on_bandPassRadioButton_clicked()
{
    m_fourierOp = BandPass;
}

void MainWindow::on_horizontalSlider_sliderReleased()
{
    int position = ui->horizontalSlider->value();

    switch (m_fourierOp) {
    case LowPass:
        ui->label->setPixmap(QPixmap::fromImage(Filter::lowPassFilter(m_modifiedImage, 100 - position)));
        ui->label->show();
        break;
    case HighPass:
        ui->label->setPixmap(QPixmap::fromImage(Filter::highPassFilter(m_modifiedImage, position)));
        ui->label->show();
        break;
    case BandPass:
        ui->label->setPixmap(QPixmap::fromImage(Filter::bandPassFilter(m_modifiedImage, position, ui->horizontalSlider_2->value())));
        ui->label->show();
        break;
    default:
        break;

    }
}

void MainWindow::on_horizontalSlider_2_sliderReleased()
{
    int position = ui->horizontalSlider_2->value();

    switch (m_fourierOp) {
    case BandPass:
        ui->label->setPixmap(QPixmap::fromImage(Filter::bandPassFilter(m_modifiedImage, ui->horizontalSlider->value(), position)));
        ui->label->show();
        break;
    default:
        break;
    }

}

void MainWindow::on_bandPassRadioButton_toggled(bool checked)
{
    if(checked) {
        ui->radius1label->setText("MinRadius");
        ui->radius2label->show();
        ui->horizontalSlider_2->show();

    }
    else {
        ui->radius1label->setText("Radius");
        ui->radius2label->hide();
        ui->horizontalSlider_2->hide();
    }
}

void MainWindow::on_applyButton_clicked()
{
    m_modifiedImage = ui->label->pixmap()->toImage();
}
