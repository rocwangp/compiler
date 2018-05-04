int main()
{
    int a = 0;
    float b = 1.0;
    int i = 0;
    for(i = 0; i != 5; i = i + 1)
    {
        b *= a;
        a = a + 1;
    }

    if(a > 3)
    {
        b -= 2;
    }
    else
    {
        b += 2;
    }

    int s = 0;
    s = s + 1;
    switch(s)
    {
        case 5:
            a = a + 1;
        case 10:
            a = a - 1;
        default:
            a = a * a;
    }
}
