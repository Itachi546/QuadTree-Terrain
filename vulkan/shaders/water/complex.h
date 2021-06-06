struct complex
{
    float r;
    float im;
};

complex conjugate(complex v)
{
    complex result = complex(v.r, -v.im);
    return result;
}

complex mul(complex c0, complex c1)
{
    complex result;
    result.r = c0.r * c1.r - c0.im * c1.im;
    result.im = c0.r * c1.im + c0.im * c1.r;
    return result;
}

complex add(complex c0, complex c1)
{
    complex result;
    result.r = c0.r + c1.r;
    result.im = c0.im + c1.im;
    return result;
}


