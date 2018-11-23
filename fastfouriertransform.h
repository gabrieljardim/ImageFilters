#ifndef FASTFOURIERTRANSFORM_H
#define FASTFOURIERTRANSFORM_H


struct Complex {
    double real;
    double imag;
};

bool FFT2D(Complex **c,int nx,int ny,int dir);

#endif // FASTFOURIERTRANSFORM_H
