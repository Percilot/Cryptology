#include <stdio.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/sha.h>

char A[0x100], B[0x100], PE[0x100];   // ecc曲线参数a, b, p
char GX[0x100], GY[0x100], NE[0x100]; // 基点G的x坐标, y坐标, G的阶(均为256位)
char DE[0x100];                       // ecc的私钥d(256位)
char N1[2][0x100], D1[2][0x100], X2[2][0x100];
// N1存放用ecc公钥加密过的rsa参数N,
// 其中N1[0]存放第1部分密文, N1[1]存放第2部分密文;
// D1存放用ecc公钥加密过的rsa私钥d;
// X2存放用ecc公钥加密过的X1
char X1[0x100];            // X1存放用rsa公钥加密过的X
char X[0x100];             // X存放一个256位的随机数, X<N 且 X<NE
char N[0x100], D[0x100];   // rsa的N及私钥d(256位)
char RX[0x100], RY[0x100]; // ecc公钥R的x坐标及y坐标
char C[2][0x100];          // 存放用ecc公钥加密过的X, C[0]是第1部分密文, C[1]是第2部分密文
char S[2][0x100];          // 存放用ecnr签名过的RSA_private_encrypt_PKCS1_type_2(SHA1(X), D)
                           // 其中SHA1是散列算法, RSA_private_encrypt_PKCS1_type_2()是用RSA的
                           // 私钥d对SHA1(X)进行加密(实际上是签名), 加密前必须对SHA1(X)按PKCS1_type_2
                           // 方式进行填充, 使得SHA1(X)的长度从20字节变成0x20字节(即256位);
                           // 针对N为256位的PKCS1_type_2填充格式如下:
                           // 0x00, 0x02, 9字节非零随机数, 0x00, 20字节明文
                           // 归纳起来讲, S是对SHA1(X)的两次签名, 第1次是用rsa的私钥签名, 第2次是用ecc的私钥签名

EC_GROUP *Base;
BN_CTX *Ctx;

// 椭圆曲线参数a b p
BIGNUM *a, *b, *p;
// 基点G的坐标和阶数
BIGNUM *Gx, *Gy, *Gn;
// 公钥R的坐标
BIGNUM *Rx, *Ry;
// 临时点T的坐标
BIGNUM *Tx, *Ty;
// 私钥d
BIGNUM *d;

// ECC两段密文
BIGNUM *r, *s;

// 临时变量
BIGNUM *temp0, *temp1;
BIGNUM *tempN, *tempD, *tempX1;

// 基点G 公钥点R 临时点T
EC_POINT *G, *R, *T;

// RSA加密
RSA *myrsa;

void GetInput();
void InitGCC();
void SolveND();
void SolveX();
void EncryptX();
void SignTwice();

int main()
{
    /* here is your code */
    GetInput();
    InitGCC();
    SolveND();
    SolveX();
    EncryptX();
    SignTwice();

    puts(C[0]); /* ======================== */
    puts(C[1]); /* 程序最终要输出这4项,     */
    puts(S[0]); /* 除此以外不可以有别的输出 */
    puts(S[1]); /* ======================== */
    return 0;
}

void GetInput()
{
    scanf("%s", A);
    scanf("%s", B);
    scanf("%s", PE);
    scanf("%s", GX);
    scanf("%s", GY);
    scanf("%s", NE);
    scanf("%s", DE);
    scanf("%s", N1[0]);
    scanf("%s", N1[1]);
    scanf("%s", D1[0]);
    scanf("%s", D1[1]);
    scanf("%s", X2[0]);
    scanf("%s", X2[1]);
    return;
}

void InitGCC()
{
    a = BN_new();
    BN_hex2bn(&a, A);
    b = BN_new();
    BN_hex2bn(&b, B);
    p = BN_new();
    BN_hex2bn(&p, PE);
    Gx = BN_new();
    BN_hex2bn(&Gx, GX);
    Gy = BN_new();
    BN_hex2bn(&Gy, GY);
    d = BN_new();
    BN_hex2bn(&d, DE);
    Gn = BN_new();
    BN_hex2bn(&Gn, NE);
    Rx = BN_new();
    Ry = BN_new();
    Tx = BN_new();
    Ty = BN_new();
    r = BN_new();
    s = BN_new();
    temp0 = BN_new();
    temp1 = BN_new();
    tempN = BN_new();
    tempD = BN_new();
    tempX1 = BN_new();

    Base = EC_GROUP_new(EC_GFp_mont_method());
    Ctx = BN_CTX_new();
    EC_GROUP_set_curve_GFp(Base, p, a, b, Ctx);
    G = EC_POINT_new(Base);
    EC_POINT_set_affine_coordinates_GFp(Base, G, Gx, Gy, Ctx);
    EC_GROUP_set_generator(Base, G, Gn, BN_value_one());
    R = EC_POINT_new(Base);
    T = EC_POINT_new(Base);
    return;
}

void SolveND()
{
    // 由ECC私钥推出公钥
    // R = d * G
    EC_POINT_mul(Base, R, d, NULL, NULL, Ctx);
    EC_POINT_get_affine_coordinates_GFp(Base, R, Rx, Ry, Ctx);

    BN_hex2bn(&r, N1[0]);
    BN_hex2bn(&s, N1[1]);
    // T = d * r
    EC_POINT_set_compressed_coordinates_GFp(Base, T, r, 0, Ctx);
    EC_POINT_mul(Base, T, NULL, T, d, Ctx);
    // 获取T的坐标Tx
    EC_POINT_get_affine_coordinates_GFp(Base, T, Tx, Ty, Ctx);
    BN_mod_inverse(temp0, Tx, Gn, Ctx);
    BN_clear(tempN);
    // tempN = s / (d * r)
    // 此即明文N
    BN_mod_mul(tempN, s, temp0, Gn, Ctx);

    BN_hex2bn(&r, D1[0]);
    BN_hex2bn(&s, D1[1]);
    // T = d * r
    EC_POINT_set_compressed_coordinates_GFp(Base, T, r, 0, Ctx);
    EC_POINT_mul(Base, T, NULL, T, d, Ctx);
    // 获取T的坐标Tx
    EC_POINT_get_affine_coordinates_GFp(Base, T, Tx, Ty, Ctx);
    BN_mod_inverse(temp0, Tx, Gn, Ctx);
    BN_clear(tempD);
    // tempD = s / (d * r)
    // 此即明文D
    BN_mod_mul(tempD, s, temp0, Gn, Ctx);

#ifdef debug
    printf("N is %s\n", BN_bn2hex(tempN));
    printf("D is %s\n", BN_bn2hex(tempD));
#endif
    return;
}

void SolveX()
{
    // 由X2 -> X1
    BN_hex2bn(&r, X2[0]);
    BN_hex2bn(&s, X2[1]);
    // T = d * r
    EC_POINT_set_compressed_coordinates_GFp(Base, T, r, 0, Ctx);
    EC_POINT_mul(Base, T, NULL, T, d, Ctx);
    // 获取T的坐标Tx
    EC_POINT_get_affine_coordinates_GFp(Base, T, Tx, Ty, Ctx);
    BN_mod_inverse(temp0, Tx, Gn, Ctx);
    BN_clear(tempX1);
    // tempX = s / (d * r)
    // 此即明文X1
    BN_mod_mul(tempX1, s, temp0, Gn, Ctx);
    strcpy(X1, BN_bn2hex(tempX1));

#ifdef debug
    printf("X1 is %s\n", X1);
#endif

    // 由X1 -> X
    myrsa = RSA_new();
    myrsa->flags |= RSA_FLAG_NO_BLINDING;
    myrsa->n = tempN;
    myrsa->e = NULL;
    myrsa->d = tempD;

    unsigned char *TempArray0, *TempArray1;
    TempArray0 = (char *)malloc(256 * sizeof(unsigned char));
    TempArray1 = (char *)malloc(256 * sizeof(unsigned char));

    int i;
    int Length = strlen((char *)X1) / 2;
    for (i = 0; i < Length; ++i)
        sscanf((char *)&X1[i << 1], "%02X", &TempArray0[i]);
    Length = RSA_private_decrypt(Length, TempArray0, TempArray1, myrsa, RSA_NO_PADDING);
    for (i = 0; i < Length; ++i)
        sprintf((char *)&X[i << 1], "%02X", TempArray1[i]);
    X[i << 1] = '\0';

#ifdef debug
    printf("X is %s\n", X);
#endif

    free(TempArray0);
    free(TempArray1);
    return;
}

void EncryptX()
{
    EC_POINT_mul(Base, R, d, NULL, NULL, Ctx);
    EC_POINT_get_affine_coordinates_GFp(Base, R, Rx, Ry, Ctx);

    long Seed;
    Seed = 20030606;
    RAND_add(&Seed, sizeof(Seed), 1);
    BN_rand(temp0, BN_num_bits(Gn), 0, 0);

    // T = k * G
    // 此即第一段密文
    EC_POINT_mul(Base, T, temp0, NULL, NULL, Ctx);
    EC_POINT_get_affine_coordinates_GFp(Base, T, Tx, Ty, Ctx);
    strcpy(C[0], BN_bn2hex(Tx));

    // T = k * R
    EC_POINT_mul(Base, T, NULL, R, temp0, Ctx);
    EC_POINT_get_affine_coordinates_GFp(Base, T, Tx, Ty, Ctx);
    // s = X * T (mod n)
    BN_hex2bn(&temp0, X);
    BN_mod_mul(s, temp0, Tx, Gn, Ctx);
    strcpy(C[1], BN_bn2hex(s));
    return;
}

void SignTwice()
{
    unsigned char XTemp[256], XSha1[256], PaddedXSha1[256], RSASignedXSha1[256], Temp[256];
    int i;
    for (i = 0; i < 32; i++)
        sscanf((char *)&X[i << 1], "%02X", &XTemp[i]);

    SHA1(XTemp, 32, XSha1);
    PaddedXSha1[0] = 0;
    PaddedXSha1[1] = 2;
    for (i = 2; i <= 10; ++i)
        PaddedXSha1[i] = 0xaa;
    PaddedXSha1[11] = 0;
    for (i = 12; i <= 31; ++i)
        PaddedXSha1[i] = XSha1[i - 12];

    myrsa->n = tempN;
    myrsa->e = NULL;
    myrsa->d = tempD;
    int Length = RSA_private_encrypt(32, PaddedXSha1, Temp, myrsa, RSA_NO_PADDING);

    for (i = 0; i < Length; ++i)
        sprintf((char *)&RSASignedXSha1[i << 1], "%02X", Temp[i]);
    RSASignedXSha1[i << 1] = '\0';
    BN_hex2bn(&temp0, RSASignedXSha1);

    long Seed = 20020523;
    RAND_add(&Seed, sizeof(Seed), 1);
    BN_rand(temp1, BN_num_bits(Gn), 0, 0);
    // T = k * G
    EC_POINT_mul(Base, T, temp1, NULL, NULL, Ctx);
    EC_POINT_get_affine_coordinates_GFp(Base, T, Tx, Ty, Ctx);
    // r = k * G + m
    BN_mod_add(r, Tx, temp0, Gn, Ctx);
    // s = d * r
    BN_mod_mul(s, r, d, Gn, Ctx);
    // s = k - d * r
    BN_sub(s, temp1, s);
    strcpy(S[0], BN_bn2hex(r));
    strcpy(S[1], BN_bn2hex(s));
    return;
}