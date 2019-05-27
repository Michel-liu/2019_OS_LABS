#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include<stdlib.h>
#include<pthread.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>

enum allType {
    NAME=1, // identifier
    STRING=2, // string constant "..."
    ICON=3, // integer or character constant
    FCON=4, // floating point constant
    PLUS=5, // +
    MINUS=6, // -
    STAR=7, // *
    AND=8, // &
    QUEST=9, // ?
    COLON=10, // :
    ANDAND=11, // &&
    OROR=12, // ||
    RELOP=13, // > < >= <=
    EQUOP=14, // == !=
    DIVOP=15, // / %
    OR=16, // |
    XOR=17, // ^
    SHIFTOP=18, // << >>
    INCOP=19, // ++ --
    UNOP=20, // ! ~
    STRUCTOP=21, // . ->
    TYPE=22, // int, long, etc
    CLASS=23, // extern, static, typeof, etc
    STRUCT=24, // struct, union
    RETURN=25, // return
    GOTO=26, // goto
    IF=27, // if
    ELES=28, //eles
    SWITCH=29, // switch
    BREAK=30, // break
    CONTINUE=31, // continue
    WHILE=32, // while
    DO=33, // do
    FOR=34, // for
    DEFAULT=35, // default
    CASE=36, // case
    SIZEOF=37, // sizeof
    LP=38, // (
    RP=39, // )
    LC=40, // {
    RC=41, // }
    LB=42, // [
    RB=43, // ]
    COMMA=44, // ,
    SEMI=45, // ;
    EQUAL=46, // =
    ASSIGNOP=47, // += -= etc

    JINGHAO=48, // #
    INCLUDE=49, // include
    DEFINE=50, // define
    IFDEF=51, // ifdef
    IFNDEF=52, // ifndef
    ENDIF=53, // endif
};

typedef struct
{
    char *name;
    int val;
}KWRDSTRUCT;

KWRDSTRUCT Ktab[]={
        {"auto", CLASS},
        {"break", BREAK},
        {"case", CASE},
        {"char", TYPE},
        {"continue", CONTINUE},
        {"default", DEFAULT},
        {"do", DO},
        {"double", TYPE},
        {"eles", ELES},
        {"extern", CLASS},
        {"float", TYPE},
        {"for", FOR},
        {"goto", GOTO},
        {"if", IF},
        {"int", TYPE},
        {"long", TYPE},
        {"register", CLASS},
        {"return", RETURN},
        {"short", TYPE},
        {"sizeof", SIZEOF},
        {"static", CLASS},
        {"struct", STRUCT},
        {"switch", SWITCH},
        {"typedef", CLASS},
        {"union", STRUCT},
        {"unsigned", TYPE},
        {"void", TYPE},
        {"while", WHILE},

        {"include", INCLUDE},
        {"define", DEFINE},
        {"ifdef", IFDEF},
        {"ifndef", IFNDEF},
        {"endif", ENDIF}
};

enum charType {
    letter=201,
    number,
    operate,
    dot,
    delimiter,
    blank,
    singleMark,
    doubleMark,
    error
};

enum charsType {
    word=300, // identifier or keyword
    intergernum,
    floatnum,
    sigleOp,
    string,
    doubleOp
};

short which_type(char* t){
    if(*t >= 'a' && *t <= 'z') return letter;
    if(*t >= 'A' && *t <= 'Z') return letter;
    if(*t == '_'||*t == '$') return letter;
    if(*t >= '0' && *t <= '9') return number;
    if(*t == '+'||*t == '-'||*t == '*'||*t == '/'||*t == ':'||*t == '='||*t == '<'||*t == '>'||*t=='?'||*t=='&'||*t=='|'||*t=='!'||*t=='%'||*t=='^'||*t=='~') return operate;
    if(*t == '.') return dot;
    if(*t == ','||*t == ';'||*t == '('||*t == ')'||*t == '['||*t == ']'||*t == '{'||*t == '}'||*t == '#') return delimiter;
    if(*t == '\t'||*t == ' '||*t == '\n') return blank;
    if(*t == '\'') return singleMark;
    if(*t == '\"') return doubleMark;
    return error;
}
/*
    && &=
    || |=
    >= >>
    <= <<
    ==
    !=
    ++ +=
    -- -> -=
    *=
    /=
    %=
    ^=
*/
short whichSingleOp(char* first){
    if(*first == '=') return EQUAL;
    if(*first == ';') return SEMI;
    if(*first == ',') return COMMA;
    if(*first == ']') return RB;
    if(*first == '[') return LB;
    if(*first == '}') return RC;
    if(*first == '{') return LC;
    if(*first == ')') return RP;
    if(*first == '(') return LP;
    if(*first == '.') return STRUCTOP;
    if(*first == '!' || *first == '~') return UNOP;
    if(*first == '^') return XOR;
    if(*first == '|') return OR;
    if(*first == '/' || *first == '%') return DIVOP;
    if(*first == '>' || *first == '<') return RELOP;
    if(*first == ':') return COLON;
    if(*first == '?') return QUEST;
    if(*first == '&') return AND;
    if(*first == '*') return STAR;
    if(*first == '-') return MINUS;
    if(*first == '+') return PLUS;
    return 0;
}

short whichDoubleOp(char *first, char *second){
    if(*first == '&' && *second == '&') return ANDAND;
    if(*first == '|' && *second == '|') return OROR;

    if(*first == '>' && *second == '=') return RELOP;
    if(*first == '<' && *second == '=') return RELOP;

    if(*first == '=' && *second == '=') return EQUOP;
    if(*first == '!' && *second == '=') return EQUOP;

    if(*first == '<' && *second == '<') return SHIFTOP;
    if(*first == '>' && *second == '>') return SHIFTOP;

    if(*first == '+' && *second == '+') return INCOP;
    if(*first == '-' && *second == '-') return INCOP;

    if(*first == '-' || *second == '>') return STRUCTOP;

    if(*first == '+' && *second == '=') return ASSIGNOP;
    if(*first == '-' && *second == '=') return ASSIGNOP;
    if(*first == '*' && *second == '=') return ASSIGNOP;
    if(*first == '/' && *second == '=') return ASSIGNOP;
    if(*first == '%' && *second == '=') return ASSIGNOP;
    if(*first == '^' && *second == '=') return ASSIGNOP;
    if(*first == '&' && *second == '=') return ASSIGNOP;
    if(*first == '|' && *second == '=') return ASSIGNOP;

    return 0;
}

KWRDSTRUCT *bsrch(KWRDSTRUCT *a, KWRDSTRUCT *tab, int number){
    for(int i=0; i<number; i++)
        if(!strcmp(a->name,tab[i].name))
            return &tab[i]; // keyword
    return NULL; // identifier
}

int idOrKeyword(char *lx){
    KWRDSTRUCT *p;
    KWRDSTRUCT dummy;
    static int number = sizeof(Ktab)/sizeof(KWRDSTRUCT);
    dummy.name = lx;
    p = bsrch(&dummy,Ktab,number);
    return(p?p->val:NAME);
}

void wirteOut(int c, char *text){
    printf("(");
    switch (c)
    {
        case 1: printf("NAME, %s)\n",text); break;// identifier
        case 2: printf("STRING, %s)\n",text); break; // string constant "..."
        case 3: printf("ICON, %s)\n",text); break;// integer or character constant
        case 4: printf("FCON, %s)\n",text); break;// floating point constant
        case 5: printf("PLUS, %s)\n",text); break;// +
        case 6: printf("MINUS, %s)\n",text); break;// -
        case 7: printf("STAR, %s)\n",text); break;// *
        case 8: printf("AND, %s)\n",text); break;// &
        case 9: printf("QUEST, %s)\n",text); break; // ?
        case 10: printf("COLON, %s)\n",text); break; // :
        case 11: printf("ANDAND, %s)\n",text); break; // &&
        case 12: printf("OROR, %s)\n",text); break; // ||
        case 13: printf("RELOP, %s)\n",text); break; // > < >= <=
        case 14: printf("EQUOP, %s)\n",text); break; // == !=
        case 15: printf("DIVOP, %s)\n",text); break; // / %
        case 16: printf("OR, %s)\n",text); break; // |
        case 17: printf("XOR, %s)\n",text); break;// ^
        case 18: printf("SHIFTOP, %s)\n",text); break;// << >>
        case 19: printf("INCOP, %s)\n",text); break; // ++ --
        case 20: printf("UNOP, %s)\n",text); break; // ! ~
        case 21: printf("STRUCTOP, %s)\n",text); break; // . ->
        case 22: printf("TYPE, %s)\n",text); break; // int, long, etc
        case 23: printf("CLASS, %s)\n",text); break; // extern, static, typeof, etc
        case 24: printf("STRUCT, %s)\n",text); break; // struct, union
        case 25: printf("RETURN, %s)\n",text); break; // return
        case 26: printf("GOTO, %s)\n",text); break; // goto
        case 27: printf("IF, %s)\n",text); break; // if
        case 28: printf("ELES, %s)\n",text); break; //eles
        case 29: printf("SWITCH, %s)\n",text); break; // switch
        case 30: printf("BREAK, %s)\n",text); break; // break
        case 31: printf("CONTINUE, %s)\n",text); break; // continue
        case 32: printf("WHILE, %s)\n",text); break; // while
        case 33: printf("DO, %s)\n",text); break; // do
        case 34: printf("FOR, %s)\n",text); break; // for
        case 35: printf("DEFAULT, %s)\n",text); break; // default
        case 36: printf("CASE, %s)\n",text); break; // case
        case 37: printf("SIZEOF, %s)\n",text); break; // sizeof
        case 38: printf("LP, %s)\n",text); break; // (
        case 39: printf("RP, %s)\n",text); break; // )
        case 40: printf("LC, %s)\n",text); break; // {
        case 41: printf("RC, %s)\n",text); break;// }
        case 42: printf("LB, %s)\n",text); break;// [
        case 43: printf("RB, %s)\n",text); break; // ]
        case 44: printf("COMMA, %s)\n",text); break;// ,
        case 45: printf("SEMI, %s)\n",text); break;// ;
        case 46: printf("EQUAL, %s)\n",text); break; // =
        case 47: printf("ASSIGNOP, %s)\n",text); break; // += -= etc
        case 48: printf("JINGHAO, %s)\n",text); break;// #
        case 49: printf("INCLUDE, %s)\n",text); break;// include
        case 50: printf("DEFINE, %s)\n",text); break;// define
        case 51: printf("IFDEF, %s)\n",text); break;// ifdef
        case 52: printf("IFNDEF, %s)\n",text); break;// ifndef
        case 53: printf("ENDIF, %s)\n",text); break;// endif
        default: break;
    }
}

char readBuf[65]={'\0'};
short startPr, scanfPr = 0;
short writePr = 0;
FILE *fp = NULL;

sem_t* canWrite = NULL;
sem_t* canRead = NULL;

void*writeBufProcess(void*arg){
    while(1)
    {
        sem_wait(canWrite);
        char t;
        t = fgetc(fp);
        readBuf[writePr++%64] = t;
        sem_post(canRead);
        if (t==EOF) {break;}
    }
}

void*readBufProcess(void*arg){
    while(1){
        int wordType;
        short errorState = 0;
        sem_wait(canRead);
        char *t = readBuf+startPr;
        short type = which_type(t);
        if(*t == EOF) break;
        if(type == letter)
        {
            // 标识符处理
            wordType = word;
            while(1){
                sem_wait(canRead);
                scanfPr = (scanfPr+1)%64;
                char *next = readBuf + scanfPr;
                short next_type = which_type(next);
                if(next_type == number || next_type == letter) continue;
                if(next_type == error) errorState = 1;
                break;
            }
            sem_post(canRead);
        } else if(type == number)
        {
            // 整数或者小数
            short hasdot = 0;
            // 使用科学计数法
            short hasE = 0;
            while(1){
                sem_wait(canRead);
                scanfPr = (scanfPr+1)%64;
                char *next = readBuf + scanfPr;
                short next_type = which_type(next);
                if(next_type == number) continue;
                if(next_type == dot){
                    if(hasdot) {errorState=1;} // error
                    hasdot=1;
                    continue;
                }
                if(*next == 'E' || *next == 'e'){
                    if(hasE) {errorState=1;} // error
                    hasE=1;
                    continue;
                }
                if(next_type == delimiter||next_type == operate) break;
                else
                {
                    errorState=1;
                    break;
                }
            }
            sem_post(canRead);
            if(hasdot || hasE) wordType = floatnum;
            else wordType = intergernum;

        } else if(type == operate || type == dot)
        {
            // 其它符号
            sem_wait(canRead);
            scanfPr = (scanfPr+1)%64;
            char *next = readBuf + scanfPr;
            short next_type = which_type(next);
            if(next_type == operate){
                // 先匹配两个，不能匹配一个，操作符号数量错误留到语法分析处理解决
                if(whichDoubleOp(t,next)) {wordType = doubleOp;scanfPr = (scanfPr+1)%64;}
                else {wordType = sigleOp;
                    sem_post(canRead);
                }
            }else {wordType = sigleOp;
                sem_post(canRead);
            }
        }else if(type == delimiter){
            wordType = delimiter;
            scanfPr = (scanfPr+1)%64;
        }else if(type == blank)
        {
            // blank has no meaning ?
            wordType = blank;
            scanfPr = (scanfPr+1)%64;

        } else if(type == singleMark)
        {
            wordType = intergernum;
            while(1){
                sem_wait(canRead);
                scanfPr = (scanfPr+1)%64;
                char *next = readBuf + scanfPr;
                short next_type = which_type(next);
                if(next_type == singleMark && *(next-1) != '\\'){
                    scanfPr = (scanfPr+1)%64;
                    break;
                }
                continue;
            }
        }else if(type == doubleMark){
            wordType = string;
            while(1){
                sem_wait(canRead);
                scanfPr = (scanfPr+1)%64;
                char *next = readBuf + scanfPr;
                short next_type = which_type(next);
                if(next_type == doubleMark && *(next-1) != '\\'){
                    scanfPr = (scanfPr+1)%64;
                    break;
                }
                continue;
            }
        }else{
            // 错误
            wordType = error;
            errorState = 1;
        }

        // 结果输出 错误处理
        int length = (scanfPr - startPr + 64)%64;
        char strings[length+1];
        memset(strings,'\0',sizeof(strings));
        for(int i=0; i<length; i++) strings[i] = *(readBuf+(startPr+i)%64);

        if(errorState){
            if(wordType==error) printf("(发现非法字符,%s)\n",strings);
            if(wordType==word) printf("(发现错误标识符,%s)\n",strings);
            if(wordType==floatnum) printf("(发现浮点数定义错误,%s)\n",strings);
            if(wordType==intergernum) printf("(发现整数定义错误,%s)\n",strings);
        } else
        {
            if(wordType==word){
                int t = idOrKeyword(strings);
                wirteOut(t,strings);
            }
            else if(wordType==floatnum){
                wirteOut(4,strings);
            }
            else if(wordType==intergernum){
                wirteOut(3,strings);
            }
            else if(wordType==doubleOp){
                int t = whichDoubleOp(strings,strings+1);
                wirteOut(t,strings);
            }
            else if(wordType==sigleOp || wordType == delimiter){
                int t = whichSingleOp(strings);
                wirteOut(t,strings);
            } else if(wordType == blank){
                if(strcmp(strings,"\n")==0) printf("(BLANK,\\n)\n");
                if(strcmp(strings," ")==0) printf("(BLANK, )\n");
                if(strcmp(strings,"\t")==0) printf("(BLANK,\\t)\n");
            } else if(wordType == string)
            {
                wirteOut(2,strings);
            }

        }
        while(length--)sem_post(canWrite);
        startPr = scanfPr;
        errorState = 0;
    }
}

int main(){
    char file_path[30] = {"/Users/liuhuan/test.c"};
//    printf("Please enter the file path for analysing.(Defalut is %s)\n",file_path);
//    char file_path_enter[30];
//    scanf("%s",file_path_enter);
//    if (access(file_path_enter, R_OK))
//        printf("File does not exist or have no access to read! Use defalut.\n");
//    else{
//        memset(file_path,'\0',sizeof(file_path));
//        strcpy(file_path,file_path_enter);
//    }
    fp = fopen(file_path, "r");

    pthread_t p1,p2;

    canWrite = sem_open("M1", O_CREAT, 0666, 64);
    canRead = sem_open("M2", O_CREAT, 0666, 0);

    pthread_create(&p1,NULL,writeBufProcess,NULL);
    pthread_create(&p2,NULL,readBufProcess,NULL);
    pthread_join(p1,NULL);
    pthread_join(p2,NULL);

    sem_close(canWrite);
    sem_close(canRead);
    sem_unlink("M1");
    sem_unlink("M2");
    fclose(fp);
    return 0;
}