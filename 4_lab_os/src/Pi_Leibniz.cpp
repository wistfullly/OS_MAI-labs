extern "C" float Pi(int k)
{
    float res = 0;
    for (int i = 0; i < k; ++i)
    {
        int a = 1;
        if (i % 2 != 0)
            a = -1;
        res += (float(a) / (2.0 * float(i) + 1.0));
    }
    return 4 * res;
}