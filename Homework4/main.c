#include <stdio.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/sha.h>

char A[0x100], B[0x100], PE[0x100];   // ecc���߲���a, b, p
char GX[0x100], GY[0x100], NE[0x100]; // ����G��x����, y����, G�Ľ�(��Ϊ256λ)
char DE[0x100];                       // ecc��˽Կd(256λ)
char N1[2][0x100], D1[2][0x100], X2[2][0x100];
// N1�����ecc��Կ���ܹ���rsa����N,
// ����N1[0]��ŵ�1��������, N1[1]��ŵ�2��������;
// D1�����ecc��Կ���ܹ���rsa˽Կd;
// X2�����ecc��Կ���ܹ���X1
char X1[0x100];            // X1�����rsa��Կ���ܹ���X
char X[0x100];             // X���һ��256λ�������, X<N �� X<NE
char N[0x100], D[0x100];   // rsa��N��˽Կd(256λ)
char RX[0x100], RY[0x100]; // ecc��ԿR��x���꼰y����
char C[2][0x100];          // �����ecc��Կ���ܹ���X, C[0]�ǵ�1��������, C[1]�ǵ�2��������
char S[2][0x100];          // �����ecnrǩ������RSA_private_encrypt_PKCS1_type_2(SHA1(X), D)
                           // ����SHA1��ɢ���㷨, RSA_private_encrypt_PKCS1_type_2()����RSA��
                           // ˽Կd��SHA1(X)���м���(ʵ������ǩ��), ����ǰ�����SHA1(X)��PKCS1_type_2
                           // ��ʽ�������, ʹ��SHA1(X)�ĳ��ȴ�20�ֽڱ��0x20�ֽ�(��256λ);
                           // ���NΪ256λ��PKCS1_type_2����ʽ����:
                           // 0x00, 0x02, 9�ֽڷ��������, 0x00, 20�ֽ�����
                           // ����������, S�Ƕ�SHA1(X)������ǩ��, ��1������rsa��˽Կǩ��, ��2������ecc��˽Կǩ��

EC_GROUP *Base;
BN_CTX *Ctx;

// ��Բ���߲���a b p
BIGNUM *a, *b, *p;
// ����G������ͽ���
BIGNUM *Gx, *Gy, *Gn;
// ��ԿR������
BIGNUM *Rx, *Ry;
// ��ʱ��T������
BIGNUM *Tx, *Ty;
// ˽Կd
BIGNUM *d;

// ECC��������
BIGNUM *r, *s;

// ��ʱ����
BIGNUM *temp0, *temp1;
BIGNUM *tempN, *tempD, *tempX1;

// ����G ��Կ��R ��ʱ��T
EC_POINT *G, *R, *T;

// RSA����
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
    puts(C[1]); /* ��������Ҫ�����4��,     */
    puts(S[0]); /* �������ⲻ�����б����� */
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
    // ��ECC˽Կ�Ƴ���Կ
    // R = d * G
    EC_POINT_mul(Base, R, d, NULL, NULL, Ctx);
    EC_POINT_get_affine_coordinates_GFp(Base, R, Rx, Ry, Ctx);

    BN_hex2bn(&r, N1[0]);
    BN_hex2bn(&s, N1[1]);
    // T = d * r
    EC_POINT_set_compressed_coordinates_GFp(Base, T, r, 0, Ctx);
    EC_POINT_mul(Base, T, NULL, T, d, Ctx);
    // ��ȡT������Tx
    EC_POINT_get_affine_coordinates_GFp(Base, T, Tx, Ty, Ctx);
    BN_mod_inverse(temp0, Tx, Gn, Ctx);
    BN_clear(tempN);
    // tempN = s / (d * r)
    // �˼�����N
    BN_mod_mul(tempN, s, temp0, Gn, Ctx);

    BN_hex2bn(&r, D1[0]);
    BN_hex2bn(&s, D1[1]);
    // T = d * r
    EC_POINT_set_compressed_coordinates_GFp(Base, T, r, 0, Ctx);
    EC_POINT_mul(Base, T, NULL, T, d, Ctx);
    // ��ȡT������Tx
    EC_POINT_get_affine_coordinates_GFp(Base, T, Tx, Ty, Ctx);
    BN_mod_inverse(temp0, Tx, Gn, Ctx);
    BN_clear(tempD);
    // tempD = s / (d * r)
    // �˼�����D
    BN_mod_mul(tempD, s, temp0, Gn, Ctx);

#ifdef debug
    printf("N is %s\n", BN_bn2hex(tempN));
    printf("D is %s\n", BN_bn2hex(tempD));
#endif
    return;
}

void SolveX()
{
    // ��X2 -> X1
    BN_hex2bn(&r, X2[0]);
    BN_hex2bn(&s, X2[1]);
    // T = d * r
    EC_POINT_set_compressed_coordinates_GFp(Base, T, r, 0, Ctx);
    EC_POINT_mul(Base, T, NULL, T, d, Ctx);
    // ��ȡT������Tx
    EC_POINT_get_affine_coordinates_GFp(Base, T, Tx, Ty, Ctx);
    BN_mod_inverse(temp0, Tx, Gn, Ctx);
    BN_clear(tempX1);
    // tempX = s / (d * r)
    // �˼�����X1
    BN_mod_mul(tempX1, s, temp0, Gn, Ctx);
    strcpy(X1, BN_bn2hex(tempX1));

#ifdef debug
    printf("X1 is %s\n", X1);
#endif

    // ��X1 -> X
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
    // �˼���һ������
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