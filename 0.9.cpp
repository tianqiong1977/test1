#include<iostream>
#include<ostream>
#include<string>
#include<io.h>
#include <direct.h>
#include <vector>
#include <fstream>
#include<string.h>
#include <stdlib.h>
#include <cmath>
#define MAXLEN 32768

struct fileInfo{
    std::string fileDirect;
    int fileLen;
};

class TarContents{
    private:
        bool getShortDir(std::string outputRoot);
        bool makeNewDirFromFilePath(std::string filePath);
        void findAllfileInDir(std::string rootDir);
        int getFileLen(std::string file);
    public:
        bool filePath2tar(std::string inputPath, std::string outputTarDir);
        bool files2tar(std::string outputTarDir);
        bool tar2file(std::string inputTarFileDir, std::string outputTarDir);
};

std::vector<std::string> fileDirVector;
std::vector<std::string> fileDirOutVector;
std::string fileDirNow;

int main()
{

    fileDirVector.reserve(MAXLEN);
    fileDirOutVector.reserve(MAXLEN);
    std::string output = "D://users//1.tar";
    fileDirNow = "C://Users//tq979//Desktop//2";
    //fileDirVector.push_back("C://Users//tq979//Desktop//2//0.1.cpp");
    //fileDirVector.push_back("C://Users//tq979//Desktop//2//1211//0.1.cpp");
    //std::cout<<output.length()<<std::endl;
    TarContents tc;
    //fileDirNow = output;
    //tc.files2tar(output);
    //tc.filePath2tar("C://Users//tq979//Desktop//2", output);
    tc.tar2file(output, "D://Users//tq979");

    return 0;
}

bool TarContents::tar2file(std::string inputTarFileDir, std::string outputTarDir) {
    if(inputTarFileDir.length() * outputTarDir.length() == 0)
        return false;
    FILE *tarFile = fopen(inputTarFileDir.c_str(), "r");
    if(!tarFile)
        return false;
    int fileNumCount = 0, blocks = 0, remains = 0, fileLenTemp = 0;
    struct fileInfo fileNow;

    unsigned char buf[MAXLEN];
    //char workPathChar[MAXLEN];
    char dirBuffer[MAXLEN];

    memset(dirBuffer, '\0', sizeof(buf));
    fgets(dirBuffer,MAXLEN,tarFile);
    fileNumCount = atoi(dirBuffer);
    memset(dirBuffer, '\0', sizeof(buf));
    fgets(dirBuffer, MAXLEN, tarFile);
    fileDirNow = dirBuffer;


    //fread(&fileDirLenNum, sizeof(int), 1, tarFile);
    //fread(buf, log10(fileDirLenNum) + 5 + fileDirNow.length(), 1, tarFile);
    //memset(buf, '/0', sizeof(buf))
    memset(dirBuffer, '\0', sizeof(buf));
    std::cout<<fileDirNow<<std::endl;

    for(int fc = 0; fc != fileNumCount; fc++) {
        fgets(dirBuffer, MAXLEN, tarFile);
        fileLenTemp = atoi(dirBuffer);
        //std::string SCHECK = dirBuffer;
        fileNow.fileLen = fileLenTemp;
        memset(dirBuffer, '\0', sizeof(buf));
        fgets(dirBuffer, MAXLEN, tarFile);
        fileNow.fileDirect = dirBuffer;
        memset(dirBuffer, '\0', sizeof(buf));
        blocks = fileNow.fileLen / MAXLEN;
        remains = fileNow.fileLen % MAXLEN;
        //fread(buf, log10(fileNow.fileLen) + 5 + fileNow.fileDirect.length(), 1, tarFile);
        for(int bc = 0; bc != blocks; bc++)
            fread(buf, sizeof(unsigned char), MAXLEN, tarFile);
        memset(buf, '\0', sizeof(buf));
        fread(buf, sizeof(unsigned char), remains, tarFile);

        /*SCHECK = "";
        while(SCHECK.compare("E!N@D#")) {
            fgets(dirBuffer, MAXLEN, tarFile);
            SCHECK = dirBuffer;
            std::cout<<SCHECK<<std::endl;
            memset(dirBuffer, '\0', sizeof(buf));
        }*/
        fgets(dirBuffer, MAXLEN, tarFile);
        //SCHECK = dirBuffer;
        memset(dirBuffer, '\0', sizeof(buf));
        fileDirVector.push_back(fileNow.fileDirect);
    }
    int testNum = fileDirVector.size();
    fclose(tarFile);
    tarFile = fopen(inputTarFileDir.c_str(), "r");
    getShortDir(outputTarDir);
    fileDirVector.clear();
	for(int kkk = 0;kkk!=fileDirOutVector.size();kkk++)
		std::cout<<fileDirOutVector.at(kkk)<<" "<<std::endl;

    FILE *outFileDir;
    //char fileDirTemp[MAXLEN];
    //char fileLenTemp[MAXLEN];

    for(int vc = 0; vc != fileDirOutVector.size() ; vc++)
        makeNewDirFromFilePath(fileDirOutVector.at(vc));
    tarFile = fopen(inputTarFileDir.c_str(), "r");
    fgets(dirBuffer, MAXLEN, tarFile);
    fileNumCount = atoi(dirBuffer);
    fgets(dirBuffer, MAXLEN, tarFile);
    memset(dirBuffer, '\0', sizeof(dirBuffer));
    for(int frc = 0; frc != fileNumCount; frc++) {
        fgets(dirBuffer, MAXLEN, tarFile);
        fileNow.fileLen = atoi(dirBuffer);
        memset(dirBuffer, '\0', sizeof(dirBuffer));
        fgets(dirBuffer, MAXLEN, tarFile);
        fileNow.fileDirect = dirBuffer;
        memset(dirBuffer, '\0', sizeof(dirBuffer));
		//if(a[strlen(a)-1])=='\n') a[strlen(a)-1])=0;
		
		
//		std::cout<<fileDirOutVector.at(frc).c_str()<<std::endl;
		
        FILE *outFileDirTemp = fopen(fileDirOutVector.at(frc).substr(0, fileDirOutVector.at(frc).length() - 1).c_str(), "w+");
        if(outFileDirTemp == NULL)
            return false;
        blocks = fileNow.fileLen / MAXLEN;
        remains = fileNow.fileLen % MAXLEN;
        for(int bc2 = 0; bc2 != blocks; bc2++) {
            fread(buf, sizeof(unsigned char), MAXLEN, tarFile);
            fwrite(buf, sizeof(unsigned char), MAXLEN, outFileDirTemp);
        }
        memset(buf, '\0', sizeof(buf));
        fread(buf, sizeof(unsigned char), remains, tarFile);
        fwrite(buf, sizeof(unsigned char), remains, outFileDirTemp);
        fclose(outFileDirTemp);
        fgets(dirBuffer, MAXLEN, tarFile);
        memset(dirBuffer, '\0', sizeof(buf));
    }
    fclose(tarFile);
    return true;
}

bool TarContents::filePath2tar(std::string inputPath, std::string outputTarDir) {
    makeNewDirFromFilePath(outputTarDir);
    fileDirNow = inputPath;
    findAllfileInDir(inputPath);
    files2tar(outputTarDir);
    return true;
}

bool TarContents::files2tar(std::string outputTarDir) {
    if(fileDirNow.length() == 0)
        return false;
    if(fileDirVector.size() == 0)
        return false;
    makeNewDirFromFilePath(outputTarDir);

    FILE *nowFile,*outputFile;
    outputFile = fopen(outputTarDir.c_str(), "w");
    if(outputFile == NULL)
        return false;


    unsigned char buf[MAXLEN];
    struct fileInfo fInfo;
    int fileHandle = -1, re = 0;// headLen;
    int fileNum = fileDirVector.size();


    fprintf(outputFile,"%d\n", fileNum);
    //std::cout<<"fileNum:"<<fileNum<<td::endl;
    //int fileNameLen = fileDirNow.length();
    //fwrite(&fileNameLen, sizeof(int), 1, outputFile);
    fprintf(outputFile,"%s\n", fileDirNow.c_str());
    //fwrite(&fileDirNow, fileDirNow.length(), 1, outputFile);
    for(int fileCounter = 0; fileCounter != fileNum; fileCounter++) {
        //fprintf(outputFile,"%s\n","fileFiRst789456");
        nowFile = fopen(fileDirVector.at(fileCounter).c_str(), "r");
        if(nowFile == NULL)
            return false;
        fileHandle = open(fileDirVector.at(fileCounter).c_str(), 0x0100);
        fInfo.fileDirect = fileDirVector.at(fileCounter);
        fInfo.fileLen = getFileLen(fInfo.fileDirect);
        //headLen = 2 * sizeof(int) + fInfo.fileDirect.length();

        //fwrite(&headLen, 1, 1, outputFile);
        std::cout<<"file:"<<fileCounter+1<<"  "<<fInfo.fileLen<<std::endl;
        fprintf(outputFile,"%d\n", fInfo.fileLen);
        fprintf(outputFile,"%s\n", fInfo.fileDirect.c_str());
        //fwrite(&fInfo.fileDirect, fInfo.fileDirect.length(), 1, outputFile);

        while((re = fread(buf, sizeof(unsigned char), MAXLEN, nowFile)) != 0)
            fwrite(buf, sizeof(unsigned char), re, outputFile);
        fprintf(outputFile,"\n","");
        //fprintf(outputFile,"\n%s\n","E!N@D#");
        fclose(nowFile);
    }
    fclose(outputFile);
    fileDirVector.clear();
    return true;
}

bool TarContents::getShortDir(std::string outputRoot) {
    if(outputRoot.length() == 0 || fileDirNow.length() == 0)
        return false;
    int fileCount = fileDirVector.size();
    for(int count = 0; count != fileCount; count++) {
        if(fileDirVector[count].length() < fileDirNow.length())
            return false;
        std::string dirStrAfter = fileDirVector[count].substr(fileDirNow.length());
        std::string dirStr = outputRoot + "/" + dirStrAfter;
        //dirStr.erase(dirStr.length() - 1, dirStr.length());
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
                if(strcmp(fileInfoData.name,".") != 0 && strcmp(fileInfoData.name,"..") != 0)
                findAllfileInDir(p.assign(rootDir).append("//").append(fileInfoData.name));
            } else {
                fileDirVector.push_back(p.assign(rootDir).append("//").append(fileInfoData.name));
            }
        } while (_findnext(fileHandle, &fileInfoData) == 0 );
        _findclose(fileHandle);
    }
}

int TarContents::getFileLen(std::string file) {
    std::fstream fs(file.c_str());
    std::istreambuf_iterator<char> start(fs),end;
    std::string temp(start,end);
    return temp.length();
}

