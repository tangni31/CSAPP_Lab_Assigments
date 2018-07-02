/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
  return ~(~x|~y);
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
  return x>>(n<<3) & (0xff);

}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
  /*int n1 = n + ~0;
  int n2 = ((!!n)&1) << 31;
  int mask = n2>>n1;
  int ans = ~mask & (x>>n);
  return ans;*/
  return (x>>n)&(~(((!!n) & 1)<<31>>(n+~0)));
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
  int mask1  = 0x55555555; //0101 0101 ... 0101
  int mask2 = 0x33333333;  //0011 0011 ... 0011
  int mask3 = 0x0f0f0f0f; //0000 1111 ... 0000 1111
  int mask4 = 0x00ff00ff;  //0000 0000 1111 1111 0000 0000 1111 1111
  int mask5 = 0x0000ffff;  //0000 0000 0000 0000 1111 1111 1111 1111
  int count = 0;
  count = (x & mask1) + ((x>>1) &mask1);
  count = (count & mask2) + ((count >>2) &mask2);
  count = (count & mask3) + ((count >>4) & mask3);
  count = (count & mask4) + ((count >>8) & mask4);
  count = (count & mask5) + ((count >>16) & mask5);
  return count;
}
/* 
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int bang(int x) {
  
  return ~((x|(~x+1))>>31) & 1;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  return 0x80000000;//32-bit minium numerber is 1000 0000 0000 0000 0000 0000 0000 0000 = 0x80 00 00 00
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) { 
	//for x>=0: 3-bit can represent: 000 001 010 011
  int neg = x >>31;   				// if x >= 0 : ans is !(x>>(n-1)) = !(x>>(n+~0))
  return (~neg& !(x>>(n+~0))) + (neg&!((~x)>>(n+~0)));// if x<0, use ~x to represent x, if ~x can be represented by n-bit,then x can be represented too.(eg. x = 100, ~x = 011)
}
/* 
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int divpwr2(int x, int n) { 
    int sign = x >> 31; // if neg: sign = 0xffffffff, if pos: sign = 0x0
    int bias = sign & ((1<<n) +~0); /*need add bias if neg(round toward zero)*/
    x = x + bias;
    return x>>n;
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  
  return ~x+1;
}
/* 
 * isPositive - return 1 if x > 0, return 0 otherwise 
 *   Example: isPositive(-1) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 3
 */
int isPositive(int x) {
  return !((x>>31)|(!x));
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  int signx = x>>31;
  int signy = y>>31;
  int equal = !(signx^signy) & ((x+~y)>>31);/*sign x = sign y, x-y-1<0*/
  int nequal = signx & !signy;
  return equal|nequal;
}
/*
 * ilog2 - return floor(log base 2 of x), where x > 0
 *   Example: ilog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
int ilog2(int x) {
  /* ans is the leftmost 1's position of x*/
  /* remove all '1's except the left most '1'*/
  x |= x >>1;
  x |= x >>2;
  x |= x >>4;
  x |= x >>8;
  x |= x >>16;
  x ^= x>>1;
  /*find leftmost 1's postion*/
  int pos_16,pos_8,pos_4,pos_2,pos_1;
  pos_16 = !(!(x>>16)) << 4;
  x=x>>pos_16;
  pos_8 = !(!(x>>8)) << 3;
  x=x>>pos_8;
  pos_4 = !(!(x>>4)) << 2;
  x=x>>pos_4;
  pos_2 = !(!(x>>2)) << 1;
  x=x>>pos_2;
  pos_1 = !(!(x>>1));
  return pos_16+pos_8+pos_4+pos_2+pos_1;
}
/* 
 * float_neg - Return bit-level equivalent of expression -f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_neg(unsigned uf) {
 unsigned exp = uf >>23 & 0xff;
 unsigned frac = uf &0x7fffff;
 if (exp == 0xff && frac != 0)
     return uf;
 unsigned mask = 1<<31;
 return uf^mask;
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {
    unsigned shiftLeft=0;  
    unsigned afterShift, tmp, flag;  
    unsigned absX=x;  
    unsigned sign=0;  
    //special case  
    if (x==0) return 0;  
    //if x < 0, sign = 1000...,abs_x = -x  
    if (x<0){  
        sign=0x80000000;  
        absX=-x;  
    }  
    afterShift=absX;  
    //count shift_left and after_shift  
    while (1){  
        tmp=afterShift;  
        afterShift<<=1;  
        shiftLeft++;  
        if (tmp & 0x80000000) break;  
    }  
    if ((afterShift & 0x01ff)>0x0100)  
        flag=1;  
    else if ((afterShift & 0x03ff)==0x0300)  
        flag=1;  
    else  
        flag=0;  
    return sign + (afterShift>>9) + ((159-shiftLeft)<<23) + flag;
}
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
  unsigned f=uf;  
    if ((f & 0x7F800000) == 0){  /*0x7f800000 = 0 (111 1111 1) (000 0000 0000 0000 0000 0000)*/
        // f*2 = shift one bit to left  
                f = ((f & 0x007FFFFF)<<1) | (0x80000000 & f);  
    }  
    else if ((f & 0x7F800000) != 0x7F800000){  
        /* Float has a special exponent. */  
        /* Increment exponent, effectively multiplying by 2. */  
        f =f+0x00800000;  
        }  
    return f;
}
