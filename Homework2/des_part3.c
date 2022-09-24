void des_cfb_encrypt(unsigned char p[], int n, unsigned char des_seed_iv[], unsigned char des_iv[], unsigned char des_seed_key[])
{
    int i, j;
    unsigned char des_key[8];
    unsigned char iv[8];
    unsigned char _register[8];
    unsigned char temp;
    char ciphertext[0xff];

    //des_iv[i] = des_seed_iv[i] ^ RC4_KEY[i];
    //---->
    //des_iv[i] ^ des_seed_iv[i] = des_seeed_iv[i] ^ des_seed_iv[i] ^ RC4_KEY[i];
    //                           = 0 ^ RC4_KEY[i]
    //                           = RC4_KEY[i]
    //des_key[i] = des_seed_key[i] ^ RC4_KEY[i];
    for(i = 0; i < 8; i++)
        des_key[i] = des_seed_key[i] ^ des_iv[i] ^ des_seed_iv[i];

    memcpy(iv, des_iv, 8);
    //循环加密
    for(i = 0; i < n; i++)
    {
        //将向量送入寄存器中
        memcpy(_register, iv, 8);
        //对寄存器加密
        des_encrypt(_register, des_key);

        //与明文单元异或
        temp = p[i];
        temp ^= _register[0];
        //送入密文
        ciphertext[i] = temp;

        //将寄存器恢复为原向量并移位
        memcpy(_register, iv, 8);
        for(j = 0; j < 7; j++)
            _register[j] = _register[j + 1];
        //将新产生的密文送入寄存器
        _register[7] = temp;
        //产生新的向量
        memcpy(iv, _register, 8);
    }

    for(int i = 0; i < n; i++)
        printf("%02X", ciphertext[i] & 0xff);
    return;
}
