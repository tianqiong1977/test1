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


struct fileHead{
    char fileName[256];
    int fileLen;
    int isFolder;
};


class TarContents{
    private:
        bool getOutputDir(std::string outputRoot);
        bool makeNewDirFromFilePath(std::string filePath);
        void findAllfileInDir(std::string rootDir);
        int getFileLen(std::string file);
        void checkVector();
        bool readFileAndFolderFromTar();

        std::vector<std::string> fileDirVector;
        std::vector<std::string> fileDirOutVector;
        std::vector<std::string> dirOutVector;
        std::string fileDirNow;

        std::vector<std::string> checkDirVector;
    public:
        void initial(std::string fileDirNow1){fileDirNow = fileDirNow1;}
        void adddir(std::string fileDir){fileDirVector.push_back(fileDir);}
        int MAXLEN = 1024;
        bool filePath2tar(std::string inputPath, std::string outputTarDir);
        bool files2tar(std::string outputTarDir);
        bool tar2file(std::string inputTarFileDir, std::string outputTarDir);
        void recordFileAndFolder(std::string dir,int select);
};

class  consoleUI{
    private:
        TarContents tar;
        void welcome();
    public:
        void run();
};
void consoleUI::welcome() {
	std::cout<<"��ӭʹ�ã�����������ʹ�ù���"<<std::endl;
    std::cout<<"1:ѹ��ָ���ļ�����ȫ���ļ�"<<std::endl;
    std::cout<<"2:ѹ��ָ�����ļ����ļ���"<<std::endl;
    std::cout<<"3:��ѹ��ָ���ļ���"<<std::endl;
    std::cout<<"4:�޸�ѹ����ʽ"<<std::endl;
    std::cout<<"9:EXIT"<<std::endl;
}

void consoleUI::run() {
    int input;
    input = -1;
	int input2;
    std::string str;
    std::string out;
    welcome();
    while(std::cin>>input){
        switch (input) {
            case 1:
                std::cout<<"��������Ҫѹ�����ļ��� ����C://user"<<std::endl;
                std::cin>>str;
                if(str.length() != 0) {
                    tar.initial(str);
                }
                std::cout<<"���������·�� ����D://2.tar"<<std::endl;
                std::cin>>out; 
				if(tar.filePath2tar(str,out) == true)
                    std::cout<<"���!"<<std::endl;
                break;
            case 2:
            	std::cout<<"��������Ҫѹ���ļ�/�ļ��еĸ�Ŀ¼ ����C://user"<<std::endl; 
				std::cin>>str;
				if(str.length() != 0) {
                    tar.initial(str);
                }
                
				std::cout<<"����ѹ��Ŀ������� 1���ļ� 0���ļ��� -1���˳�"<<std::endl;
				while(std::cin>>input2){
					if(input2 != -1) {
						std::cout<<"�����ļ�/�ļ���·��"<<std::endl; 
						std::string inputPath;
						std::cin>>inputPath;
						tar.recordFileAndFolder(inputPath, input2);	
					}else{
						std::cout<<"���������·�� ����D://2.tar"<<std::endl;
                		std::cin>>out; 
						tar.files2tar(out);
						break;
					}
					std::cin.clear();
					std::cin.ignore();
				}
            	break;
            default:
            break;
        }
		if(input == 9)exit(0);
		welcome();
    	std::cin.clear();
		std::cin.ignore();
	}
}


bool TarContents::filePath2tar(std::string inputPath, std::string outputTarDir) {
    findAllfileInDir(inputPath);
    files2tar(outputTarDir);
    return true;
}

bool TarContents::tar2file(std::string inputTarFileDir, std::string outputTarDir) {
    FILE* inputTar;
    inputTar = fopen(inputTarFileDir.c_str(), "rb");
    if(inputTar == NULL)
        return false;
    if(outputTarDir.length() != 0)
        makeNewDirFromFilePath(outputTarDir.append("//"));
    int headNum = 0;
    int fileNum = 0;
    std::vector<int> fileLenVector;
    unsigned char buf[MAXLEN];
    fread(&headNum, sizeof(int), 1, inputTar);
    fread(&fileNum, sizeof(int), 1, inputTar);
    /*                */
    int tempInt = 0;
    fread(&tempInt, sizeof(int), 1, inputTar);
    fread(buf, tempInt, 1, inputTar);
    memset(buf, '\0', sizeof(buf));
    /*                */
    struct fileHead fh;
    for(int hc = 0; hc != headNum; hc++) {
        fread(&fh, sizeof(fileHead), 1,inputTar);
        if(fh.isFolder)
            dirOutVector.push_back(fh.fileName);
        else{
            fileDirVector.push_back(fh.fileName);
            fileLenVector.push_back(fh.fileLen);
        }
    }
    FILE *newFile;
    int blocks = 0, remains = 0;
    getOutputDir(outputTarDir);
    int cNum = fileDirOutVector.size();
    for(int fc = 0; fc != cNum; fc++) {
        std::string testFileName = fileDirOutVector.at(fc);
        makeNewDirFromFilePath(fileDirOutVector.at(fc).c_str());
        newFile = fopen(fileDirOutVector.at(fc).c_str(), "wb");
        if(newFile == NULL)return false;
        blocks = fileLenVector.at(fc) / MAXLEN;
        remains = fileLenVector.at(fc) % MAXLEN;
        for(int rwc = 0; rwc!= blocks; rwc++) {
            fread(buf, sizeof(unsigned char), MAXLEN, inputTar);
            fwrite(buf, sizeof(unsigned char), MAXLEN, newFile);
        }

        memset(buf, '\0', sizeof(buf));
        fread(buf, sizeof(unsigned char), remains, inputTar);
        fwrite(buf, sizeof(unsigned char), remains, newFile);
        fclose(newFile);
    }
    fileDirOutVector.clear();

    std::string testFileName =dirOutVector.at(0);
    fileDirVector.swap(dirOutVector);
    getOutputDir(outputTarDir);
    cNum = fileDirOutVector.size();
    for(int dirc = 0;dirc != cNum; dirc++)
        makeNewDirFromFilePath(fileDirOutVector.at(dirc).append("//"));
    dirOutVector.clear();
    return true;
}

bool TarContents::files2tar(std::string outputTarDir) {
    if(fileDirNow.length() == 0)
        return false;
    if(fileDirVector.size() == 0 && dirOutVector.size() == 0)
        return false;
    makeNewDirFromFilePath(outputTarDir);

    FILE *nowFile, *outputFile;
    outputFile = fopen(outputTarDir.c_str(), "wb");
    if(outputFile == NULL)
        return false;

    unsigned char buf[1024];
    struct fileHead fh;
    int re = 0, fileNum = fileDirVector.size(), headNum = fileDirVector.size() + dirOutVector.size();

    checkVector();
    //д���ļ�ͷ���� �ļ�����+�ļ������� ��ǰ·������ ��ǰ·��
    char nowDirInput[255];
    fwrite(&headNum, sizeof(int), 1, outputFile);
    fwrite(&fileNum, sizeof(int), 1, outputFile);

    int fileDirNowLength = 0;
    memset(nowDirInput, '\0', 255);
    for(; fileDirNowLength != fileDirNow.length() && fileDirNowLength < 255; fileDirNowLength++){
        nowDirInput[fileDirNowLength] = fileDirNow[fileDirNowLength];
    }
    nowDirInput[fileDirNowLength] = '\0';
    fwrite(&fileDirNowLength, sizeof (int), 1, outputFile);
    fwrite(nowDirInput, fileDirNowLength, 1, outputFile);
    //�������ļ����Ƿ��Ѿ����ڣ����������˳�
   /*for(const auto& item:fileDirVector) {
        if(item == outputTarDir) {
            std::cout<<"���·��������ͬ�ļ���"<<std::endl;
            exit(-2);
        }
    }*/ 

    //��ÿ���ļ�ѹ������



    //д���ļ�ͷ   �ļ�����
    for(int fileCounter = 0; fileCounter != fileNum; fileCounter++) {
        nowFile = fopen(fileDirVector.at(fileCounter).c_str(), "rb");
        if(nowFile == NULL)
            return false;
        memset(&fh, '\0', sizeof(fileHead));
        fh.isFolder = 0;
        strcpy(fh.fileName, fileDirVector.at(fileCounter).c_str());
        fh.fileLen = getFileLen(fh.fileName);
        fwrite(&fh, sizeof(fileHead), 1, outputFile);
        fclose(nowFile);
    }
    fileNum = dirOutVector.size();
    //д���ļ�ͷ  �ļ��в���
    for(int folderCounter = 0;folderCounter != fileNum; folderCounter++) {
        memset(&fh, '\0', sizeof(fileHead));
        fh.isFolder = 1;
        fh.fileLen = 0;
        strcpy(fh.fileName, dirOutVector.at(folderCounter).c_str());
        fwrite(&fh, sizeof(fileHead), 1, outputFile);
    }

    //д���ļ�
    fileNum = fileDirVector.size();
    for(int fileCounter = 0; fileCounter != fileNum; fileCounter++) {
        nowFile = fopen(fileDirVector.at(fileCounter).c_str(), "rb");
        if(nowFile == NULL)
            return false;
        while((re = fread(buf, sizeof(unsigned char), MAXLEN, nowFile)) != 0)
            fwrite(buf, sizeof(unsigned char), re, outputFile);
        memset(buf, '\0', sizeof(buf));
        fclose(nowFile);
    }
    fclose(outputFile);
    fileDirVector.clear();
    return true;
}

bool TarContents::getOutputDir(std::string outputRoot) {
    if(outputRoot.length() == 0 || fileDirNow.length() == 0)
        return false;
    int fileCount = fileDirVector.size();
    for(int count = 0; count != fileCount; count++) {
        if(fileDirVector[count].size() < fileDirNow.length())
            return false;
        std::string dirStrAfter = fileDirVector[count].substr(fileDirNow.length());
        std::string dirStr = outputRoot + dirStrAfter;
        fileDirOutVector.push_back(dirStr);
    }
    return true;
}

bool TarContents::makeNewDirFromFilePath(std::string filePath) {
    if(filePath.length() == 0)
        return false;
    int finalIndex = filePath.find_last_of("//");
    if(finalIndex == 0)
        return true;

    const std::string foldPath = filePath.substr(0, finalIndex-1);
    if(0 != access(foldPath.c_str(), 0)) {
        if(0 == mkdir(foldPath.c_str()))
            return true;
    }
    return false;
}

void TarContents::findAllfileInDir(std::string rootDir) {
    long fileHandle = 0;
    struct _finddata_t fileInfoData;
    std::string p;
    if((fileHandle = _findfirst(p.assign(rootDir).append("\\*").c_str(), &fileInfoData)) != -1) {
        do{
            if(_A_SUBDIR & fileInfoData.attrib){
                if(strcmp(fileInfoData.name,".") != 0 && strcmp(fileInfoData.name,"..") != 0) {
                    findAllfileInDir(p.assign(rootDir).append("//").append(fileInfoData.name));
                    std::string Dir = p.assign(rootDir).append("//").append(fileInfoData.name);
                    dirOutVector.push_back(Dir);
                }
            } else {
                fileDirVector.push_back(p.assign(rootDir).append("//").append(fileInfoData.name));
            }
        } while (_findnext(fileHandle, &fileInfoData) == 0 );
        _findclose(fileHandle);
    }
}

int TarContents::getFileLen(std::string file) {
        FILE *pFile;
        pFile=fopen(file.c_str(),"rb");  //��ȡ�Ѵ��ļ���ָ��
        fseek(pFile,0,SEEK_END);  //����fseek���ļ�ָ���Ƶ��ļ�ĩβ
        int n=ftell(pFile);
        return n;
}
/*
        std::vector<std::string> fileDirVector;
        std::vector<std::string> fileDirOutVector;
        std::vector<std::string> dirOutVector;

*/
void TarContents::checkVector() {
    std::cout<<"fileDirVector"<<std::endl;
    for(int i = 0; i != fileDirVector.size(); i++)
        std::cout<<fileDirVector.at(i)<<std::endl;
    std::cout<<"dirOutVector.at(i)"<<std::endl;
    for(int i = 0; i != dirOutVector.size(); i++)
        std::cout<<dirOutVector.at(i)<<std::endl;
    std::cout<<"fileDirOutVector.at(i)"<<std::endl;
    for(int i = 0; i != fileDirOutVector.size(); i++)
        std::cout<<fileDirOutVector.at(i)<<std::endl;
}

void TarContents::recordFileAndFolder(std::string dir, int select) {
   //����ΪΪ0ʱ�� ��Ϊ����һ���ļ���
    if(select == 0) {
        dirOutVector.push_back(dir);
    }else{
        fileDirVector.push_back(dir);
    }
}



int main()
{
    consoleUI cui;
    cui.run();
    return 0;
}

