Problem statement: given a 32-bit integer, write a function that returns the number of bits in the binary representation of the integer which are set.

For example,

`count_bits(0x00000001) → 1<br />
count_bits(0xFFFFFFFF) → 32<br />
count_bits(0x10101010) → 4`

Much credit is due to this page of [bit twiddling hacks][1].

 [1]: http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetNaive

The naïve solution is to shift the input by one bit and check the last bit 32 times:

<pre class="prettyprint lang-c"><code>unsigned int count_bits_naive1 (unsigned int num)
{
    int count = 0;
    for (int i=0; i&lt;32; i++) {
        count += (num >> i) & 1;
    }
    return count;
}
</code></pre>

This version adds the optimization so that it stops once the leftmost set bit is reached. No need to continue counting if num == 0:

<pre class="prettyprint lang-c"><code>unsigned int count_bits_naive2 (unsigned int num)
{
    int count = 0;
    while (num) {
        count += (num & 1);
        num = num >> 1;
    }
    return count;
}
</code></pre>

We can reduce the number of operations by pre-calculating the bit sum for all possible octets and then just looking at the 4 octets in a 32-bit integer:

<pre class="prettyprint lang-c"><code>static const unsigned char BitsSetTable256[256] =
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
</code></pre>

Finally, we can count the bits by summing in parallel, 16 pairs of 1-bit integers, 8 pairs of 2-bit integers, 4 pairs of 4-bit integers, 2 pairs of 8-bit integers, and finally 1 pair of 16-bit integers:

<pre class="prettyprint lang-c"><code>static const unsigned int B[] = {
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
</code></pre>

This method as written has more instructions than the previous version but uses less memory.

Here's the whole C program:

<pre class="prettyprint lang-c"><code>#include &lt;stdio.h>

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
    for (int i=0; i&lt;32; i++) {
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
</code></pre>