#include <stdio.h>

void ok(int);
unsigned int count_bits_naive1(unsigned int);
unsigned int count_bits_naive2(unsigned int);
unsigned int count_bits_table(unsigned int);
unsigned int count_bits_parallel(unsigned int);
void testgroup (unsigned int (*)(unsigned int));

int test_count = 0;

int main (int argc, const char * argv[])
{
    testgroup(&count_bits_naive1);
    testgroup(&count_bits_naive2);
    testgroup(&count_bits_table);
    testgroup(&count_bits_parallel);
    return 0;
}

unsigned int count_bits_naive1 (unsigned int num)
{
    int count = 0;
    for (int i=0; i<32; i++) {
        count += (num >> i) & 1;
    }
    return count;
}

// Stops once leftmost set bit is reached
unsigned int count_bits_naive2 (unsigned int num)
{
    int count = 0;
    while (num) {
        count += (num & 1);
        num = num >> 1;
    }
    return count;
}

// Using a pre-calculated table for all bytes
static const unsigned char BitsSetTable256[256] =
{
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
    B6(0), B6(1), B6(1), B6(2)
};
unsigned int count_bits_table (unsigned int num)
{
    return BitsSetTable256[num       & 0x000000FF] +
           BitsSetTable256[num >>  8 & 0x000000FF] +
           BitsSetTable256[num >> 16 & 0x000000FF] +
           BitsSetTable256[num >> 24             ];
}

// Using summing in parallel
static const unsigned int B[] = {
0b01010101010101010101010101010101,
0b00110011001100110011001100110011,
0b00001111000011110000111100001111,
0b00000000111111110000000011111111,
0b00000000000000001111111111111111
};
unsigned int count_bits_parallel (unsigned int num)
{
    num = (num & B[0]) + (num >> 1  & B[0]);
    num = (num & B[1]) + (num >> 2  & B[1]);
    num = (num & B[2]) + (num >> 4  & B[2]);
    num = (num & B[3]) + (num >> 8  & B[3]);
    num = (num & B[4]) + (num >> 16 & B[4]);
    return num;
}

void testgroup (unsigned int count_bits(unsigned int))
{
    ok(count_bits(0xFFFFFFFF) == 32);
    ok(count_bits(1) == 1);
    ok(count_bits(0) == 0);
    ok(count_bits(0x10101010) == 4);
    ok(count_bits(0x01010101) == 4);
    ok(count_bits(0xFFFF0000) == 16);
    ok(count_bits(0x00FF00FF) == 16);
}

void ok (int test) {
    printf(test ? "ok %dn" : "NOT ok %dn", ++test_count);
}