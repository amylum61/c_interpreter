#include<stdio.h>
#include<memory.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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

enum{
    Num = 128, Fun, Sys, Glo, Loc, Id,
    Char, Else, Enum, If, Int, Return, Sizeof, While,
    Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt,
    Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc,
    Dec, Brak
};//支持的标记符

/*
IMM <num> 将 <num> 放入寄存器 ax 中。
LC 将对应地址中的字符载入 ax 中，要求 ax 中存放地址。
LI 将对应地址中的整数载入 ax 中，要求 ax 中存放地址。
SC 将 ax 中的数据作为字符存放入地址中，要求栈顶存放地址。
SI 将 ax 中的数据作为整数存放入地址中，要求栈顶存放地址。
*/
struct identifier
{
    int token;//该标识符返回的标记
    int hash;//字符串哈希值
    char *name;//标识符本身字符串
    int class;//标识符的类别（数字 全局/局部变量）
    int type;//标识符类型char int等
    int value;//标识符的值  函数地址  变量值
    int Bclass;//全局标识符信息
    int Btype;
    int Bvalue;
};

int tokenVal;//标记流的值
int *currentId,//流ID
    *symbols;// 符号表

enum {Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize};

void next()
{
    char *lastPos;
    int hash;

    while(token = *src)
    {
        src++;
        //此处解析token
        if(token == '\n')
        {
            line++;
        }
        else if(token == '#')
        {
            //跳过宏定义 不支持宏定义
            while(*src != 0 && *src != '\n')
            {
                src++;
            }
        }
        else if((token >= 'a' && token <= 'z') || (token >='A' && token <= 'Z') || (token == '_'))
        {
            //解析标识符
            lastPos = src - 1;
            hash = token;

            while((*src >= 'a' && *src <= 'z') || (*src >='A' && *src <= 'Z') || (*src >='0' && *src <= '9') || (*src == '_'))
            {
                hash = hash * 147 + *src;
                src++;
            }

            //搜寻现有标识符 线性搜索
            currentId = symbols;
            while(currentId[token])
            {
                if(currentId[hash] = hash && !memcmp((char *)currentId[Name], lastPos, src - lastPos))
                {
                    token - currentId[token];
                    return;
                }
                currentId = currentId + IdSize;
            }

            //存储新ID
            currentId[Name] = (int)lastPos;
            currentId[Hash] = hash;
            token = currentId[Token] = Id;
            return;
        }
        else if(token >= '0' && token <= '9')
        {
            tokenVal = token - '0';
            if(tokenVal > 0)
            {
                //十进制  以1-9开头
                while(*src >= '0' && *src <='9')
                {
                    tokenVal = tokenVal * 10 + *src++ -'0';
                }
            }
            else
            {
                //十六进制 以0x开头
                if(*src == 'x' || *src == 'X')
                {
                    token = *++src;
                    while((token >= '0' && token <= '9') || (token >= 'a' && token <= 'f') || (token >= 'A' && token <= 'F'))
                    {
                        //dec hex转换
                        tokenVal = tokenVal * 16 + (token & 15) + (token >= 'A' ? 9 : 0);
                        token = *++src;
                    }
                }
                //八进制 以0开头
                else
                {
                    while(*src >= '0' && *src <= '7')
                    {
                        tokenVal = tokenVal * 8 + *src++ -'0';
                    }
                }
            }
            token = Num;
            return;
        }
        else if(token == '"' || token == '\'')
        {
            //解析字符串字面量，目前唯一支持的转义
            //字符为'\n'，将字符串字面值存储到数据中
            lastPos = data;
            while(*src != 0 && *src != token)
            {
                tokenVal = *src++;
                if(tokenVal = '\\')
                {
                    tokenVal = *src++;
                    if(tokenVal == 'n')
                    {
                        tokenVal = '\n';
                    }
                }
                if(token == '"')
                {
                    *data++ == tokenVal;
                }
            }
            src++;
            if(token == '"')
            {
                tokenVal = (int)lastPos;
            }
            else
            {
                token = Name;
            }
            return ;
        }
        else if(token == '/')
        {
            if(*src == '/')
            {
                while(*src != 0 && *src != '\n')
                {
                    src++;
                }
            }
            else
            {
                token = Div;
                return ;
            }
        }
        else if (token == '=') {
            // parse '==' and '='
            if (*src == '=') {
                src ++;
                token = Eq;
            } else {
                token = Assign;
            }
            return;
        }
        else if (token == '+') {
            // parse '+' and '++'
            if (*src == '+') {
                src ++;
                token = Inc;
            } else {
                token = Add;
            }
            return;
        }
        else if (token == '-') {
            // parse '-' and '--'
            if (*src == '-') {
                src ++;
                token = Dec;
            } else {
                token = Sub;
            }
            return;
        }
        else if (token == '!') {
            // parse '!='
            if (*src == '=') {
                src++;
                token = Ne;
            }
            return;
        }
        else if (token == '<') {
            // parse '<=', '<<' or '<'
            if (*src == '=') {
                src ++;
                token = Le;
            } else if (*src == '<') {
                src ++;
                token = Shl;
            } else {
                token = Lt;
            }
            return;
        }
        else if (token == '>') {
            // parse '>=', '>>' or '>'
            if (*src == '=') {
                src ++;
                token = Ge;
            } else if (*src == '>') {
                src ++;
                token = Shr;
            } else {
                token = Gt;
            }
            return;
        }
        else if (token == '|') {
            // parse '|' or '||'
            if (*src == '|') {
                src ++;
                token = Lor;
            } else {
                token = Or;
            }
            return;
        }
        else if (token == '&') {
            // parse '&' and '&&'
            if (*src == '&') {
                src ++;
                token = Lan;
            } else {
                token = And;
            }
            return;
        }
        else if (token == '^')
        {
            token = Xor;
            return;
        }
        else if (token == '%')
        {
            token = Mod;
            return;
        }
        else if (token == '*')
        {
            token = Mul;
            return;
        }
        else if (token == '[')
        {
            token = Brak;
            return;
        }
        else if (token == '?')
        {
            token = Cond;
            return;
        }
        else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':')
        {
            // directly return the character as token;
            return;
        }

    }


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

#undef int

int main(int argc,char *argv[])
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

    i = 0;
    text[i++] = IMM;
    text[i++] = 10;
    text[i++] = PUSH;
    text[i++] = IMM;
    text[i++] = 20;
    text[i++] = ADD;
    text[i++] = PUSH;
    text[i++] = EXIT;
    pc = text;

    program();

    return eval();
}
