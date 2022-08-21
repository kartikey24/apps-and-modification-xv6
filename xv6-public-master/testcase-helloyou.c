#include "types.h"
#include "stat.h"
#include "user.h"

int main(void)
{
        hello();
    for (int i = 0; i < 10; i++){
        printf(1,"%d %d",getMaxPid(),getNumProc());
    }
    exit();
}