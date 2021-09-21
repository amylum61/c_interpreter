#include<stdio.h>
#include<memory.h>
#include<stdlib.h>
#include<string.h>
#define int long long
/*
 next()词法分析，获取下一个标记，它将自动忽略空白字符
 program()语法分析的入口，分析整个c语言程序
 expression()用于解析一个表达式
 eval()虚拟机的入口，用于解释目标代码
*/

int token;          //current token 词法分析得到的标记流
char *src,*oldSrc;  //pointer to source code string 指向源代码字符串
int poolSize;       //default size of text/data/stack 池默认大小
int line;           //line数量
int *text;          //代码段指针
int *oldText;       //
int *stack;         //堆栈段指针
char *data;         //数据段指针

int *pc, *bp, *sp, ax, cycle;//虚拟机寄存器


enum{
    LEA,IMM,JMP,CALL,JZ,JNZ,ENT,ADJ,LEV,LI,LC,SI,SC,\
PUSH,OR,XOR,AND,EQ,NE,LT,GT,LE,GE,SHL,SHR,ADD,\
SUB,MUL,DIV,MOD,OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT
};//指令集 枚举类型
/*
IMM <num> 将 <num> 放入寄存器 ax 中。
LC 将对应地址中的字符载入 ax 中，要求 ax 中存放地址。
LI 将对应地址中的整数载入 ax 中，要求 ax 中存放地址。
SC 将 ax 中的数据作为字符存放入地址中，要求栈顶存放地址。
SI 将 ax 中的数据作为整数存放入地址中，要求栈顶存放地址。
*/
void next()
{
    token = *src++;
    return;
}

void expression(int level)
{
    //
}

void program()
{
    next();//获得下一个token
    while(token > 0)
    {
        printf("token is:%c\n",token);
        next();
    }
}

#undef int

int eval()
{
    int op, *tmp;
    while(1)
    {
        op = *pc++;//获得下一个操作符
        if(op ==IMM)       ax = *pc++;
        else if(op ==LC)   ax = *(char *)ax;
        else if(op ==LI)   ax = *(int *)ax;
        else if(op ==SC)
        {
            *(char *)*sp = ax;
            sp++;
            ax = *(char *)sp;
        }
        else if(op ==SI)   *(int *)*sp++ = ax;
        else if(op ==PUSH) *sp-- = ax;
        else if(op ==JMP)  pc = (int *)*pc;
        else if(op ==JZ)   pc = ax ? pc + 1 : (int *)*pc; //ax==1 pc++;
        else if(op ==JNZ)  pc = ax ? (int *)*pc : pc + 1;
        else if(op ==CALL)//调用子函数
        {
            sp--;
            *sp = (int)pc + 1;
            pc = (int *)*pc;
        }
            //else if(op == RET)    {pc = (int *)*sp;   sp++;} //   从子函数中返回
        else if(op ==ENT)
        {
            sp--;
            *sp = (int)bp;
            bp = sp;
            sp = sp - *pc;
            pc++;
        }
        else if(op ==ADJ)sp = sp + *pc++;
        else if(op ==LEV)
        {
            sp = bp; bp = (int *)*sp++;
            pc = (int *)*sp++;
        }
        else if(op ==ENT)
        {
            sp = bp;
            bp = (int *)*sp;
            sp++;
            pc = (int *)*sp;
            sp++;
        }
        else if(op ==LEA)  ax = (int)(bp + *pc++);
        else if (op ==OR)  ax = *sp++ | ax;
        else if (op ==XOR) ax = *sp++ ^ ax;
        else if (op ==AND) ax = *sp++ & ax;
        else if (op ==EQ)  ax = *sp++ == ax;
        else if (op ==NE)  ax = *sp++ != ax;
        else if (op ==LT)  ax = *sp++ < ax;
        else if (op ==LE)  ax = *sp++ <= ax;
        else if (op ==GT)  ax = *sp++ >  ax;
        else if (op ==GE)  ax = *sp++ >= ax;
        else if (op ==SHL) ax = *sp++ << ax;
        else if (op ==SHR) ax = *sp++ >> ax;
        else if (op ==ADD) ax = *sp++ + ax;
        else if (op ==SUB) ax = *sp++ - ax;
        else if (op ==MUL) ax = *sp++ * ax;
        else if (op ==DIV) ax = *sp++ / ax;
        else if (op ==MOD) ax = *sp++ % ax;

        else if (op ==EXIT){   ax = printf("exit(%d)",*sp);   return *sp;  }
        else if (op ==OPEN){   ax = open((char *)sp[1], sp[0]);    }
        else if (op ==CLOS){   ax = close(*sp);    }
        else if (op ==READ){   ax = read(sp[2], (char *)sp[1], *sp);   }
        else if (op ==PRTF){   tmp = sp + pc[1]; ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]);    }
        else if (op ==MALC){   ax = (int)malloc(*sp);  }
        else if (op ==MSET){   ax = (int)memset((char *)sp[2], sp[1], *sp);    }
        else if (op ==MCMP){   ax = memcmp((char *)sp[2], (char *)sp[1], *sp); }
        else    { printf("unknown instruction:%d\n",op);    return -1;}


    }
    return 0;
}

int main(int argc,char **argv)
{
#define int long long

    int i,fd;

    argc--;
    argv++;//删除main本身传入argument的数量

    poolSize = 256*1024;//任意大小
    line = 1;

    if((fd = open(*argv,0)) < 0)
    {
        printf("could not open(%s)\n", *argv);
        return -1;
    }

    if(!(src = oldSrc = malloc(poolSize)))
    {
        printf("could not malloc(%d) for source area\n",poolSize);
        return -1;
    }

    if((i = read(fd, src, poolSize-1)) <= 0 )
    {
        printf("read() return %d\n", i);
        return -1;
    }

    src[i] = 0;
    close(fd);

    if(!(text = oldText = malloc(poolSize)))
    {
        printf("could not malloc(%d) for source area\n",poolSize);
        return -1;
    }

    if(!(data = malloc(poolSize)))
    {
        printf("could not malloc(%d) for source area\n",poolSize);
        return -1;
    }

    if(!(stack = malloc(poolSize)))
    {
        printf("could not malloc(%d) for source area\n",poolSize);
        return -1;
    }

    memset(text, 0, poolSize);
    memset(data, 0, poolSize);
    memset(stack, 0, poolSize);

    bp = sp = (int *)((int)stack + poolSize);//初始化bp和sp为
    ax = 0;

    program();

    return eval();
}
