extern "C" float Pi(int k)
{
    float res = 1;
    for (int i = 1; i < k; ++i)
    {
        res *= (4.0 * float(i) * float(i)) / (4.0 * float(i) * float(i) - 1.0);
    }
    float m = float(k);
    float ost_chlen = ((2.0 * m) / (2.0 * m - 1.0)) * (((2.0 * m / (2.0 * m + 1.0)) * (1.0 / 4.0) + 1) + 3.0 / 4.0);

    return res * ost_chlen;
}