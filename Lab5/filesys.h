//
// Created by Huan Liu on 2019-06-06.
//

#ifndef OSLAB5PLUS_FILESYS_H
#define OSLAB5PLUS_FILESYS_H

#define DIRLEN 48         // 文件名最大长度

#define BLOCKSIZE 512       // 磁盘块大小 512字节
#define SIZE 512000         // 磁盘容量共512000字节
#define BLOCKNUM 1000       // 磁盘总共能够分为1000块 0-999

// 文件控制块
typedef struct FCB{
    char free; // fcb是否被删除
    char filename[DIRLEN]; // 文件名
    char exname[3];     // 拓展名
    unsigned char attribute; // 文件属性字段，0为目录文件，1位数据文件
    unsigned short time;    // 创建日期
    unsigned short first; // 文件起始盘块
    unsigned int length; // 文件长度
} fcb;


// FAT 表
typedef struct FAT {
    short id; // 下一块链接id 0-999
} fat;

// FAB 标识空闲可分配，EOF 标志文件结束，其他数字为下一块编号
#define FREE 0

// 超级块定义
typedef struct superblock {
    // 引导识别项
    char information[20];
    // 盘块大小
    unsigned short blocksize;
    // 盘块数目
    unsigned short blockcount;
    // 根目录起始块
    unsigned short root;
    // 根目录存放文件个数
    unsigned short rootCount;
} superblock;

typedef struct memFileList{
    fcb open_fcb; // 文件 FCB 的内容
    unsigned int count; // 读写指针在文件中的位置
    int dirno; // 打开文件的目录项的盘块号
    char fcbstate; // 是否修改了文件的 FCB 的内容，如果修改了置为 1，否则为 0
}memFileList;

// 最大同时打开的文件
#define MAXOPEN 10

// 初始化FAT文件系统
void initSystem();

// 初始化FCB块
void init_FCB(fcb* newFCB, char *name, char attr, short first);

// 磁盘格式化
void disk_format();

// 创建一个新文件
void create(char *name);

// 寻找一个空磁盘块
unsigned short findFreeBlock();

// 删除给定文件名的文件
void destroy(char *name);

// 查找目录寻找是否存在name的文件
fcb* findFile(char *name);

// 查找给定name文件是否已经打开
memFileList* findOpenFile(char *name);

// 根据打开文件索引号关闭文件
void close(int index);

// 读取文件内容
void read(int index, char* mem_area, int count);

// 写文件内容
void write(int index, char* mem_area, int count);

// 移动文件读写指针
int lseek(int index, unsigned int pos);

// 关闭系统
void closeSystem();

// 人机交互界面
void menu();

// 列目录
void ls();
#endif //OSLAB5PLUS_FILESYS_H
