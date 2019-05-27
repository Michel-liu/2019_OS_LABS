#include <stdio.h>
#include<stdlib.h>
#include<time.h>
#include <cstring>

#define random(x) (rand()%x)
#define MAXLEN 1000
#define POOLLEN 4
#define FREELISTLENGTH 4
#define CHANGELISTLENGTH 4
#define FREEMLENGTH 4

int testCount,testAlignment[MAXLEN],lackInterrupt,pool[POOLLEN],lastTimeAccess[POOLLEN]={0};
//={27,4,29,31,1,7,14,24,24,5,18,11,28,31,5,4,11,14,8,13,10,12,27,11,1,24,3,29,18,7,14,8}
short test_M[MAXLEN];
//={1,0,1,1,0,0,0,0,1,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0}
typedef struct memPageState{
    short A=0;
    short M=0;
}memPageState;
memPageState poolState[POOLLEN];

short findAPlaceForImprovedClock(){
    while (1) {
        for (int i = 0; i < POOLLEN; ++i) {
            if (poolState[i].M == 0 && poolState[i].A == 0) return i;
        }
        for (int j = 0; j < POOLLEN; ++j) {
            if (poolState[j].M == 1 && poolState[j].A == 0) return j;
            else poolState[j].A = 0;
        }
    }
}

void updateLastTime(int select=-1){
    if(select+1)
        for (int i = 0; i < POOLLEN; ++i)
            if(i==select)
                lastTimeAccess[i]=0;
            else
                lastTimeAccess[i]++;
    else
        for (int j = 0; j < POOLLEN; ++j)
            lastTimeAccess[j]++;
}

short findTheWorstTime(){
    int place = 0;
    int maxTime = lastTimeAccess[0];
    for (int i = 1; i < POOLLEN; ++i) {
        if(maxTime < lastTimeAccess[i])
        {
            place = i;
            maxTime = lastTimeAccess[i];
        }
    }
    return place;
}

void print(int cur, short hasFind, short newPoint, short inter=0){
    printf("***********************************\n");
    printf("正在执行第%d条指令，需要访问的页号为%d。\n",cur,testAlignment[cur]);
    if(!hasFind){
        printf("缓冲区未找到该页号!加载其至位置%d\n",newPoint);
    } else{
        printf("缓冲区找到该页号!其位于位置%d\n",newPoint);
    }
    printf("当前缓冲区内容为:");
    for (int i = 0; i <POOLLEN; ++i) {
        printf("%4d",pool[i]);
    }
    putchar('\n');
    if(inter)printf("缺页中断!\n");
}

short hasEmptyPlace(int cur)
{
    for(int i=0;i<POOLLEN;i++)
        if(pool[i]==-1) return i+1;
    return 0;
}

short isInPool(int Align){
    for(short i=0;i<POOLLEN;i++)
        if(pool[i]==Align)
            return 1+i;
    return 0;
}

short findAPlace(int cur){
    short find[POOLLEN]={0};
    short findCount = 0;
    for(int j=cur+1;j<testCount;j++)
    {
        if(findCount==POOLLEN-1) break;
        for(short i=0;i<POOLLEN;i++)
        {
            if(testAlignment[j]==pool[i] && find[i]!=1){
                findCount++;
                find[i] = 1;
                break;
            }
        }
    }
    for (short i = 0;  i < POOLLEN; i++) {
        if(!find[i]) return i;
    }
}

void bestExchange()
{
    int curAlign = 0;
    for(int i=0; i<testCount; i++,curAlign++)
    {
        short p = hasEmptyPlace(curAlign);
        if(p)
        {
            short t = isInPool(testAlignment[curAlign]);
            if(!t)
            {
                pool[p-1]=testAlignment[curAlign];
                print(curAlign,0,p-1);
                continue;
            }
            else
                print(curAlign,1,t-1);
        } else {
            if(short t = isInPool(testAlignment[curAlign]))
            {
                print(curAlign,1,t-1);
                continue;
            }
            else{
                lackInterrupt++;
                short newPlace = findAPlace(curAlign);
                pool[newPlace] = testAlignment[curAlign];
                print(curAlign,0,newPlace,1);
            }
        }
    }
}

void firstInFirstOut(){
    int curAlign = 0;
    short nextOut = 0;
    for(int i=0;i < testCount; i++,curAlign++)
    {
        short p = hasEmptyPlace(curAlign);
        if(p)
        {
            short t = isInPool(testAlignment[curAlign]);
            if(!t)
            {
                pool[p-1]=testAlignment[curAlign];
                print(curAlign,0,p-1);
                continue;
            }
            else
                print(curAlign,1,t-1);
        } else {
            if(short t = isInPool(testAlignment[curAlign]))
            {
                print(curAlign,1,t-1);
                continue;
            }
            else{
                lackInterrupt++;
                short newPlace = nextOut;
                pool[newPlace] = testAlignment[curAlign];
                print(curAlign,0,newPlace,1);
                nextOut = (nextOut+1)%POOLLEN;
            }
        }
    }
}

void lastRecentUsed(){
    int curAlign = 0;
    for(int i=0;i < testCount; i++,curAlign++)
    {
        short p = hasEmptyPlace(curAlign);
        if(p)
        {
            short t = isInPool(testAlignment[curAlign]);
            if(!t)
            {
                pool[p-1]=testAlignment[curAlign];
                print(curAlign,0,p-1);
                continue;
            }
            else
                print(curAlign,1,t-1);
        } else {
            if(short t = isInPool(testAlignment[curAlign]))
            {
                updateLastTime(t-1);
                print(curAlign,1,t-1);
                continue;
            }
            else{
                updateLastTime();
                lackInterrupt++;
                short newPlace = findTheWorstTime();
                pool[newPlace] = testAlignment[curAlign];
                print(curAlign,0,newPlace,1);
            }
        }
    }
}

void improvedClock(){
    int curAlign = 0;
    for(int i=0; i<testCount; i++,curAlign++)
    {
        short p = hasEmptyPlace(curAlign);
        if(p)
        {
            short t = isInPool(testAlignment[curAlign]);
            if(!t)
            {
                pool[p-1]=testAlignment[curAlign];
                print(curAlign,0,p-1);
                poolState[p-1].M = poolState[curAlign].M==0?test_M[curAlign]:1;
                poolState[p-1].A = 1;
            } else
            {
                print(curAlign,1,t-1);
                poolState[t-1].M = poolState[curAlign].M==0?test_M[curAlign]:1;
                poolState[t-1].A = 1;
            }
        } else {
            if(short t = isInPool(testAlignment[curAlign]))
            {
                print(curAlign,1,t-1);
                poolState[t-1].M = poolState[curAlign].M==0?test_M[curAlign]:1;
                poolState[t-1].A = 1;
            }
            else{
                lackInterrupt++;
                short newPlace = findAPlaceForImprovedClock();
                pool[newPlace] = testAlignment[curAlign];
                print(curAlign,0,newPlace,1);
                poolState[newPlace].M = poolState[curAlign].M==0?test_M[curAlign]:1;
                poolState[newPlace].A = 1;
            }
        }
    }
}

typedef struct list{
    list* before;
    int align;
    list* next;
}list;

list *freePageList,*changePageList;
int freePageListLength=0,changePageListLength=0;
int free_M[FREEMLENGTH]={-1};
int free_State[FREEMLENGTH]={-1};
int freeMPointer = 0;

short hasEmptyPlacePBA(){
    for(int i=0;i<FREEMLENGTH;i++)
        if(free_M[i]==-1) return i+1;
    return 0;
}

short isInPoolPBA(int align){
    for (int i = 0; i < FREEMLENGTH; ++i)
        if(free_M[i]==align) return i+1;
    return 0;
}

short isInFreePagePBA(int align){
    list* firstElem = freePageList;
    for (int i = 0; i < freePageListLength; ++i) {
        if (firstElem->align == align) {
            return i;
        }
        firstElem = firstElem->next;
    }
    return -1;
}

short isInChangePagePBA(int align){
    list *firstElem = changePageList;
    for (int j = 0; j < changePageListLength; ++j) {
        if(firstElem->align==align){
            return j;
        }
        firstElem = firstElem->next;
    }
    return -1;
}

int getElemOfFreePagePBA(int index){
    if(index == 0){
        if(freePageListLength == 1){
            int t = freePageList->align;
            free(freePageList);
            freePageList = NULL;
            freePageListLength--;
            return t;
        } else{
            list* wait = freePageList;
            int t = wait->align;
            freePageList->next->before = NULL;
            freePageList = freePageList->next;
            free(wait);
            freePageListLength--;
            return t;
        }
    } else {
        list *selectElem = freePageList;
        for (int i = 0; i < index; ++i) {
            selectElem = selectElem->next;
        }
        if(selectElem->before != NULL)
            selectElem->before->next = selectElem->next;
        if(selectElem->next != NULL)
            selectElem->next->before = selectElem->before;
        int t = selectElem->align;
        free(selectElem);
        freePageListLength--;
        return t;
    }
}

int getElemOfChangePagePBA(int index){
    if(index == 0){
        if(changePageListLength == 1){
            int t = changePageList->align;
            free(changePageList);
            changePageList = NULL;
            changePageListLength--;
            return t;
        } else{
            list* wait = changePageList;
            int t = wait->align;
            changePageList->next->before = NULL;
            changePageList = changePageList->next;
            free(wait);
            changePageListLength--;
            return t;
        }
    } else {
        list *selectElem = changePageList;
        for (int i = 0; i < index; ++i) {
            selectElem = selectElem->next;
        }
        if (selectElem->before != NULL)
            selectElem->before->next = selectElem->next;
        if (selectElem->next != NULL)
            selectElem->next->before = selectElem->before;
        int t = selectElem->align;
        free(selectElem);
        changePageListLength--;
        return t;
    }
}

void pushBackToFreePagePBA(int align){
    list* newElem = (list*)malloc(sizeof(list));
    newElem->align = align;
    newElem->before = NULL;
    newElem->next = NULL;

    if (freePageListLength == 0){
        freePageList = newElem;
    } else{
        list* firstElem = freePageList;
        for (int i = 1; i < freePageListLength; ++i) {
            firstElem = firstElem->next;
        }
        firstElem->next = newElem;
        newElem->before = firstElem;
    }

    freePageListLength++;
}

void pushBackToChangePagePBA(int align){
    list* newElem = (list*)malloc(sizeof(list));
    newElem->align = align;
    newElem->before = NULL;
    newElem->next = NULL;

    if (changePageListLength == 0){
        changePageList = newElem;
    } else{
        list* firstElem = changePageList;
        for (int i = 1; i < changePageListLength; ++i) {
            firstElem = firstElem->next;
        }
        firstElem->next = newElem;
        newElem->before = firstElem;
    }
    changePageListLength++;
}

void printPBA(int cur, short hasFind, short newPoint, short inter=0){
    printf("***********************************\n");
    printf("正在执行第%d条指令，需要访问的页号为%d，访问方式为%d。\n",cur,testAlignment[cur],test_M[cur]);
    if(hasFind == 3){
        printf("在缓冲区中找到！位置是%d\n",newPoint);
    } else if(hasFind == 1){
        printf("在空闲列表中找到！位置是%d\n",newPoint);
    } else if (hasFind == 2){
        printf("在修改列表中找到！位置是%d\n",newPoint);
    } else{
        printf("未在两列表中找到！加载其至位置%d\n",newPoint);
    }
    printf("当前缓冲区内容为:");
    for (int i = 0; i <FREEMLENGTH; ++i) {
        printf("%4d",free_M[i]);
    }
    putchar('\n');
    printf("当前缓冲区修改状态为:");
    for (int j = 0; j < FREEMLENGTH; ++j) {
        printf("%4d",free_State[j]);
    }
    putchar('\n');

    printf("当前空闲列表为:");
    list *fr = freePageList;
    for (int k = 0; k < freePageListLength; ++k) {
        printf("%4d",fr->align);
        fr = fr->next;
    }
    putchar('\n');

    printf("当前修改列表为:");
    list *ch = changePageList;
    for (int k = 0; k < changePageListLength; ++k) {
        printf("%4d",ch->align);
        ch = ch->next;
    }
    putchar('\n');

    if(inter)printf("缺页中断!\n");
}

void PBA(){
    printf("操作系统开始分配空闲块到空闲链表：%d个\n",2);
    for (int j = 0; j < 2; ++j) {
        pushBackToFreePagePBA(-1);
    }
    memset(free_M,-1, sizeof(free_M));
    memset(free_State,-1, sizeof(free_State));
    int curAlign = 0;
    for(int i=0;i < testCount; i++,curAlign++)
    {
        short p = hasEmptyPlacePBA();
        if(p)
        {
            short t = isInPoolPBA(testAlignment[curAlign]);
            if(!t)
            {
                free_M[p-1]=testAlignment[curAlign];
                free_State[p-1]=test_M[curAlign];
                printPBA(curAlign,0,p-1);
                continue;
            }
            else
            {
                free_State[t-1] = (free_State[t-1]==0)?test_M[curAlign]:1;
                printPBA(curAlign,3,t-1);
            }
        } else {
            if(short t = isInPoolPBA(testAlignment[curAlign]))
            {
                free_State[t-1] = (free_State[t-1]==0)?test_M[curAlign]:1;
                printPBA(curAlign,3,t-1);
                continue;
            }
            else{
                lackInterrupt++;
                int waitInsert = free_M[freeMPointer];
                if(free_State[freeMPointer] == 0) // 未修改插入到free
                    if(freePageListLength < FREELISTLENGTH){
                        pushBackToFreePagePBA(waitInsert);
                    } else{
                        getElemOfFreePagePBA(0);
                        pushBackToFreePagePBA(waitInsert);
                    }
                else { // 修改插入到change
                    pushBackToChangePagePBA(waitInsert);
                }

                short find;
                find = isInFreePagePBA(testAlignment[curAlign]);
                if(find != -1) {
                    free_M[freeMPointer] = getElemOfFreePagePBA(find);
                    free_State[freeMPointer] = 0;
                    freeMPointer = (freeMPointer+1)%FREEMLENGTH;
                    printPBA(curAlign,1,find+1);
                } else{
                    find = isInChangePagePBA(testAlignment[curAlign]);
                    if(find != -1){
                        free_M[freeMPointer] = getElemOfChangePagePBA(find);
                        freeMPointer = (freeMPointer+1)%FREEMLENGTH;
                        free_State[freeMPointer] = 1;
                        printPBA(curAlign,2,find+1);
                    } else{
                        if(freePageListLength > 0)
                            getElemOfFreePagePBA(0);
                        free_M[freeMPointer] = testAlignment[curAlign];
                        free_State[freeMPointer] = test_M[curAlign];
                        freeMPointer = (freeMPointer+1)%FREEMLENGTH;
                        printPBA(curAlign,0,freeMPointer,1);
                    }
                }

                if(changePageListLength == CHANGELISTLENGTH)
                {
                    printf("修改队列已满，全部写回！\n");
                    list* temp = changePageList;
                    list *tt = temp;
                    for (int ii = 0; ii < changePageListLength; ++ii) {
                        temp = temp->next;
                        if(tt!=NULL) free(tt);
                        tt = temp;
                    }
                    changePageList = NULL;
                    changePageListLength = 0;
                }
            }
        }
    }
}

void createTestAlignment(){
    srand((int)time(0));

    int N = 64;
    int p = 0;
    int e = 5;
    int m = 6;
    int t = 4;

    for (int i = 0; i < testCount/m; ++i) {
        for (int j = i*m; j < i*m+6; ++j) {
            testAlignment[j] = (random(e)+p)%N;
            test_M[j] = random(10)>5?1:0;
        }
        int r = random(10);
        if(r<t){
            p = random(N)%N;
        } else{
            p = (p+1)%N;
        }
    }
}

int main()
{

    testCount = 600;
    createTestAlignment();
    printf("测试指令序列为：\n");
    for (int i = 0; i < testCount; ++i) {
        printf("%3d",testAlignment[i]);
    }
    putchar('\n');
    printf("测试指令访存为：\n");
    for (int j = 0; j < testCount; ++j) {
        printf("%3d",test_M[j]);
    }
    putchar('\n');

    double lackRate[5];
    unsigned long time[5];

    for (int k = 0; k < 5; ++k) {
        memset(pool,-1, sizeof(pool));
        lackInterrupt = 0;

        clock_t start, finish;
        start = clock();
        if(k==0){
            bestExchange();
        } else if(k==1){
            firstInFirstOut();
        } else if(k==2){
            lastRecentUsed();
        } else if(k==3){
            improvedClock();
        } else if(k==4){
            PBA();
        }
        finish = clock();
        lackRate[k] = lackInterrupt*1.0/testCount;
        time[k] = finish - start;
    }
    printf("************************************************\n");
    for (int l = 0; l < 5; ++l) {
        printf("算法%d测试完毕，缺页率为%lf，测试总共耗时%ld个CPU时钟单元\n",l+1,lackRate[l],time[l]);

    }
    return 0;
}