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
        _windows.reserve(N);             //����һ�����ڣ���С��N
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
    string lz_Compress(const string& filename)     //��ԭ�ļ�����
    {
        assert(filename.c_str());
        string FirstCompressFileName = filename;
        FirstCompressFileName =  FirstCompressFileName+".zip";             //��һ����׺  .zip

        FILE *fInput = fopen(filename.c_str(), "rb+");
        assert(fInput);

        FILE *fOut = fopen(FirstCompressFileName.c_str(), "wb+");
        assert(fOut);

        FILE *pWindows = fopen(filename.c_str(), "rb+");  //����ָ�򻬶�������ʼλ�õ�ָ��
        assert(pWindows);

        int ch = 0;
        ch = fgetc(fInput);

        LongType count = 0;
        //��ԭ�ļ��ж����ַ������ж��費��Ҫ����ѹ����ֻ�е��ظ����ַ�����3������ʱ��ѹ��
        unsigned char buf = 0;
        int flag = 7;     //���buf�����˼�λ

        while (ch != EOF)
        {
            long distance = 0;
            int length = 0;
            long OFFSET = ftell(fInput);          //�ļ��ĵ�ǰλ�þ�����ʼλ�õ�ƫ����

            //���û������ڵĴ�С
            if (OFFSET > SlipBlock)
            {
                fseek(pWindows, OFFSET - 1 - SlipBlock, SEEK_SET);  //�ļ�ָ���˻ص��������ڵ���ʼλ��
                distance = SlipBlock;
            }
            else
            {
                fseek(pWindows, 0, SEEK_SET);   //�����ǰλ��ƫ����û�л����,������ָ�������ڳ�ʼλ��
                distance = OFFSET - 1;
            }

            if (distance > 1)
            {
                fseek(fInput, OFFSET - 1, SEEK_SET);    //��ǰ�˻�һ���ַ�
                length = FindRepetition(pWindows, fInput, distance);
                fseek(fInput, OFFSET, SEEK_SET);     //ǰ����ԭ��λ��
            }

            if (length>0)             //���ظ���,��1��ʾ���ظ���
            {
                fseek(fInput, length - 1, SEEK_CUR);

                buf |= (1 << flag);        //�Ȱ�flag��һλ��� ���ó�1
                flag--;
                if (flag < 0)    //buf��8λ�Ծ�������ϣ�����д��
                {
                    fputc(buf, fOut);
                    flag = 7;
                    buf = 0;
                }

                //��������distance��lengthд��
                int pos = 15;
                while (pos >= 0)                    //���������ֽڵ�distance
                {
                    if (distance&(1 << pos))        //���length�ĵ�posλΪ1
                        buf |= (1 << flag);         //��buf��д1
                    else
                        buf &= (~(1 << flag));       //��buf��д0
                    flag--;
                    if (flag < 0)                    //buf��8λ�Ծ�������ϣ�����д��
                    {
                        fputc(buf, fOut);
                        flag = 7;
                        buf = 0;
                    }
                    pos--;
                }

                pos = 7;                             //������д��length
                while (pos >= 0)
                {
                    if (length&(1 << pos))           //���length�ĵ�posλΪ1
                        buf |= (1 << flag);           //��buf��д1
                    else
                        buf &= (~(1 << flag));       //��buf��д0
                    flag--;
                    if (flag < 0)                    //buf��8λ�Ծ�������ϣ�����д��
                    {
                        fputc(buf, fOut);
                        flag = 7;
                        buf = 0;
                    }
                    pos--;
                }
                count += 3;              //����һ��distance��һ��length��count�������ֽ�
            }
            else                             //����ַ�����ͨ�ַ�
            {
                buf &= (~(1 << flag));       //��flag��һλ���ó�0
                flag--;
                if (flag < 0)                //buf��8λ�Ѿ�������ϣ�����д��
                {
                    fputc(buf, fOut);
                    flag = 7;
                    buf = 0;
                }

                //����������ch����ַ�
                int pos = 7;
                while (pos >= 0)
                {
                    if (ch&(1 << pos))                //���ch�ĵ�posλΪ1
                        buf |= (1 << flag);           //��buf��д1
                    else
                        buf &= (~(1 << flag));        //��buf��д0
                    flag--;
                    if (flag < 0)                     //buf��8λ�Ծ�������ϣ�����д��
                    {
                        fputc(buf, fOut);
                        flag = 7;
                        buf = 0;
                    }
                    pos--;
                }
                count++;              //����һ���ַ���count++һ��
            }
            ch = fgetc(fInput);
        }

        if (flag != 7)       //�������bitλ����һ������������ں��油0
        {
            fputc(buf, fOut);
        }
        fwrite(&count, 1, 8, fOut);
        fclose(fInput);
        fclose(fOut);
        fclose(pWindows);                             //�رմ���ָ����ָ����ļ�
        return FirstCompressFileName;
    }

    //LZ77��ѹ
    string lzUnCompress(const string& CompressFileName)        //��Ҫ��ѹ���ļ�����
    {
        string UnCompressFileName = CompressFileName;
        UnCompressFileName = UnCompressFileName.substr(0,UnCompressFileName.length()-4);

        FILE* fInput = fopen(CompressFileName.c_str(), "rb+");
        assert(fInput);

        FILE* fOut = fopen(UnCompressFileName.c_str(), "wb+");
        assert(fOut);

        FILE* pWindows = fopen(UnCompressFileName.c_str(), "rb+");  //���崰��ָ��
        assert(pWindows);

        LongType count;
        fseek(fInput, -8, SEEK_END);
        fread(&count, 1, 8, fInput);          //count��������ַ��ĸ���

        fseek(fInput, 0, SEEK_SET);
        //��ѹ��
        int c = 0;
        int ch = 0;
        ch = fgetc(fInput);

        unsigned char buf = 0;
        int status = 0;             //������¼�����Ǵ����ַ����Ǿ���ͳ���
        int flag = 7;               //��¼buf�����˵ڼ�λ
        while (count>0)
        {
            int distance = 0;
            int length = 0;
            status = ch&(1 << flag);       //�ж�״̬
            flag--;
            if (flag < 0)
            {
                ch = fgetc(fInput);
                flag = 7;
            }

            if (status != 0)            //��һλΪ1����ʾ����ͳ���
            {
                //��ԭdistance��������ȡ�����ֽ�
                int pos = 15;
                while (pos >= 0)
                {
                    if (ch&(1 << flag))
                        distance |= (1 << pos);           //����һλд1
                    else
                        distance &= (~(1 << pos));           //����һλд0
                    flag--;
                    if (flag < 0)
                    {
                        ch = fgetc(fInput);
                        flag = 7;
                    }
                    pos--;
                }
                //��ȡlength
                pos = 7;
                while (pos >= 0)
                {
                    if (ch&(1 << flag))
                        length |= (1 << pos);           //����һλд1
                    else
                        length &= (~(1 << pos));           //����һλд0
                    flag--;
                    if (flag < 0)
                    {
                        ch = fgetc(fInput);
                        flag = 7;
                    }
                    pos--;
                }

                //���ƻ����������ظ����ַ�
                fflush(fOut);                                  //��������������ȫ����д���ļ�
                int OFFSET = ftell(fOut);                      //��¼���λ�ã�д������λ��
                fseek(pWindows, OFFSET - distance, SEEK_SET);  //�ô���ָ��ָ�򴰿���ʼλ��
                while (length--)
                {
                    int c = fgetc(pWindows);
                    fputc(c, fOut);
                }
                count -= 3;
            }
            else              //ԭ�ַ�
            {
                int pos = 7;
                while (pos >= 0)
                {
                    if (ch&(1 << flag))
                        buf |= (1 << pos);           //����һλд1
                    else
                        buf &= (~(1 << pos));           //����һλд0
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
        long OFFSET1 = ftell(pWindows);     //��¼���ھ����ļ���ʼ�ľ���
        long OFFSET2 = ftell(fInput);       //��¼��ǰҪ�Ƚϵ��ַ��������ļ���ʼ�ľ���
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
            _frist = OFFSET1;           //��¼���ص����ڵ����ݵ���ʼλ��
            _last = _windows.size() + OFFSET1;
        }

        int length = GetRepetionlength(fInput, distance, OFFSET1);
        return length;
    }

    int  GetRepetionlength(FILE *fInput, long& distance, long pWindowsPos)     //�õ��ظ��ĳ���
    {
        long OFFSET = ftell(fInput);                //�õ�Ҫ�Ƚϵ��ַ���λ��

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
                    return length;             //����ظ����ֵ��ַ��ĳ��ȴ���3����ͷ����ظ�����
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

//���������ڵ�
typedef struct huffNode
{
    int parent,lchild,rchild; //��������ϵ
    unsigned long count;      //���Ÿ���
    unsigned char alpha;          //����
    char code[520];           //����

}HuffNode;

//����ļ��и����ַ���������ִ���
typedef struct ascll{
    unsigned char alpha;          //����
    unsigned long count;      //���Ÿ���
}Ascll;

//�������ַ����ַ���Ӧ�ı���
/*
typedef struct huffTable{
    unsigned char alpha;          //����
    char code[520];           //����
}HuffTable;
*/

//չʾ��������
/*void showGUI()
{
    cout<<"                         ѹ������ѹ������                 \n\n";
    cout<<"����:"<<endl;
    cout<<"     1.ѹ��"<<endl;
    cout<<"     2.��ѹ��"<<endl;
    cout<<"     3.�������"<<endl;
    cout<<"     4.����ZIP"<<endl;
    cout<<"     5.�˳�"<<endl;
    cout<<endl;
    cout<<"ע�⣺ʹ�ñ�����ѹ����ѹ���ļ���չ��Ϊ.gr��"<<endl;
    cout<<"      ѹ���ͽ�ѹʱ�������������ļ�·����"<<endl;
    cout<<endl;
    cout<<"��ѡ�������";
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
    s = 0; //��¼��ǰ�ҵ�����СȨֵ�Ľ����±�
    for(j=1;j<=i;j++)
    {
        if(HT[j].parent == 0)   //����С
        {
            if(s==0) //��һ���ҵ��ĵ�
                s=j;
            if(HT[j].count < HT[s].count)
                s=j;
        }
    }
    *s1 = s;

    s = 0;
    for(j=1;j<=i;j++)   //�Ҵ�С
    {
        if((HT[j].parent == 0)&&(j!=*s1)) //��������һ������J!=*s1��ӦΪ��������С
        {
            if(s==0)
                s=j;
            if(HT[j].count < HT[s].count)
                s=j;
        }
    }
    *s2 = s;
}
//�����Ĺ�����������һά���齨����ͬʱ��ʼ��ַ��1��
int huff::creatHuffmanTree(HuffNode* HT, Ascll* ascll)
{
    int i,s1,s2,leafNum=0,j=0;

    //��ʼ��Ҷ�ڵ�,256��ascll�ַ�
    for(i = 0; i < 256; i ++)
    {
        //ֻʹ�ó��ֵĹ����ַ� ascll[i].count > 0
        if(ascll[i].count > 0)
        {
            HT[++j].count = ascll[i].count;
            HT[j].alpha = ascll[i].alpha;
            HT[j].parent=HT[j].lchild=HT[j].rchild=0;
        }
    }
    // [Ҷ��] [Ҷ��] [Ҷ��] [Ҷ��] ������[�ڲ�] [��]
    leafNum = j;
    int nodeNum = 2*leafNum -1; //�ڵ����

    //��ʼ���ڲ��ڵ�
    for(i = leafNum + 1; i <= nodeNum; i++)
    {
        HT[i].count = 0;
        HT[i].code[0] = 0;
        HT[i].parent = HT[i].lchild = HT[i].rchild = 0;
    }
    //���ڲ��ڵ��Һ���
    for(i = leafNum + 1; i <= nodeNum; i++)
    {
        select(HT, i - 1, &s1, &s2); //�ҵ���ǰ��С�ʹ�С������
        HT[s1].parent=i;
        HT[s2].parent=i;
        HT[i].lchild=s2;
        HT[i].rchild=s1;
        HT[i].count=HT[s1].count+HT[s2].count;
    }
    return leafNum;
}

//����������
void huff::HuffmanCoding(char* hTable[256], HuffNode* HT, int leafNum)
{
    int i,j,m,c,f,start;
    char cd[520];
    m = 520;
    cd[m-1] = 0;
    for(i=1;i <= leafNum;i++)
    {
        start = m-1;
        //���ǴӺ���ǰ���룬����Ҷ��ʼ����
        for(c=i,f=HT[c].parent; f!=0; c=f,f=HT[f].parent) //�Ұְ�
        {   //�ж��Լ�c�ǰְֵ��ĸ�����
            if(HT[f].lchild==c)
            {
                // �� 0
                cd[start--]='0';
            }
            else
            {
                // �� 1
                cd[start--]='1';
            }
        }
        // [0 0 0 0 0 start 0 1 0 1 1], start ��ʾƫ�ƣ�m-start ��ʾѹ���01�ĳ��ȣ�start�����
        start++;
        //int end = m-1;
        for(j=0;j<m-start;j++)
        {
            // ��ȡ�ַ�����
            HT[i].code[j]=cd[start+j];
            // ���� [Ҷ��]---[��]
            //HT[i].code[j]=cd[end--];
        }
        // ��ӽ�β
        HT[i].code[j]='\0';
        //д���ַ�-Ƶ����
        hTable[ HT[i].alpha ] = HT[i].code;
    }
}

void huff::compress(string filename)
{
    bool compress =TRUE;
    FILE *infile = NULL,
         *outfile = NULL;
    char infileName[520],outfileName[520];

    // ���ļ�
    infile = fopen(filename.c_str(),"rb");
    // �����ļ���
    strcpy(outfileName,filename.c_str());
    strcat(outfileName,".gr");
    // �ж��ļ��Ƿ����
    // ���ļ����в������ж��ļ��Ƿ����
    //�ж��Ƿ���Դ������ļ���������У���ʾ�޷����ļ�ϵͳ�д������ļ���������������
    outfile = fopen(outfileName,"wb");
    if(outfile == NULL)
    {
        cout<<"\n�޷�������ѹ���ļ�..."<<endl;
        exit(-4);
    }

    cout<<"�ļ�ѹ����..."<<endl;

    //[TIME-START]
    const double begin=(double)clock()/CLK_TCK;

    //ͳ���ַ���������Ƶ��
    unsigned char c;
    int i,k;
    unsigned long total=0;              //�ļ�����

    // ����hash������ĸ����ĸ����Ƶ��
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
        total++; //��ȡ�����ַ�����
    }

    total--;
//   ascll[c].count--; //TODO
// ���� ���������ڵ�����
    HuffNode HT[520];
    int leafNum = creatHuffmanTree(HT,ascll);

    char *hTable[520];
    for(i = 0; i < 256; i ++)
    {
        hTable[i] = new char[520];
    }
// ����������
    HuffmanCoding(hTable, HT, leafNum);

    /*if(!compress){
        cout<<"��ĸ\t��Ƶ����\t����\t"<<endl;
        for(i = 0; i < 256; i ++){
            if(ascll[i].count > 0){
                cout<<ascll[i].alpha<<"\t"<<ascll[i].count<<"\t"<<hTable[ascll[i].alpha]<<endl;
            }
        }
        // �رմ򿪵��ļ�
        fclose(infile);
        fclose(outfile);
        //[TIME-END]
        const double end=(double)clock()/CLK_TCK;
        cout<<"�����ʱ��"<<(end-begin)<<" s"<<endl;
        return;
    }*/

//дͷ�ļ� -- ��ѹ���Ĺ�������������Ϣд���������
    fseek(infile,0,0);
    fwrite(&total,sizeof(unsigned long),1,outfile);          //ԭ�ļ��ܳ���

    for(i=0; i<=255; i++)
    {
        // �������������� unsigned long �ķ�ʽѹ���ļ��У��±��ʾ��ĸ����ֵ��ʾ��ĸ��Ƶ��
        fwrite(&ascll[i].count,sizeof(unsigned long),1,outfile);
    }

//��ʼѹ�����ļ�
    //char buf[520];
    unsigned long j=0;             //���Ϊtotal
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
        // �����õ� 0 1 ÿ8���Ϳ��Թ���һ����ĸ�ķ�ʽѹ���ļ���
        while(k>=8)
        {
            for(i=0; i<8; i++)
            {
                // ����������Ϊ���ұ߿ճ�һ����λ
                // ������ 1 ȡ��ķ�ʽѹ�� bit 1
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
            // ȷ��ʣ�µ�bit �ĳ��ȣ��������8��ʾ������ѹ��һ���ֽ�
            k = buf.length();
        }
        if(j==total){
            break;
        }
    }
    // �� k < 8 ʱ����ʾ��ʣ�²��� 8 λ��bit����Ҫ��չ0λѹ��
    if(k>0)            //���ܻ���ʣ���ַ�
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
// �رմ򿪵��ļ�
    fclose(infile);
    fclose(outfile);

    //[TIME-END]
    const double end=(double)clock()/CLK_TCK;

    cout<<"ѹ���ɹ���"<<endl;
    float s;
    s=(float)charNum / (float)total;
    cout<<"ѹ����Ϊ��"<<s<<endl;
    cout<<"��ʱΪ��"<<(end-begin)<<" s"<<endl;
    _getch();
    return;
}

void huff::decompress(string filename) {
     FILE *infile,*outfile;
    char infilename[255],outfilename[255];

    infile = fopen(filename.c_str(),"rb");
    //ѭ���ж��ļ��Ƿ����
    /*while(infile==NULL){
        char option;
        cout<<"�ļ�"<<infilename<<"������...\n";
        cout<<"���������ļ�����1���򷵻����˵���2����";
        cin>>option;
        while(option!='1' && option!='2')
        {
            cout<<"\n��Ч�����룡\n";
            cout<<"���������ļ�����1���򷵻����˵���2����";
            cin>>option;
        }
        if(option  == '2'){
            return;
        }
        else {
            cout<<"\n������Ҫ��ѹ���ļ����ļ�·��(����.gr)��";

            cin>>outfilename;

            // ������ѹ�ļ���
            strcpy(infilename,outfilename);
            strcat(infilename,".gr");

            infile = fopen(infilename,"rb");
        }
    }*/
    // �����ѹ����ļ���
    // cout<<"�������ѹ����ļ����ļ�·����";
    // cin>>outfilename;

    outfile = fopen(filename.substr(0,filename.length()-3).c_str(),"wb");
    if(outfile==NULL) {
        cout<<"\n��ѹ�ļ�ʧ�ܣ��޷�������ѹ����ļ�...";
        return;
    }
    cout<<"��ѹ�ļ���..."<<endl;
    //[TIME-BEGIN]
    const double begin=(double)clock()/CLK_TCK;

    unsigned long total = 0;
    // ����һ�� long �������ݶ��� tatol �У�Ϊ�ļ����ܴ�С
    fread(&total,sizeof(unsigned long),1,infile);
    //cout<<"ԭ����С: "<<total<<endl;
    Ascll ascll[256];
    int i;
    for(i = 0; i < 256; i++) {
        // ֮���ÿ��long���ȶ���һ���ַ���Ƶ��
        fread(&ascll[i].count,sizeof(unsigned long),1,infile);
        ascll[i].alpha = i;
    }

    HuffNode HT[520];
    // ������������
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
        // ������ĸ��ȡ
        c=fgetc(infile);
        if(test){
            cout<<"��һ���ַ���"<<c<<endl;
        }
        // �Ӹ��ڵ���Ҷ���ߣ���ȡ������һ�� ��ĸ��8λ���� ����ʹ��ѭ��8��
        for(i=0; i<8; i++)
        {
            unsigned int cod = (c & 128);
            c = c << 1;
            if(cod == 0 ){
                // �� 0 �� 1
                index = HT[index].lchild;
            }
            else{
                index = HT[index].rchild;
            }
            if(HT[index].rchild == 0 && HT[index].lchild == 0){

                charNum ++;
                // ����Ҷ��
                char trueChar = HT[index].alpha;
                fwrite(&trueChar,sizeof(unsigned char),1,outfile);
                // index ����ָ����ڵ�
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

    // �رմ򿪵��ļ�
    fclose(infile);
    fclose(outfile);

    //[TIME-END]
    const double end=(double)clock()/CLK_TCK;

    cout<<"��ѹ�ɹ�"<<endl;
    float s;
    s=(float)charNum / (float)total;
    cout<<"������Ϊ��"<<s<<endl;
    cout<<"��ʱΪ��"<<(end-begin)<<" s"<<endl;
    return;
}




