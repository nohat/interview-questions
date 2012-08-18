Problem statement: given a 32-bit integer, write a function that returns the number of bits in the binary representation of the integer which are set.

For example,

```c
count_bits(0x00000001) → 1  
count_bits(0xFFFFFFFF) → 32  
count_bits(0x10101010) → 4
```

Much credit is due to this page of [bit twiddling hacks][1].

 [1]: http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetNaive

The naïve solution is to shift the input by one bit and check the last bit 32 times:

```c
unsigned int count_bits_naive1 (unsigned int num)
{
    int count = 0;
    for (int i=0; i&lt;32; i++) {
        count += (num >> i) & 1;
    }
    return count;
}
```

This version adds the optimization so that it stops once the leftmost set bit is reached. No need to continue counting if num == 0:

```c
unsigned int count_bits_naive2 (unsigned int num)
{
    int count = 0;
    while (num) {
        count += (num & 1);
        num = num >> 1;
    }
    return count;
}
```

We can reduce the number of operations by pre-calculating the bit sum for all possible octets and then just looking at the 4 octets in a 32-bit integer:

```c
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
```

Finally, we can count the bits by summing in parallel, 16 pairs of 1-bit integers, 8 pairs of 2-bit integers, 4 pairs of 4-bit integers, 2 pairs of 8-bit integers, and finally 1 pair of 16-bit integers:

```c
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
```

This method as written has more instructions than the previous version but uses less memory.
