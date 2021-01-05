#define _CRT_SECURE_NO_WARNINGS 1
#pragma once
#include<cassert>
#include<cstdlib>
#include<algorithm>
#include<vector>

#include<cstdio>
#include<cstring>
#include<conio.h>
#include <ctime>
#include <iostream>
#include <ostream>
#include <string>
#include <io.h>
#include <direct.h>
#include <vector>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <cmath>
#include <windows.h>


using namespace std;

const size_t N =2* 32 * 1024;
enum { SlipBlock = 32 * 1024 };

typedef long long LongType;

class lzCompress
{
public:
    lzCompress()
    {
        _windows.reserve(N);             //开辟一个窗口，大小是N
        _frist = 0;
        _last = 0;
    }

    string Compress(const string& filename)
    {
        return lz_Compress(filename);
    }

    string UnCompress(const string& filename)
    {
        return lzUnCompress(filename);
    }

protected:
    string lz_Compress(const string& filename)     //将原文件传入
    {
        assert(filename.c_str());
        string FirstCompressFileName = filename;
        FirstCompressFileName =  FirstCompressFileName+".zip";             //加一个后缀  .zip

        FILE *fInput = fopen(filename.c_str(), "rb+");
        assert(fInput);

        FILE *fOut = fopen(FirstCompressFileName.c_str(), "wb+");
        assert(fOut);

        FILE *pWindows = fopen(filename.c_str(), "rb+");  //定义指向滑动窗口起始位置的指针
        assert(pWindows);

        int ch = 0;
        ch = fgetc(fInput);

        LongType count = 0;
        //从原文件中读入字符，再判断需不需要进行压缩，只有当重复的字符出现3个以上时才压缩
        unsigned char buf = 0;
        int flag = 7;     //标记buf处理了几位

        while (ch != EOF)
        {
            long distance = 0;
            int length = 0;
            long OFFSET = ftell(fInput);          //文件的当前位置距离起始位置的偏移量

            //设置滑动窗口的大小
            if (OFFSET > SlipBlock)
            {
                fseek(pWindows, OFFSET - 1 - SlipBlock, SEEK_SET);  //文件指针退回到滑动窗口的起始位置
                distance = SlipBlock;
            }
            else
            {
                fseek(pWindows, 0, SEEK_SET);   //如果当前位置偏移量没有滑块大,将窗口指针设置在初始位置
                distance = OFFSET - 1;
            }

            if (distance > 1)
            {
                fseek(fInput, OFFSET - 1, SEEK_SET);    //向前退回一个字符
                length = FindRepetition(pWindows, fInput, distance);
                fseek(fInput, OFFSET, SEEK_SET);     //前进到原来位置
            }

            if (length>0)             //有重复的,用1表示有重复的
            {
                fseek(fInput, length - 1, SEEK_CUR);

                buf |= (1 << flag);        //先把flag这一位标记 设置成1
                flag--;
                if (flag < 0)    //buf这8位以经处理完毕，进行写入
                {
                    fputc(buf, fOut);
                    flag = 7;
                    buf = 0;
                }

                //接下来把distance和length写入
                int pos = 15;
                while (pos >= 0)                    //处理两个字节的distance
                {
                    if (distance&(1 << pos))        //如果length的第pos位为1
                        buf |= (1 << flag);         //向buf中写1
                    else
                        buf &= (~(1 << flag));       //向buf中写0
                    flag--;
                    if (flag < 0)                    //buf这8位以经处理完毕，进行写入
                    {
                        fputc(buf, fOut);
                        flag = 7;
                        buf = 0;
                    }
                    pos--;
                }

                pos = 7;                             //接下来写入length
                while (pos >= 0)
                {
                    if (length&(1 << pos))           //如果length的第pos位为1
                        buf |= (1 << flag);           //向buf中写1
                    else
                        buf &= (~(1 << flag));       //向buf中写0
                    flag--;
                    if (flag < 0)                    //buf这8位以经处理完毕，进行写入
                    {
                        fputc(buf, fOut);
                        flag = 7;
                        buf = 0;
                    }
                    pos--;
                }
                count += 3;              //处理一个distance和一个length，count加三个字节
            }
            else                             //这个字符是普通字符
            {
                buf &= (~(1 << flag));       //把flag这一位设置成0
                flag--;
                if (flag < 0)                //buf这8位已经处理完毕，进行写入
                {
                    fputc(buf, fOut);
                    flag = 7;
                    buf = 0;
                }

                //接下来处理ch这个字符
                int pos = 7;
                while (pos >= 0)
                {
                    if (ch&(1 << pos))                //如果ch的第pos位为1
                        buf |= (1 << flag);           //向buf中写1
                    else
                        buf &= (~(1 << flag));        //向buf中写0
                    flag--;
                    if (flag < 0)                     //buf这8位以经处理完毕，进行写入
                    {
                        fputc(buf, fOut);
                        flag = 7;
                        buf = 0;
                    }
                    pos--;
                }
                count++;              //处理一个字符，count++一次
            }
            ch = fgetc(fInput);
        }

        if (flag != 7)       //如果最后的bit位不够一个整数，则就在后面补0
        {
            fputc(buf, fOut);
        }
        fwrite(&count, 1, 8, fOut);
        fclose(fInput);
        fclose(fOut);
        fclose(pWindows);                             //关闭窗口指针所指向的文件
        return FirstCompressFileName;
    }

    //LZ77解压
    string lzUnCompress(const string& CompressFileName)        //将要解压的文件传入
    {
        string UnCompressFileName = CompressFileName;
        UnCompressFileName = UnCompressFileName.substr(0,UnCompressFileName.length()-4);

        FILE* fInput = fopen(CompressFileName.c_str(), "rb+");
        assert(fInput);

        FILE* fOut = fopen(UnCompressFileName.c_str(), "wb+");
        assert(fOut);

        FILE* pWindows = fopen(UnCompressFileName.c_str(), "rb+");  //定义窗口指针
        assert(pWindows);

        LongType count;
        fseek(fInput, -8, SEEK_END);
        fread(&count, 1, 8, fInput);          //count保存的是字符的个数

        fseek(fInput, 0, SEEK_SET);
        //解压缩
        int c = 0;
        int ch = 0;
        ch = fgetc(fInput);

        unsigned char buf = 0;
        int status = 0;             //用来记录现在是处理字符还是距离和长度
        int flag = 7;               //记录buf处理到了第几位
        while (count>0)
        {
            int distance = 0;
            int length = 0;
            status = ch&(1 << flag);       //判断状态
            flag--;
            if (flag < 0)
            {
                ch = fgetc(fInput);
                flag = 7;
            }

            if (status != 0)            //这一位为1，表示距离和长度
            {
                //还原distance，连续读取两个字节
                int pos = 15;
                while (pos >= 0)
                {
                    if (ch&(1 << flag))
                        distance |= (1 << pos);           //在这一位写1
                    else
                        distance &= (~(1 << pos));           //在这一位写0
                    flag--;
                    if (flag < 0)
                    {
                        ch = fgetc(fInput);
                        flag = 7;
                    }
                    pos--;
                }
                //读取length
                pos = 7;
                while (pos >= 0)
                {
                    if (ch&(1 << flag))
                        length |= (1 << pos);           //在这一位写1
                    else
                        length &= (~(1 << pos));           //在这一位写0
                    flag--;
                    if (flag < 0)
                    {
                        ch = fgetc(fInput);
                        flag = 7;
                    }
                    pos--;
                }

                //复制滑动窗口中重复的字符
                fflush(fOut);                                  //将缓冲区的内容全部都写入文件
                int OFFSET = ftell(fOut);                      //记录这个位置，写入的这个位置
                fseek(pWindows, OFFSET - distance, SEEK_SET);  //让窗口指针指向窗口起始位置
                while (length--)
                {
                    int c = fgetc(pWindows);
                    fputc(c, fOut);
                }
                count -= 3;
            }
            else              //原字符
            {
                int pos = 7;
                while (pos >= 0)
                {
                    if (ch&(1 << flag))
                        buf |= (1 << pos);           //在这一位写1
                    else
                        buf &= (~(1 << pos));           //在这一位写0
                    flag--;
                    if (flag < 0)
                    {
                        ch = fgetc(fInput);
                        flag = 7;
                    }
                    pos--;
                }
                fputc(buf, fOut);
                count--;
                buf = 0;
            }
        }
        fclose(fInput);
        fclose(fOut);
        fclose(pWindows);
        return UnCompressFileName;
    }
protected:
    int FindRepetition(FILE* pWindows, FILE *fInput, long& distance)
    {
        long OFFSET1 = ftell(pWindows);     //记录窗口距离文件开始的距离
        long OFFSET2 = ftell(fInput);       //记录当前要比较的字符串距离文件开始的距离
        int ch = 0;

        if ((size_t)OFFSET2>_last)
        {
            _windows.clear();
            for (size_t i = 0; i<N; i++)
            {
                ch = fgetc(pWindows);
                if (ch == EOF)
                    break;
                _windows.push_back(ch);
            }
            _frist = OFFSET1;           //记录加载到窗口的数据的起始位置
            _last = _windows.size() + OFFSET1;
        }

        int length = GetRepetionlength(fInput, distance, OFFSET1);
        return length;
    }

    int  GetRepetionlength(FILE *fInput, long& distance, long pWindowsPos)     //得到重复的长度
    {
        long OFFSET = ftell(fInput);                //得到要比较的字符的位置

        vector<unsigned char> v;
        if (Getch(fInput, v) == false)
            return 0;

        size_t size = OFFSET - pWindowsPos;
        size_t index = pWindowsPos - _frist;

        int length = 0;

        for (; index<size; ++index)
        {
            if (_windows[index] == v[0])
            {
                size_t flag = index;
                size_t i = 0;
                while ((flag < size) && (length<255))
                {
                    if (i == v.size() - 1)
                    {
                        if (Getch(fInput, v) == false)
                            break;
                    }
                    if (_windows[flag] == v[i])
                    {
                        length++;
                        flag++;
                        i++;
                    }
                    else
                        break;
                }
                if (length >=3)
                {
                    distance = OFFSET - (index + _frist);
                    return length;             //如果重复出现的字符的长度大于3，则就返回重复长度
                }
                length = 0;
            }
        }
        return 0;
    }

    bool Getch(FILE *fInput, vector<unsigned char>& v)
    {
        int ch = 0;

        ch = fgetc(fInput);
        v.push_back(ch);

        if (ch == EOF)
            return false;
        else
            return true;
    }

private:
    vector<unsigned char> _windows;
    size_t _frist;
    size_t _last;
};


int test = false;

//哈夫曼树节点
typedef struct huffNode
{
    int parent,lchild,rchild; //二叉树关系
    unsigned long count;      //符号个数
    unsigned char alpha;          //符号
    char code[520];           //编码

}HuffNode;

//存放文件中各个字符，及其出现次数
typedef struct ascll{
    unsigned char alpha;          //符号
    unsigned long count;      //符号个数
}Ascll;

//这个存放字符及字符对应的编码
/*
typedef struct huffTable{
    unsigned char alpha;          //符号
    char code[520];           //编码
}HuffTable;
*/

//展示交互界面
/*void showGUI()
{
    cout<<"                         压缩、解压缩工具                 \n\n";
    cout<<"功能:"<<endl;
    cout<<"     1.压缩"<<endl;
    cout<<"     2.解压缩"<<endl;
    cout<<"     3.输出编码"<<endl;
    cout<<"     4.测试ZIP"<<endl;
    cout<<"     5.退出"<<endl;
    cout<<endl;
    cout<<"注意：使用本程序压缩后压缩文件拓展名为.gr。"<<endl;
    cout<<"      压缩和解压时请输入完整的文件路径。"<<endl;
    cout<<endl;
    cout<<"请选择操作：";
}
*/
class huff{
    private:
        void select(HuffNode* HT, int i, int* s1, int* s2);
        int creatHuffmanTree(HuffNode* HT, Ascll* ascll);
        void HuffmanCoding(char* hTable[256], HuffNode* HT, int leafNum);
    public:
        void compress(string filename);
        void decompress(string filename);

};


void huff::select(HuffNode* HT, int i, int* s1, int* s2)
{
    unsigned int j, s;
    s = 0; //记录当前找到的最小权值的结点的下标
    for(j=1;j<=i;j++)
    {
        if(HT[j].parent == 0)   //找最小
        {
            if(s==0) //第一个找到的点
                s=j;
            if(HT[j].count < HT[s].count)
                s=j;
        }
    }
    *s1 = s;

    s = 0;
    for(j=1;j<=i;j++)   //找次小
    {
        if((HT[j].parent == 0)&&(j!=*s1)) //仅比上面一个多了J!=*s1，应为不能是最小
        {
            if(s==0)
                s=j;
            if(HT[j].count < HT[s].count)
                s=j;
        }
    }
    *s2 = s;
}
//创建的哈夫曼树是以一维数组建立，同时起始地址是1。
int huff::creatHuffmanTree(HuffNode* HT, Ascll* ascll)
{
    int i,s1,s2,leafNum=0,j=0;

    //初始化叶节点,256个ascll字符
    for(i = 0; i < 256; i ++)
    {
        //只使用出现的过的字符 ascll[i].count > 0
        if(ascll[i].count > 0)
        {
            HT[++j].count = ascll[i].count;
            HT[j].alpha = ascll[i].alpha;
            HT[j].parent=HT[j].lchild=HT[j].rchild=0;
        }
    }
    // [叶子] [叶子] [叶子] [叶子] ···[内部] [根]
    leafNum = j;
    int nodeNum = 2*leafNum -1; //节点个数

    //初始化内部节点
    for(i = leafNum + 1; i <= nodeNum; i++)
    {
        HT[i].count = 0;
        HT[i].code[0] = 0;
        HT[i].parent = HT[i].lchild = HT[i].rchild = 0;
    }
    //给内部节点找孩子
    for(i = leafNum + 1; i <= nodeNum; i++)
    {
        select(HT, i - 1, &s1, &s2); //找到当前最小和次小的树根
        HT[s1].parent=i;
        HT[s2].parent=i;
        HT[i].lchild=s2;
        HT[i].rchild=s1;
        HT[i].count=HT[s1].count+HT[s2].count;
    }
    return leafNum;
}

//哈弗曼编码
void huff::HuffmanCoding(char* hTable[256], HuffNode* HT, int leafNum)
{
    int i,j,m,c,f,start;
    char cd[520];
    m = 520;
    cd[m-1] = 0;
    for(i=1;i <= leafNum;i++)
    {
        start = m-1;
        //先是从后往前编码，从子叶开始编码
        for(c=i,f=HT[c].parent; f!=0; c=f,f=HT[f].parent) //找爸爸
        {   //判断自己c是爸爸的哪个孩子
            if(HT[f].lchild==c)
            {
                // 左 0
                cd[start--]='0';
            }
            else
            {
                // 右 1
                cd[start--]='1';
            }
        }
        // [0 0 0 0 0 start 0 1 0 1 1], start 表示偏移，m-start 表示压入的01的长度，start到达根
        start++;
        //int end = m-1;
        for(j=0;j<m-start;j++)
        {
            // 获取字符编码
            HT[i].code[j]=cd[start+j];
            // 编码 [叶子]---[根]
            //HT[i].code[j]=cd[end--];
        }
        // 添加结尾
        HT[i].code[j]='\0';
        //写入字符-频数表
        hTable[ HT[i].alpha ] = HT[i].code;
    }
}

void huff::compress(string filename)
{
    bool compress =TRUE;
    FILE *infile = NULL,
         *outfile = NULL;
    char infileName[520],outfileName[520];

    // 打开文件
    infile = fopen(filename.c_str(),"rb");
    // 创建文件名
    strcpy(outfileName,filename.c_str());
    strcat(outfileName,".gr");
    // 判断文件是否存在
    // 对文件进行操作，判断文件是否存在
    //判断是否可以创建该文件，如果不行，表示无法再文件系统中创建该文件。输入内容有误
    outfile = fopen(outfileName,"wb");
    if(outfile == NULL)
    {
        cout<<"\n无法创建该压缩文件..."<<endl;
        exit(-4);
    }

    cout<<"文件压缩中..."<<endl;

    //[TIME-START]
    const double begin=(double)clock()/CLK_TCK;

    //统计字符种类数和频数
    unsigned char c;
    int i,k;
    unsigned long total=0;              //文件长度

    // 利用hash表存放字母表及字母出现频数
    Ascll ascll[256];
    for(i = 0; i < 256; i++)
    {
        ascll[i].count = 0;
    }

    while(!feof(infile))
    {
        c=fgetc(infile);
        ascll[c].alpha = c;
        ascll[c].count++;
        total++; //读取到的字符个数
    }

    total--;
//   ascll[c].count--; //TODO
// 创建 哈弗曼树节点数组
    HuffNode HT[520];
    int leafNum = creatHuffmanTree(HT,ascll);

    char *hTable[520];
    for(i = 0; i < 256; i ++)
    {
        hTable[i] = new char[520];
    }
// 哈夫曼编码
    HuffmanCoding(hTable, HT, leafNum);

    /*if(!compress){
        cout<<"字母\t字频数表\t编码\t"<<endl;
        for(i = 0; i < 256; i ++){
            if(ascll[i].count > 0){
                cout<<ascll[i].alpha<<"\t"<<ascll[i].count<<"\t"<<hTable[ascll[i].alpha]<<endl;
            }
        }
        // 关闭打开的文件
        fclose(infile);
        fclose(outfile);
        //[TIME-END]
        const double end=(double)clock()/CLK_TCK;
        cout<<"编码耗时："<<(end-begin)<<" s"<<endl;
        return;
    }*/

//写头文件 -- 将压缩的哈夫曼树编码信息写入哈夫曼树
    fseek(infile,0,0);
    fwrite(&total,sizeof(unsigned long),1,outfile);          //原文件总长度

    for(i=0; i<=255; i++)
    {
        // 将哈夫曼树按照 unsigned long 的方式压入文件中，下标表示字母，数值表示字母的频数
        fwrite(&ascll[i].count,sizeof(unsigned long),1,outfile);
    }

//开始压缩主文件
    //char buf[520];
    unsigned long j=0;             //最大为total
   // buf[0]=0;
    string buf = "\0";
    int charNum=2;

    while(!feof(infile))
    {
        c=fgetc(infile);

    //  cout<<c<<endl;
    //  cout<<hTable[c]<<endl;
        string tempCode = hTable[c];
        j++;
       // strcat(buf,tempCode);
        buf += tempCode;
        //k=strlen(buf);
        k = buf.length();
        c=0;
        // 将所得的 0 1 每8个就可以构建一个字母的方式压入文件用
        while(k>=8)
        {
            for(i=0; i<8; i++)
            {
                // 利用左移以为在右边空出一个空位
                // 利用与 1 取或的方式压入 bit 1
                if(buf[i]=='1')
                    c=(c<<1)|1;
                else
                    c=c<<1;
            }
        //  cout<<c<<endl;
            fwrite(&c,sizeof(unsigned char),1,outfile);

            charNum ++;
         //   strcpy(buf,buf+8);
            buf.erase(0,8);
            //k=strlen(buf);
            // 确定剩下的bit 的长度，如果大于8表示还可以压成一个字节
            k = buf.length();
        }
        if(j==total){
            break;
        }
    }
    // 当 k < 8 时，表示还剩下不足 8 位的bit，需要拓展0位压缩
    if(k>0)            //可能还有剩余字符
    {
      //  strcat(buf,"00000000");
        buf += "00000000";
        for(i=0; i<8; i++)
        {
            if(buf[i]=='1')
                c=(c<<1)|1;
            else
                c=c<<1;
        }
        fwrite(&c,sizeof(unsigned char),1,outfile);
        charNum ++;
    }
// 关闭打开的文件
    fclose(infile);
    fclose(outfile);

    //[TIME-END]
    const double end=(double)clock()/CLK_TCK;

    cout<<"压缩成功！"<<endl;
    float s;
    s=(float)charNum / (float)total;
    cout<<"压缩率为："<<s<<endl;
    cout<<"耗时为："<<(end-begin)<<" s"<<endl;
    _getch();
    return;
}

void huff::decompress(string filename) {
     FILE *infile,*outfile;
    char infilename[255],outfilename[255];

    infile = fopen(filename.c_str(),"rb");
    //循环判断文件是否存在
    /*while(infile==NULL){
        char option;
        cout<<"文件"<<infilename<<"不存在...\n";
        cout<<"重新输入文件名（1）或返回主菜单（2）？";
        cin>>option;
        while(option!='1' && option!='2')
        {
            cout<<"\n无效的输入！\n";
            cout<<"重新输入文件名（1）或返回主菜单（2）？";
            cin>>option;
        }
        if(option  == '2'){
            return;
        }
        else {
            cout<<"\n请输入要解压的文件的文件路径(不含.gr)：";

            cin>>outfilename;

            // 构建解压文件名
            strcpy(infilename,outfilename);
            strcat(infilename,".gr");

            infile = fopen(infilename,"rb");
        }
    }*/
    // 输入解压后的文件名
    // cout<<"请输入解压后的文件的文件路径：";
    // cin>>outfilename;

    outfile = fopen(filename.substr(0,filename.length()-3).c_str(),"wb");
    if(outfile==NULL) {
        cout<<"\n解压文件失败！无法创建解压后的文件...";
        return;
    }
    cout<<"解压文件中..."<<endl;
    //[TIME-BEGIN]
    const double begin=(double)clock()/CLK_TCK;

    unsigned long total = 0;
    // 将第一个 long 长度数据读入 tatol 中，为文件的总大小
    fread(&total,sizeof(unsigned long),1,infile);
    //cout<<"原来大小: "<<total<<endl;
    Ascll ascll[256];
    int i;
    for(i = 0; i < 256; i++) {
        // 之后的每个long长度都是一个字符的频数
        fread(&ascll[i].count,sizeof(unsigned long),1,infile);
        ascll[i].alpha = i;
    }

    HuffNode HT[520];
    // 创建哈夫曼树
    int leafNum = creatHuffmanTree(HT,ascll);
    //cout<<"leafNum = "<<leafNum<<endl;
    if(test){
        for(i = 0; i < 256;i ++){
            if(ascll[i].count > 0){
                cout<<ascll[i].alpha<<"\t"<<ascll[i].count<<endl;
            }
        }
    }
/*
    char *hTable[520];
    for(i = 0; i < 520; i ++) {
        hTable[i] = new char[520];
    }*/

    fseek(infile,sizeof(unsigned long)*257,0);
    unsigned char c = 0;

    int index = 2*leafNum - 1;
    int charNum = 0;
    while(!feof(infile))
    {
        // 按照字母读取
        c=fgetc(infile);
        if(test){
            cout<<"第一个字符："<<c<<endl;
        }
        // 从根节点往叶子走，读取到的是一个 字母（8位）， 所以使用循环8次
        for(i=0; i<8; i++)
        {
            unsigned int cod = (c & 128);
            c = c << 1;
            if(cod == 0 ){
                // 左 0 右 1
                index = HT[index].lchild;
            }
            else{
                index = HT[index].rchild;
            }
            if(HT[index].rchild == 0 && HT[index].lchild == 0){

                charNum ++;
                // 到达叶子
                char trueChar = HT[index].alpha;
                fwrite(&trueChar,sizeof(unsigned char),1,outfile);
                // index 重新指向根节点
                index = 2*leafNum - 1;
                if(charNum >= total){
                    break;
                }
            }
        }
        if(charNum >= total){
            break;
        }
    }

    // 关闭打开的文件
    fclose(infile);
    fclose(outfile);

    //[TIME-END]
    const double end=(double)clock()/CLK_TCK;

    cout<<"解压成功"<<endl;
    float s;
    s=(float)charNum / (float)total;
    cout<<"完整度为："<<s<<endl;
    cout<<"耗时为："<<(end-begin)<<" s"<<endl;
    return;
}




