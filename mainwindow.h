#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
class QGraphicsScene;
class QGraphicsView;
class QGraphicsPixmapItem;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum FourierOp {
        NotSelected,
        LowPass,
        HighPass,
        BandPass
    };

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void crazyFilter();
    ~MainWindow();

private slots:
    void on_anglelSlider_sliderMoved(int position);

    void on_checkBox_clicked(bool checked);

    void on_sobelFilterButton_clicked();

    void on_prewittButton_clicked();

    void on_blurButton_clicked();

    void on_anglelSlider_valueChanged(int value);

    void on_saveButton_clicked();

    void on_loadImageButton_clicked();

    void on_resetButton_clicked();

    void on_sobelMinSpinBox_valueChanged(int arg1);

    void on_sobelMaxSpinBox_valueChanged(int arg1);

    void on_prewittMinSpinBox_valueChanged(int arg1);

    void on_prewittMaxSpinBox_valueChanged(int arg1);

    void on_lowPassRadioButton_clicked();

    void on_highPassRadioButton_clicked();

    void on_bandPassRadioButton_clicked();

    void on_horizontalSlider_sliderReleased();

    void on_horizontalSlider_2_sliderReleased();

    void on_bandPassRadioButton_toggled(bool checked);

    void on_applyButton_clicked();

private:
    Ui::MainWindow *ui;
    QGraphicsScene* m_scene;
    QGraphicsView* m_view;
    QGraphicsPixmapItem* m_item;
    QImage m_originalImage;
    QImage m_modifiedImage;
    bool m_useBilinearInterpolation;
    int m_sobelMinThreshold;
    int m_sobelMaxThreshold;
    int m_prewittMinThreshold;
    int m_prewittMaxThreshold;
    FourierOp m_fourierOp;

};

#endif // MAINWINDOW_H
