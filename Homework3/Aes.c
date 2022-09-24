void build_sbox_inverse(void)
{
    int i, j;
    for (i = 0; i < 16; ++i)
    {
        for (j = 0; j < 16; ++j)
        {
            // 根据sbox[i][j]的值，分理出对应项在sbox_inverse中的行列坐标
            int value = sbox[(i << 4) + j];
            int line = (value >> 4) & 0xf;
            int column = value & 0xf;
            // sbox_inverse对应坐标处的值前四位为i，后四位为j
            sbox_inverse[(line << 4) + column] = (i << 4) | j;
        }
    }
    return;
}


unsigned char aes_times(unsigned char x, unsigned char y)
{
    // 将传入拓展为GF(2^8)
    unsigned int a = x;
    unsigned int b = y;
    unsigned int result = 0;
    // 根据农夫算法计算乘法
    int i;
    for (i = 0; i < 8; i++)
    {
        // 判定b当前位是否有效
        if (b & (1 << i))
            result ^= a;
        
        // 左移a，等价于xtime()
        a <<= 1;
        // 根据AES标准，模为11B
        if (a & 0x100)
            a ^= 0x11B;
    }
    // 类型转换为unsigned char
    return (unsigned char)result;
}

void aes_polynomial_mul(unsigned char x[4], unsigned char y[4], unsigned char z[4])
{
    // temp临时储存结果
    // 这里有一个问题，我尝试将值直接写回z，但是遇到了错误，一直无法写入正确的值，所以只能退而求其次，用temp临时储存
    unsigned char temp[4] = {0};
    // x对应的拓展矩阵
    unsigned char matrix[4][4];
    int i, j;
    
    // 将向量x拓展为方阵matrix
    for (i = 0; i < 4; ++i)
        for (j = 0; j < 4; ++j)
            matrix[i][j] = x[(j - i + 7) % 4];
    
    // 矩阵乘法，得到结果
    for (i = 0; i < 4; ++i)
        for (j = 0; j < 4; ++j)
            temp[i] ^= aes_times(matrix[i][j], y[j]);
    
    // 将结果复制到向量z中
    memcpy(z, temp, 4);
    return;
}

void ByteSubInverse(unsigned char *p, int n)
{
    int i;
    // 我们只需要去sbox_inverse中查表即可
    for (i = 0; i < n; ++i)
        p[i] = sbox_inverse[p[i]];
    return;
}

void ShiftRowInverse(unsigned char *p)
{
    int i, j;
    unsigned char q[16];
    
    // 根据行号，循环移动
    for (i = 0; i < 4; ++i)
        for (j = 0; j < 4; ++j)
            *(q + (i << 2) + (i + j) % 4) = *(p + (i << 2) + j);

    memcpy(p, q, 16);
    return;
}

void MixColumnInverse(unsigned char *p, unsigned char a[4], int do_mul)
{
    // 临时向量
    unsigned char temp[4];
    // 最终结果
    unsigned char result[4][4];

    int i, j;
    for (i = 0; i < 4; ++i)
    {
        // 从p中取出一行进行矩阵乘法
        memcpy(temp, p + (i << 2), 4);
        // 根据do_mul判定是否需要乘法
        if (do_mul)
            aes_polynomial_mul(a, temp, temp);
        // 将结果写入result
        for (j = 0; j < 4; ++j)
            result[j][i] = temp[j];
    }
    // 将结果写回p
    memcpy(p, result, 16);
    return;
}

void aes_decrypt(unsigned char *bufin, unsigned char *bufout, unsigned char *key)
{
    int i;
    // 解密算法中使用的是a_inverse，根据书上数据可知结构如下
    // 定义多项式0xB * X^3 + 0xD * X^2 + 0x9 * X + 0xE
    unsigned char a_inverse[4] = {0x0b, 0x0d, 0x09, 0x0e};
    
    // 复制密文16字节到matrix
    unsigned char matrix[4][4];
    memcpy(matrix, bufin, 4 * 4);
    for (i = key_rounds; i >= 1; --i)
    {
        // 第1至key_rounds轮, 做以下步骤: AddRoundKey, MixColumnInverse, ShiftRowInverse, ByteSubInverse，与加密恰好完全相反
        AddRoundKey((unsigned char *)matrix, key + i * (4 * 4));
        if (i != key_rounds)
            MixColumnInverse((unsigned char *)matrix, a_inverse, 1);
        else
            MixColumnInverse((unsigned char *)matrix, a_inverse, 0);
        ShiftRowInverse((unsigned char *)matrix);
        MixColumn((unsigned char *)matrix, a_inverse, 0);
        ByteSubInverse((unsigned char *)matrix, 16);
    }
    // 第0轮只做AddRoundKey()
    AddRoundKey((unsigned char *)matrix, key);
    // 明文复制到bufout
    memcpy(bufout, matrix, 4 * 4);
    return;
}