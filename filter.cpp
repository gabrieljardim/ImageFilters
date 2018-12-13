#include "filter.h"
#include <QDebug>
#include <QtMath>
#include <stdio.h>
#include "fastfouriertransform.h"

enum FourierType {
    LOW_PASS,
    HIGH_PASS,
    BAND_PASS
};

static void bandPassFilter(Complex** complex2DArray, int width, int height, double minRadius, double maxRadius)
{
    for(int i = 0; i < height; ++i)
        for(int j = 0; j < width; ++j){
            //d(u,v) = [(u - M/2)^2 + (v - N/2)^2]^1/2
            double duv = qSqrt((j - width/2)*(j - width/2) + (i - height/2)*(i - height/2));

            if(duv < minRadius || duv > maxRadius) {
                complex2DArray[i][j].real = 0.0;
                complex2DArray[i][j].imag = 0.0;
            }
        }
}

static void lowPassFilter(Complex** complex2DArray, int width, int height, double radius)
{
    for(int i = 0; i < height; ++i)
        for(int j = 0; j < width; ++j){
            //d(u,v) = [(u - M/2)^2 + (v - N/2)^2]^1/2
            double duv = qSqrt((j - width/2)*(j - width/2) + (i - height/2)*(i - height/2));

            if(duv >= radius) {
                complex2DArray[i][j].real = 0.0;
                complex2DArray[i][j].imag = 0.0;
            }
        }
}

static void highPassFilter(Complex** complex2DArray, int width, int height, double radius)
{
    for(int i = 0; i < height; ++i)
        for(int j = 0; j < width; ++j){
            //d(u,v) = [(u - M/2)^2 + (v - N/2)^2]^1/2
            double duv = qSqrt((j - width/2)*(j - width/2) + (i - height/2)*(i - height/2));

            if(duv < radius) {
                complex2DArray[i][j].real = 0.0;
                complex2DArray[i][j].imag = 0.0;
            }
        }

}

static QImage convertComplex2dArrayToMagnitudeImage(Complex** complex2dArray, int width, int height) {

    double max = 0;

    for(int i = 0; i < height; ++i)
        for(int j = 0; j < width; ++j) {
            Complex cNum;
            cNum.real = complex2dArray[i][j].real;
            cNum.imag = complex2dArray[i][j].imag;

             double mag = qSqrt(cNum.real * cNum.real + cNum.imag * cNum.imag);
            if(mag > max)
                max = mag;
    }

    double c = 255/(qLn(1 + max));
    qDebug() << "MAX: " << max;
    qDebug() << "C: " << c;

    QImage outputImage(width, height, QImage::Format_Grayscale8);


    for(int i = 0; i < height; ++i)
        for(int j = 0; j < width; ++j) {

            double real = complex2dArray[i][j].real;
            double imag = complex2dArray[i][j].imag;
            double mag = qSqrt(real * real + imag * imag) * 150;

            double gray = c * qLn(1 + mag);

            int grayPixel = static_cast<int>(gray);

            outputImage.setPixel(j, i, qRgb(grayPixel, grayPixel, grayPixel));
        }

    return outputImage;
}

static Complex** convertQImageToCenteredComplex2dArray(const QImage& image)
{

    int width = image.width();
    int height = image.height();


    Complex* complexPtr = new Complex[width*height*sizeof(Complex)];

    Complex** rowComplexPtr = new Complex*[height*sizeof(Complex*)];

    for(int i = 0; i < height; ++i)
        rowComplexPtr[i] = &complexPtr[i*width];

    for(int i = 0; i < height; ++i)
        for(int j = 0; j < width; ++j) {
            int pixel = qGray(image.pixel(j,i));
            rowComplexPtr[i][j].real = pixel * qPow(-1, i+j);
            rowComplexPtr[i][j].imag = 0;

        }

    return rowComplexPtr;

}

static QImage convertCenteredComplex2dArrayToQImage(Complex** complex2dArray, int width, int height)
{
    QImage image(width, height, QImage::Format_Grayscale8);

    for(int i = 0; i < height; ++i)
        for(int j = 0; j < width; ++j) {
            int grayValue = static_cast<int>(complex2dArray[i][j].real* qPow(-1, i+j));

            if(grayValue < 0)
                grayValue = 0;

            if(grayValue > 255)
                grayValue = 255;

            image.setPixel(j, i, qRgb(grayValue, grayValue, grayValue));
        }

    return image;
}

static void deleteComplex2dArray(Complex** complex2dArray)
{
    delete complex2dArray[0];
    delete complex2dArray;

}

static QImage fourierPassFilter(const QImage &originalImage, double radius1, double radius2, FourierType filterType)
{
    int originalWidth = originalImage.width();
    int originalHeight = originalImage.height();
    int maxDim = qMax(originalWidth, originalHeight);
    int nextPowerOfTwo = static_cast<int>(qNextPowerOfTwo(static_cast<quint32>(maxDim-1))); //maxDim -1  is a fast workaround to handle cases where maxDim is already a power of two

    QImage scaledImage = originalImage.scaled(nextPowerOfTwo, nextPowerOfTwo);
    int width = scaledImage.width();
    int height = scaledImage.height();

    Complex** complex2dArray = convertQImageToCenteredComplex2dArray(scaledImage);

    FFT2D(complex2dArray, width, height, 1);

    //transform de radius percentage relative to the image where max radius equal to the square diagonal l/2 * 2^(1/2)
    //Given that the filter is not linea qPow is used to improve the feeling of horizontal bar movement as deltaY has a small variation between 0..1
    radius1 = qPow(radius1/100.0, 3) * (nextPowerOfTwo/2) * qSqrt(2.0);
    radius2 = qPow(radius2/100.0, 3) * (nextPowerOfTwo/2) * qSqrt(2.0);


    switch(filterType)
    {
    case LOW_PASS:
        lowPassFilter(complex2dArray, width, height, radius1);
        break;
    case HIGH_PASS:
        highPassFilter(complex2dArray, width, height, radius1);
        break;
    case BAND_PASS:
        bandPassFilter(complex2dArray, width, height, radius1, radius2);
        break;
    }
    FFT2D(complex2dArray, width, height, -1);

    QImage transformed = convertCenteredComplex2dArrayToQImage(complex2dArray, width, height).scaled(originalWidth, originalHeight);
    deleteComplex2dArray(complex2dArray);

    return transformed;

}

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

            if(pixel > 255)
                pixel = 0;

            if(pixel < minThreshold || pixel > maxThreshold)
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

            if(pixel > 255)
                pixel = 0;

            if(pixel < minThreshold || pixel > maxThreshold)
                pixel = 0;

            filteredImage.setPixel(x,y, qRgb(pixel, pixel, pixel));
        }
    return filteredImage;
}

QImage Filter::lowPassFilter(const QImage &originalImage, double radius)
{
   return fourierPassFilter(originalImage, radius, 0, LOW_PASS);
}

QImage Filter::highPassFilter(const QImage &originalImage, double radius)
{
    return fourierPassFilter(originalImage, radius, 0, HIGH_PASS);
}

QImage Filter::bandPassFilter(const QImage &originalImage, double minRadius, double maxRadius)
{
    return fourierPassFilter(originalImage, minRadius, maxRadius, BAND_PASS);
}

QImage Filter::highPassFilterMagnitude(const QImage &originalImage, double radius)
{
    QImage scaledImage = originalImage.scaled(256,256);
    int width = scaledImage.width();
    int height = scaledImage.height();
    Complex** complex2dArray = convertQImageToCenteredComplex2dArray(scaledImage);

    FFT2D(complex2dArray, width, height, 1);
    highPassFilter(complex2dArray, width, height, radius);

    QImage transformed = convertComplex2dArrayToMagnitudeImage(complex2dArray, width, height);

    deleteComplex2dArray(complex2dArray);

    return transformed;
}
