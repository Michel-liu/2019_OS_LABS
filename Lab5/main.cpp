#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include "filesys.h"

const char USERNAME[] = "liuhuan";

// 虚拟磁盘头指针
unsigned char *virHard;

// 虚拟磁盘各个盘块首地址
unsigned char *blockaddr[BLOCKNUM];

// 超级块声明
superblock initBlock;

// fat格式 fat2备用
fat fat1[BLOCKNUM], fat2[BLOCKNUM];

// 已经使用open打开的文件列表 最后一个保留用于重置本列表
memFileList fileOpened[MAXOPEN+1];

// 当前系统指向的文件号
char curFileOpenedIndex = 0;

void initSystem(){
    // 加载全部磁盘文件到内存中
    virHard = (unsigned char*)malloc(SIZE);

    // 初始化磁盘块指针
    for(int i=0; i<BLOCKNUM; ++i) blockaddr[i] = virHard + i * BLOCKSIZE;

    // fatSysDisk文件作为物理存储设备
    FILE *fp = fopen("fatSysDisk","rb");

    // 是否需要重新格式化
    int need_format = 0;

    // 读取txt中所有信息
    if(fp != NULL)
    {
        unsigned char *buf = (unsigned char*)malloc(SIZE);
        fread(buf, 1, SIZE, fp);
        memcpy(virHard, buf, SIZE);
        memcpy(&initBlock, blockaddr[0], sizeof(superblock));
        if(strcmp(initBlock.information, "000011110000") != 0) need_format = 1;
        free(buf);
        fclose(fp);
    } else need_format = 1;

    // 磁盘块1234,5678中分别加载fat1表和fat2表
    if(!need_format){
        memcpy(fat1, blockaddr[1], sizeof(fat1));
        memcpy(fat2, blockaddr[5], sizeof(fat2));
    } else{
        printf("FAT文件系统不存在，开始格式化！\n");
        disk_format();
    }

    // 程序运行至此，FAT表初始化完毕，blockaddr全部加载或初始化完毕
}

void disk_format(){
    // 写入识别串
    strcpy(initBlock.information, "000011110000");
    // 根目录块为第9块
    initBlock.root = 9;
    // 初始目录为空
    initBlock.rootCount = 0;
    // 磁盘块大小 BLOCKSIZE
    initBlock.blocksize = BLOCKSIZE;
    // 磁盘数目为 BLOCHNUM
    initBlock.blockcount = BLOCKNUM;

    // 初始化第一块内容
    memcpy(blockaddr[0], &initBlock, sizeof(initBlock));

    // fat前9项设置为结束,其余项设置为空
    for(int i=0; i<=initBlock.root; i++) fat1[i].id = EOF;
    for(int i=initBlock.root+1; i<BLOCKNUM; i++) fat1[i].id = FREE;
    for(int i=0; i<BLOCKNUM; i++) fat2[i].id = fat1[i].id;

    // 初始化1-4，5-8块内容
    memcpy(blockaddr[0] + BLOCKSIZE, &fat1, sizeof(fat1));
    memcpy(blockaddr[0] + 5*BLOCKSIZE, &fat2, sizeof(fat2));

    printf("格式化完成！\n");
}

void init_FCB(fcb* newFCB, char *name, char attr, short first){
    strcpy(newFCB->filename,name);
    newFCB->attribute = attr;
    newFCB->first = first;
    newFCB->free = 0;
    newFCB->length = 0;
}

void create(char *name) {
    if(findFile(name)){
        printf("文件重名，请更换文件名%s！\n",name);
        return;
    }
    fcb newfcb;
    init_FCB(&newfcb,name,1,findFreeBlock());
    fat1[newfcb.first].id = EOF;

    int offset = (initBlock.rootCount * sizeof(fcb)) % BLOCKSIZE;
    int blockCount = (initBlock.rootCount * sizeof(fcb)) / BLOCKSIZE;

    if(offset == 0){
        if(blockCount >= 1){
            int newblock = findFreeBlock();
            if(newblock == -1)
            {
                printf("ERROR 磁盘已满！\n");
                return;
            }
            int point = initBlock.root;
            while(--blockCount){
                point = fat1[point].id;
            }
            fat1[point].id = newblock;
            fat1[newblock].id = EOF;
            memcpy(blockaddr[0] + newblock * BLOCKSIZE ,&newfcb, sizeof(fcb));
        } else if(blockCount == 0){
            fat1[initBlock.root].id = EOF;
            memcpy(blockaddr[initBlock.root] , &newfcb, sizeof(fcb));
        }
    } else{
        int point = initBlock.root;
        blockCount++;
        while(--blockCount){
            point = fat1[point].id;
        }
        memcpy(blockaddr[point] + offset , &newfcb, sizeof(fcb));
    }
    initBlock.rootCount++;
    memcpy(blockaddr[0], &initBlock, sizeof(initBlock));
}

unsigned short findFreeBlock(){
    for (int i = initBlock.root; i < BLOCKNUM; ++i) {
        if(fat1[i].id == FREE) return i;
    }
    return -1;
}

unsigned short open(char *name){
    if(findOpenFile(name)){
        printf("文件已经打开，请勿重复打开%s!\n",name);
        return -1;
    }
    int offset = (initBlock.rootCount * sizeof(fcb)) % BLOCKSIZE;
    int blockCount = (initBlock.rootCount * sizeof(fcb))/ BLOCKSIZE;

    fcb tempFcb;

    if(offset == 0)
    {
        if(blockCount >= 1){
            int pointer = initBlock.root;
            while(blockCount--){
                for (int i = 0; i < BLOCKSIZE / sizeof(fcb); ++i) {
                    memcpy(&tempFcb, blockaddr[0] + pointer * BLOCKSIZE + i * sizeof(fcb), sizeof(fcb));
                    if(strcmp(tempFcb.filename,name)==0){
                        memFileList openFileInfo;
                        openFileInfo.open_fcb = tempFcb;
                        openFileInfo.count = 0;
                        openFileInfo.dirno = pointer;
                        fileOpened[curFileOpenedIndex] = openFileInfo;
                        return curFileOpenedIndex++;
                    }
                }
                pointer = fat1[pointer].id;
            }
        } else if(blockCount == 0){
            printf("无法找到文件%s!\n",name);
            return -1;
        }
    } else{
        int pointer = initBlock.root;
        while(blockCount--){
            for (int i = 0; i < BLOCKSIZE / sizeof(fcb); ++i) {
                memcpy(&tempFcb, blockaddr[0] + pointer * BLOCKSIZE + i * sizeof(fcb), sizeof(fcb));
                if(strcmp(tempFcb.filename,name)==0) {
                    memFileList openFileInfo;
                    openFileInfo.open_fcb = tempFcb;
                    openFileInfo.count = 0;
                    openFileInfo.dirno = pointer;
                    fileOpened[curFileOpenedIndex] = openFileInfo;
                    return curFileOpenedIndex++;
                }
            }
            pointer = fat1[pointer].id;
        }

        for (int j = 0; j < offset / sizeof(fcb); ++j) {
            memcpy(&tempFcb, blockaddr[0] + pointer * BLOCKSIZE + j * sizeof(fcb), sizeof(fcb));
            if(strcmp(tempFcb.filename,name)==0) {
                memFileList openFileInfo;
                openFileInfo.open_fcb = tempFcb;
                openFileInfo.count = 0;
                openFileInfo.dirno = pointer;
                fileOpened[curFileOpenedIndex] = openFileInfo;
                return curFileOpenedIndex++;
            }
        }
    }
    printf("无法找到文件%s!\n",name);
    return -1;
}

void destroy(char *name){
    fcb *deletFcb;

    for (int i = 0; i < curFileOpenedIndex; ++i) {
        if(strcmp(name, fileOpened[i].open_fcb.filename) == 0)
        {
            printf("请先关闭文件%s！\n",name);
            return;
        }
    }

    int offset = (initBlock.rootCount * sizeof(fcb)) % BLOCKSIZE;
    int blockCount = (initBlock.rootCount * sizeof(fcb))/ BLOCKSIZE;

    if(offset == 0){
        if(blockCount == 0)
        {
            printf("未找到文件%s！\n",name);
            return;
        } else if(blockCount >= 1){
            int pointer = initBlock.root;
            while(blockCount--){
                for (int i = 0; i < BLOCKSIZE / sizeof(fcb); ++i) {
                    deletFcb = (fcb*)(*blockaddr + pointer * BLOCKSIZE + i * sizeof(fcb));
                    if(strcmp(deletFcb->filename,name) == 0 && !deletFcb->free)
                    {
                        deletFcb->free = 1;
                        printf("已经标记为删除%s！\n", name);
                        return;
                    }
                }
                pointer = fat1[pointer].id;
            }
        }
    }else{
        int pointer = initBlock.root;
        while(blockCount--){
            for (int i = 0; i < BLOCKSIZE / sizeof(fcb); ++i) {
                deletFcb = (fcb*)(*blockaddr + BLOCKSIZE * pointer + i * sizeof(fcb));
                if(strcmp(deletFcb->filename, name) == 0 && !deletFcb->free)
                {
                    deletFcb->free = 1;
                    printf("已经标记为删除%s！\n", name);
                    return;
                }
                pointer = fat1[pointer].id;
            }
        }
        for (int j = 0; j < offset / sizeof(fcb); ++j) {
            deletFcb = (fcb*)(*blockaddr + BLOCKSIZE * pointer + j * sizeof(fcb));
            if(strcmp(deletFcb->filename, name) == 0 && !deletFcb->free)
            {
                deletFcb->free = 1;
                printf("已经标记为删除%s！\n", name);
                return;
            }
        }
    }
    printf("未找到文件%s！\n",name);
}

fcb* findFile(char *name){
    fcb *findFcb;
    int offset = (initBlock.rootCount * sizeof(fcb)) % BLOCKSIZE;
    int blockCount = (initBlock.rootCount * sizeof(fcb))/ BLOCKSIZE;

    if(offset == 0){
        if(blockCount == 0)
        {
            return NULL;
        } else if(blockCount >= 1){
            int pointer = initBlock.root;
            while(blockCount--){
                for (int i = 0; i < BLOCKSIZE / sizeof(fcb); ++i) {
                    findFcb = (fcb*)(*blockaddr + pointer * BLOCKSIZE + i * sizeof(fcb));
                    if(strcmp(findFcb->filename,name) == 0 && !findFcb->free)
                        return findFcb;
                }
                pointer = fat1[pointer].id;
            }
        }
    }else{
        int pointer = initBlock.root;
        while(blockCount--){
            for (int i = 0; i < BLOCKSIZE / sizeof(fcb); ++i) {
                findFcb = (fcb*)(*blockaddr + BLOCKSIZE * pointer + i * sizeof(fcb));
                if(strcmp(findFcb->filename, name) == 0 && !findFcb->free)
                    return findFcb;
                pointer = fat1[pointer].id;
            }
        }
        for (int j = 0; j < offset / sizeof(fcb); ++j) {
            findFcb = (fcb*)(*blockaddr + BLOCKSIZE * pointer + j * sizeof(fcb));
            if(strcmp(findFcb->filename, name) == 0 && !findFcb->free)
                return findFcb;
        }
    }
    return NULL;
}

memFileList* findOpenFile(char *name){
    for (int i = 0; i < curFileOpenedIndex; ++i) {
        if(strcmp(name, fileOpened[i].open_fcb.filename) == 0)
            return fileOpened + i;
    }
    return NULL;
}

void close(int index){
    if(index >= curFileOpenedIndex && index < 0)
    {
        printf("该打开文件不存在%d\n",index);
        return;
    }
    if(fileOpened[index].fcbstate == 1){
        fcb *closeFcb = findFile(fileOpened[index].open_fcb.filename);
        memcpy(closeFcb,&fileOpened[index].open_fcb, sizeof(fcb));
    }
    fileOpened[index] = fileOpened[MAXOPEN];
    curFileOpenedIndex--;
}

void read(int index, char* mem_area, int count){
    if(count <= 0) return;
    if(index >= curFileOpenedIndex && index < 0)
    {
        printf("该打开文件不存在%d\n",index);
        return;
    }

    memFileList *readOpenFile = &fileOpened[index];
    fcb readFcb = readOpenFile->open_fcb;
    unsigned int curPointer = readOpenFile->count;

    if(count > readFcb.length - curPointer){
        printf("访问越界！\n");
        return;
    }

    int blockCount_loop = curPointer / BLOCKSIZE;
    int offset_loop = curPointer % BLOCKSIZE;
    int pointer = readFcb.first;

    while (blockCount_loop--) pointer = fat1[pointer].id;
    unsigned char* realCurFilePointer = blockaddr[pointer] + offset_loop;

    if(offset_loop + count <= BLOCKSIZE){
        memcpy(mem_area, realCurFilePointer, sizeof(char)*count);
        readOpenFile->count += count;
    } else{
        memcpy(mem_area, realCurFilePointer, sizeof(char)* (BLOCKSIZE - offset_loop));
        mem_area += (BLOCKSIZE - offset_loop);
        readOpenFile->count += (BLOCKSIZE - offset_loop);
        count -= (BLOCKSIZE - offset_loop);

        int movBlockCount = count / BLOCKSIZE;
        int offset = count % BLOCKSIZE;

        pointer = fat1[pointer].id;
        while(movBlockCount--){
            memcpy(mem_area, blockaddr[pointer], sizeof(char) * BLOCKSIZE);
            mem_area += BLOCKSIZE;
            pointer = fat1[pointer].id;
        }

        if(offset != 0){
            memcpy(mem_area, blockaddr[pointer], sizeof(char) * offset);
        }
        readOpenFile->count += count;
    }
}

int lseek(int index, unsigned int pos){
    if(index >= curFileOpenedIndex && index < 0)
    {
        printf("该打开文件不存在%d\n",index);
        return 0;
    }
    if(pos > fileOpened[index].open_fcb.length || pos < 0)
    {
        printf("修改文件指针越界%d\n",pos);
        return 0;
    }
    fileOpened[index].count = pos;
    return 1;
}

void write(int index, char* mem_area, int count){
    if(count <= 0) return;
    if(index >= curFileOpenedIndex && index < 0)
    {
        printf("该打开文件不存在%d\n",index);
        return;
    }

    for (int j = 0; j < BLOCKNUM; ++j) {
        fat2[j] = fat1[j];
    }

    memFileList *openFile = &fileOpened[index];
    fcb openFcb = openFile->open_fcb;
    
    int needNewBlock = 0;

    int blockCount_old = openFcb.length / BLOCKSIZE;
    blockCount_old == 0?blockCount_old=1:0;
    int offset_old = openFcb.length % BLOCKSIZE;
    if(offset_old) blockCount_old++;

    int newLength = openFcb.length + count;
    int blockCount_new = newLength / BLOCKSIZE;
    int offset_new = newLength % BLOCKSIZE;
    if(offset_new) blockCount_new++;
    
    if(blockCount_new > blockCount_old) needNewBlock = 1;

    int pointer = openFcb.first;
    while(fat1[pointer].id != EOF){
        pointer = fat1[pointer].id;
    }

    if(needNewBlock){
        for (int i = 0; i < blockCount_new - blockCount_old; ++i) {
            fat1[pointer].id = findFreeBlock();
            if(fat1[pointer].id == -1) {
                printf("磁盘空间已满，请删除文件后再试！\n");
                for (int j = 0; j < BLOCKNUM; ++j) {
                    fat1[j] = fat2[j];
                }
                return;
            }
            pointer = fat1[pointer].id;
        }
        fat1[pointer].id = EOF;
    }

    openFile->fcbstate = 1;

    int tempMemLength = openFcb.length-openFile->count;
    char *tempMem = (char*)malloc(sizeof(char)*(openFcb.length-openFile->count));
    int save = openFcb.length-openFile->count;
    read(index,tempMem,sizeof(char)*(openFcb.length-openFile->count));
    openFile->count -= save;

    int blockCount_curPoint = openFile->count / BLOCKSIZE;
    int offset_curPoint = openFile->count % BLOCKSIZE;
    if(offset_curPoint) blockCount_curPoint++;
    if(offset_curPoint == 0 && blockCount_curPoint == 0) blockCount_curPoint = 1;
    pointer = openFcb.first;

    while(--blockCount_curPoint){
        pointer = fat1[pointer].id;
    }

    if(!needNewBlock){
        memcpy(blockaddr[pointer]+offset_curPoint, mem_area, sizeof(char)*count);
        memcpy(blockaddr[pointer]+offset_curPoint+count, tempMem, sizeof(char)*(openFcb.length-openFile->count));
        openFile->count += count;
        openFile->open_fcb.length += count;
    } else{
        if(offset_curPoint + count > BLOCKSIZE){
            memcpy(blockaddr[pointer]+offset_curPoint, mem_area, sizeof(char)*(BLOCKSIZE-offset_curPoint));
            openFile->count += (BLOCKSIZE-offset_curPoint);
            mem_area += (BLOCKSIZE-offset_curPoint);
            openFile->open_fcb.length += (BLOCKSIZE-offset_curPoint);
            count -= (BLOCKSIZE-offset_curPoint);

            int tempBlockCount = count / BLOCKSIZE;
            int tempOffset = count % BLOCKSIZE;
            pointer = fat1[pointer].id;

            while(tempBlockCount--){
                memcpy(blockaddr[pointer], mem_area, sizeof(char) * BLOCKSIZE);
                mem_area += BLOCKSIZE;
                pointer = fat1[pointer].id;
            }

            memcpy(blockaddr[pointer], mem_area, sizeof(char)*tempOffset);
            mem_area += tempOffset;
        } else{
            memcpy(blockaddr[pointer]+offset_curPoint, mem_area, sizeof(char)*count);
        }

        openFile->count += count;

        blockCount_curPoint = openFile->count / BLOCKSIZE;
        offset_curPoint = openFile->count % BLOCKSIZE;
        if(offset_curPoint) blockCount_curPoint++;
        pointer = openFcb.first;

        while(--blockCount_curPoint){
            pointer = fat1[pointer].id;
        }

        if(offset_curPoint + tempMemLength > BLOCKSIZE){
            memcpy(blockaddr[pointer]+offset_curPoint, tempMem, sizeof(char)*(BLOCKSIZE-offset_curPoint));
            tempMem += (BLOCKSIZE-offset_curPoint);
            tempMemLength -= (BLOCKSIZE-offset_curPoint);

            int tempBlockCount = count / BLOCKSIZE;
            int tempOffset = count % BLOCKSIZE;
            pointer = fat1[pointer].id;

            while(tempBlockCount--){
                memcpy(blockaddr[pointer], tempMem, sizeof(char) * BLOCKSIZE);
                tempMem += BLOCKSIZE;
                pointer = fat1[pointer].id;
            }

            memcpy(blockaddr[pointer], mem_area, sizeof(char)*tempOffset);
            tempMem += tempOffset;
        } else{
            memcpy(blockaddr[pointer]+offset_curPoint, tempMem, sizeof(char)*tempMemLength);
        }
        openFile->open_fcb.length += count;
        openFile->count += tempMemLength;
    }
}

void closeSystem(){
    printf("正在关闭所有打开的文件!\n");
    for (int i = 0; i < curFileOpenedIndex; ++i) {
        close(i);
    }
    printf("关闭完毕！\n");

    memcpy(blockaddr[1],fat1, sizeof(fat1));
    memcpy(blockaddr[5],fat1, sizeof(fat1));


    FILE *fp = fopen("fatSysDisk","wb");
    for (int j = 0; j < BLOCKNUM; ++j) {
        fwrite(blockaddr[j],1,BLOCKSIZE,fp);
    }
    fclose(fp);
}

void menu(){
    char command[DIRLEN*2];
    while(1){
        printf("%s@mac-desktop /$ ", USERNAME);
        scanf("%s", command);
        if(strcmp(command, "exit")==0)
        {
            break;
        } else if(strcmp(command, "create") == 0){
            printf("请输入想要创建的文件名:");
            scanf("%s", command);
            if(strlen(command) > DIRLEN){
                printf("文件名过长！\n");
                continue;
            }
            create(command);
            continue;
        } else if(strcmp(command, "open") == 0){
            printf("请输入想要打开的文件名:");
            scanf("%s", command);
            if(strlen(command) > DIRLEN){
                printf("文件名过长！\n");
                continue;
            }
            int t = open(command);
            t == -1?0:printf("文件已经打开，序号为%d\n",t);
            continue;
        } else if(strcmp(command, "close")==0){
            printf("请输入想关闭的文件序号:");
            int index;
            scanf("%d",&index);
            close(index);
            continue;
        } else if(strcmp(command, "write") == 0){
            printf("请输入要写入文件的序号");
            int index, writePoint;
            scanf("%d", &index);
            printf("请输入读写指针位置:");
            scanf("%d", &writePoint);
            if(lseek(index, writePoint))
            {
                printf("请输入写入内容：");
                char temp[SIZE/2];
                scanf("%s",temp);
                write(index,temp,strlen(temp));
            }
            continue;
        } else if(strcmp(command, "read") == 0){
            printf("请输入要读入文件的序号");
            int index, readPoint;
            scanf("%d", &index);
            printf("请输入读写指针位置:");
            scanf("%d", &readPoint);
            if(lseek(index, readPoint))
            {
                printf("请输入要读取字节数：");
                int count;
                scanf("%d",&count);
                char temp[count+1];
                temp[count] = '\0';
                read(index,temp,count);
                printf("读入内容为:\n%s\n",temp);
            }
            continue;
        } else if(strcmp(command, "ls") == 0){
            ls();
            continue;
        } else if(strcmp(command, "delete") == 0){
            printf("请输入想要删除的文件名:");
            scanf("%s", command);
            if(strlen(command) > DIRLEN){
                printf("文件名过长！\n");
                continue;
            }
            destroy(command);
            continue;
        } else{
            printf("命令有误！\n");
            continue;
        }
    }
}

void ls(){
    fcb *findFcb;
    int offset = (initBlock.rootCount * sizeof(fcb)) % BLOCKSIZE;
    int blockCount = (initBlock.rootCount * sizeof(fcb))/ BLOCKSIZE;

    if(offset == 0){
        if(blockCount == 0)
        {
            printf("当前目录为空!\n");
        } else if(blockCount >= 1){
            int pointer = initBlock.root;
            while(blockCount--){
                for (int i = 0; i < BLOCKSIZE / sizeof(fcb); ++i) {
                    findFcb = (fcb*)(*blockaddr + pointer * BLOCKSIZE + i * sizeof(fcb));
                    if(!findFcb->free)
                        printf("文件名: %s 字节数: %d\n",findFcb->filename,findFcb->length);
                }
                pointer = fat1[pointer].id;
            }
        }
    }else{
        int pointer = initBlock.root;
        while(blockCount--){
            for (int i = 0; i < BLOCKSIZE / sizeof(fcb); ++i) {
                findFcb = (fcb*)(*blockaddr + BLOCKSIZE * pointer + i * sizeof(fcb));
                if(!findFcb->free)
                    printf("文件名: %s 字节数: %d\n",findFcb->filename,findFcb->length);
                pointer = fat1[pointer].id;
            }
        }
        for (int j = 0; j < offset / sizeof(fcb); ++j) {
            findFcb = (fcb*)(*blockaddr + BLOCKSIZE * pointer + j * sizeof(fcb));
            if(!findFcb->free)
                printf("文件名: %s 字节数: %d\n",findFcb->filename,findFcb->length);
        }
    }
}

int main() {


    // 初始化或加载FAT文件系统
    initSystem();
    menu();
    closeSystem();
    return 0;
}