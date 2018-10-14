#ifndef FILTER_H
#define FILTER_H

#include <QImage>

namespace Filter
{

QImage crazyFilter(int filterParam, const QImage& originalImage);
QImage sobelFilter(const QImage& originalImage, int minThreshold, int maxThreshold);
QImage prewittFilter(const QImage& originalImage, int minThreshold, int maxThreshold);
QImage rotationTransform(int angleDegrees, const QImage& originalImage, bool bilinearInterpolation = false);
QImage grayBlurFilter(const QImage& originalImage);
QRgb bilinearInterpolation(double x, double y, const QImage& originalImage);

};

#endif // FILTER_H
