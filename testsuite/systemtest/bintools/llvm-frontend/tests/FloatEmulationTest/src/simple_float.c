#include <stdio.h>
/* #include <stdint.h> */

volatile static unsigned a = 4100000000u;
volatile static int b = 41000;

volatile float aFloat, bFloat;
volatile float resMul, resDiv, resMod, resAdd, resSub, resNeg;

int main(int argc,char **argv,char **envp) { 

    
    aFloat = a;
    bFloat = b;
    
    resAdd = aFloat + bFloat;
    resSub = aFloat - bFloat;
    resNeg = -aFloat;

    resMul = aFloat * bFloat;
    resDiv = aFloat / bFloat;
//    resMod = aFloat % bFloat;
    
    unsigned srcVal = a;
    float tempFloat1, tempFloat2;
    unsigned dstVal1, dstVal2;
    
    tempFloat1 = srcVal;
    dstVal1 = tempFloat1;
    tempFloat2 = -tempFloat1;
    dstVal2 = tempFloat2;

    printf("%x to fp(%f) to uint %x\n", srcVal, tempFloat1, dstVal1);  
    printf("%x to fp(%f) to -fp(%f) to uint %x\n", srcVal, tempFloat1, tempFloat2, dstVal2);  

    printf("%f+%f=%f in hex format: %x+%x=%x\n", aFloat, bFloat, resAdd, *((int*)&aFloat), *((int*)&bFloat), *((int*)&resAdd));  
    printf("%f-%f=%f in hex format: %x-%x=%x\n", aFloat, bFloat, resSub, *((int*)&aFloat), *((int*)&bFloat), *((int*)&resSub));  
    printf("%f*%f=%f in hex format: %x*%x=%x\n", aFloat, bFloat, resMul, *((int*)&aFloat), *((int*)&bFloat), *((int*)&resMul));  
    printf("%f/%f=%f in hex format: %x/%x=%x\n", aFloat, bFloat, resDiv, *((int*)&aFloat), *((int*)&bFloat), *((int*)&resDiv));  
    printf("-%f=%f in hex format: -%x=%x\n", aFloat, resNeg, *((int*)&aFloat), *((int*)&resNeg));

    if (resDiv > resMul) return 1;
    if (resMod >= resAdd) return 2;
    if (resMod < resDiv) return 3;
    if (resAdd <= resNeg) return 4;

    if (resMod != resNeg &&         
        resMod == resSub) return 6;        

    return 0;
}
