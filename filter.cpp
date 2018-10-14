#include "filter.h"
#include <QDebug>
#include <QtMath>
#include <stdio.h>
QImage Filter::crazyFilter(int filterParam, const QImage &originalImage)
{

    int width = originalImage.width();
    int height = originalImage.height();
    QImage resultImage(width, height, originalImage.format());

    for(int i = 0; i < height; ++i)
        for(int j = 0; j < width; ++j) {

            if((j < width ) && (i < height)) {
                QRgb pixelColor = originalImage.pixel(j,i);

                QColor color(pixelColor);

                color.setRed((color.red() + filterParam ) & 0xFF);
                color.setGreen((color.green() + filterParam) & 0xFF);
                color.setBlue((color.blue() + filterParam) & 0xFF);


                resultImage.setPixel(j, i, color.rgb());
            }


        }

    return resultImage;


}

QImage Filter::rotationTransform(int degrees, const QImage &originalImage, bool hasBilinearInterpolation)
{
    double angle = qDegreesToRadians(static_cast<double>(degrees));
    double cosAngle = qCos(angle);
    double sinAngle = qSin(angle);

    int width = originalImage.width();
    int height = originalImage.height();
    QImage resultImage(width, height, originalImage.format());
    resultImage.fill(Qt::black);




    int pivotX = width/2;
    int pivotY = height/2;


    for(int i = 0; i < height; ++i)
        for(int j = 0; j < width; j++) {
            double originalX = ((j-pivotX)*cosAngle) + ((i-pivotY)*sinAngle) + pivotX;
            double originalY = (-1*(j-pivotX)*sinAngle) + (cosAngle*(i-pivotY)) + pivotY;

            int transformX = j;// + pivotX;
            int transformY = i;// - pivotY;

            if(((originalX >= 0 && originalX < width) && (originalY >=0 && originalY < height)) &&
                    ((transformX >= 0 && transformX < width) && (transformY >=0 && transformY < height))) {

                if(hasBilinearInterpolation)
                    resultImage.setPixel(j, i, bilinearInterpolation(originalX, originalY, originalImage));
                else
                    resultImage.setPixel(j, i, originalImage.pixel(static_cast<int>(originalX), static_cast<int>(originalY)));

            }

        }

    return resultImage;

}

QRgb Filter::bilinearInterpolation(double x, double y, const QImage &originalImage)
{
     int px1 = static_cast<int>(x);
     int px2 = static_cast<int>(x) + 1;
     int py1 = static_cast<int>(y);
     int py2 = static_cast<int>(y) + 1;

     if((px1 < 0) || (px2 >= originalImage.width() )|| (py1 < 0) || (py2 >= originalImage.height()))
         return originalImage.pixel(static_cast<int>(x), static_cast<int>(y));

     // 4 cores dos pixels vizinhos
     const QColor p1Color = originalImage.pixel(px1, py1);
     const QColor p2Color = originalImage.pixel(px2, py1);
     const QColor p3Color = originalImage.pixel(px1, py2);
     const QColor p4Color = originalImage.pixel(px2, py2);

     int pointRedComponent = (p1Color.red() + p2Color.red() + p3Color.red() + p4Color.red())/4;
     int pointGreenComponent = (p1Color.green() + p2Color.green() + p3Color.green() + p4Color.green())/4;
     int pointBlueComponent = (p1Color.blue() + p2Color.blue() + p3Color.blue() + p4Color.blue())/4;

     return QColor(pointRedComponent, pointGreenComponent, pointBlueComponent).rgb();
}

QImage Filter::sobelFilter(const QImage &originalImage, int minThreshold, int maxThreshold)
{
    QImage grayImage = originalImage;
    QImage filteredImage(originalImage.width(), originalImage.height(), QImage::Format_Grayscale8);

    int sobelX[3][3]{{-1,0,1}, {-2,0,2}, {-1,0,1}};
    int sobelY[3][3] = {{-1,-2,-1}, {0,0,0},{1,2,1}};

    int width = grayImage.width();
    int height = grayImage.height();

    for(int y = 1; y < height -2; ++y)
        for(int x = 1; x < width -2; ++x) {

            int xn1yn1 = qGray((grayImage.pixel(x-1,y-1)));
            int x0yn1 = qGray((grayImage.pixel(x,y-1)));
            int x1yn1 = qGray((grayImage.pixel(x+1,y-1)));
            int xn1y0 = qGray((grayImage.pixel(x-1,y)));
            int x0y0 = qGray((grayImage.pixel(x,y)));
            int x1y0 = qGray((grayImage.pixel(x+1,y)));
            int xn1y1 = qGray((grayImage.pixel(x-1,y+1)));
            int x0y1 = qGray((grayImage.pixel(x,y+1)));
            int x1y1 = qGray((grayImage.pixel(x+1,y+1)));


            int pixelX = (sobelX[0][0] * xn1yn1) + (sobelX[0][1] * x0yn1) + (sobelX[0][2] * x1yn1) +
                         (sobelX[1][0] * xn1y0)   + (sobelX[1][1] * x0y0)   + (sobelX[1][2] * x1y0) +
                         (sobelX[2][0] * xn1y1) + (sobelX[2][1] * x0y1) + (sobelX[2][2] * x1y1);

            int pixelY = (sobelY[0][0] * xn1yn1) + (sobelY[0][1] * x0yn1) + (sobelY[0][2] * x1yn1) +
                         (sobelY[1][0] * xn1y0)   + (sobelY[1][1] * x0y0)   + (sobelY[1][2] * x1y0) +
                         (sobelY[2][0] * xn1y1) + (sobelY[2][1] * x0y1) + (sobelY[2][2] * x1y1);

            int pixel = qCeil(qSqrt(pixelX*pixelX + pixelY*pixelY));

            if(pixel > maxThreshold)
                pixel = 255;

            if(pixel < minThreshold)
                pixel = 0;

            filteredImage.setPixel(x,y, qRgb(pixel, pixel, pixel));
        }

    return filteredImage;
}

QImage Filter::grayBlurFilter(const QImage &originalImage)
{
    QImage grayImage = originalImage;
    QImage filteredImage(originalImage.width(), originalImage.height(), QImage::Format_Grayscale8);

    int width = grayImage.width();
    int height = grayImage.height();

    for(int y = 1; y < height -2; ++y)
        for(int x = 1; x < width -2; ++x) {

            int xn1yn1 = qGray((grayImage.pixel(x-1,y-1)));
            int x0yn1 = qGray((grayImage.pixel(x,y-1)));
            int x1yn1 = qGray((grayImage.pixel(x+1,y-1)));
            int xn1y0 = qGray((grayImage.pixel(x-1,y)));
            int x0y0 = qGray((grayImage.pixel(x,y)));
            int x1y0 = qGray((grayImage.pixel(x+1,y)));
            int xn1y1 = qGray((grayImage.pixel(x-1,y+1)));
            int x0y1 = qGray((grayImage.pixel(x,y+1)));
            int x1y1 = qGray((grayImage.pixel(x+1,y+1)));

            int pixel = xn1yn1 +x0yn1 +x1yn1 +xn1y0 +x0y0 +x1y0 +xn1y1 + x0y1 + x1y1;

            pixel/=9;
            filteredImage.setPixel(x,y, qRgb(pixel, pixel, pixel));
        }

    return filteredImage;

}

QImage Filter::prewittFilter(const QImage &originalImage, int minThreshold, int maxThreshold)
{
    QImage grayImage = originalImage;//.convertToFormat(QImage::Format_Grayscale8);
    QImage filteredImage(originalImage.width(), originalImage.height(), QImage::Format_Grayscale8);

    int prewittX[3][3]{{-1,0,1}, {-1,0,1}, {-1,0,1}};
    int prewittY[3][3] = {{-1,-1,-1}, {0,0,0},{1,1,1}};

    int width = grayImage.width();
    int height = grayImage.height();

    for(int y = 1; y < height -2; ++y)
        for(int x = 1; x < width -2; ++x) {

            int xn1yn1 = qGray((grayImage.pixel(x-1,y-1)));
            int x0yn1 = qGray((grayImage.pixel(x,y-1)));
            int x1yn1 = qGray((grayImage.pixel(x+1,y-1)));
            int xn1y0 = qGray((grayImage.pixel(x-1,y)));
            int x0y0 = qGray((grayImage.pixel(x,y)));
            int x1y0 = qGray((grayImage.pixel(x+1,y)));
            int xn1y1 = qGray((grayImage.pixel(x-1,y+1)));
            int x0y1 = qGray((grayImage.pixel(x,y+1)));
            int x1y1 = qGray((grayImage.pixel(x+1,y+1)));


            int pixelX = (prewittX[0][0] * xn1yn1) + (prewittX[0][1] * x0yn1) + (prewittX[0][2] * x1yn1) +
                         (prewittX[1][0] * xn1y0)   + (prewittX[1][1] * x0y0)   + (prewittX[1][2] * x1y0) +
                         (prewittX[2][0] * xn1y1) + (prewittX[2][1] * x0y1) + (prewittX[2][2] * x1y1);

            int pixelY = (prewittY[0][0] * xn1yn1) + (prewittY[0][1] * x0yn1) + (prewittY[0][2] * x1yn1) +
                         (prewittY[1][0] * xn1y0)   + (prewittY[1][1] * x0y0)   + (prewittY[1][2] * x1y0) +
                         (prewittY[2][0] * xn1y1) + (prewittY[2][1] * x0y1) + (prewittY[2][2] * x1y1);

            int pixel = qCeil(qSqrt(pixelX*pixelX + pixelY*pixelY));

            if(pixel > maxThreshold)
                pixel = 255;

            if(pixel < minThreshold)
                pixel = 0;

            filteredImage.setPixel(x,y, qRgb(pixel, pixel, pixel));
        }

    return filteredImage;

}
