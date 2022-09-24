static long32 f(ulong32 r, unsigned char subkey[8])
{
   int i, j;
   unsigned char s[4], plaintext[8], m[4], t[4];
   unsigned long int rval;

   memset(s, 0, 4);
   memset(m, 0, 4);
   memset(t, 0, 4);
   memset(plaintext, 0, 8);

   for (i = 0; i < 4; ++i)
      s[i] = (r >> ((8 * (3 - i)) & 0xff));

   for (i = 0; i < 8; ++i)
   {
      for (j = 0; j < 6; ++j)
      {
         int index = plaintext_32bit_expanded_to_48bit_table[i * 6 + j] - 1;
         int target = s[index / 8] & bytebit[index % 8];
         plaintext[i] = target ? plaintext[i] | bytebit[j + 2] : plaintext[i];
      }
   }

   for (i = 0; i < 8; i++)
      plaintext[i] = plaintext[i] ^ subkey[i];

   for (i = 0; i < 8; i++)
   {
      int line = ((plaintext[i] & 0x20) >> 4) + (plaintext[i] & 0x1);
      int row = (plaintext[i] & 0x1e) >> 1;

      if (i % 2 == 0)
         m[i / 2] = m[i / 2] | ((sbox[i][line * 16 + row]) << 4);
      else
         m[i / 2] = m[i / 2] | sbox[i][line * 16 + row];
   }

   for (i = 0; i < 4; i++)
   {
      t[i] = 0;
      for (j = 0; j < 8; j++)
      {
         int index = sbox_perm_table[i * 8 + j] - 1;
         int target = m[index / 8] & bytebit[index % 8];
         t[i] = target ? t[i] + bytebit[j] : t[i];
      }
   }

   rval = (t[0] << 24 | t[1] << 16 | t[2] << 8 | t[3]);
   return rval;
}