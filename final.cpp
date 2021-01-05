
#include "lz77.h"

struct fileHead{
    char fileName[256];
    int fileLen;
    int isFolder;

    int fileLenZip;
};


class TarContents{
    private:
        bool getOutputDir(std::string outputRoot);
        bool makeNewDirFromFilePath(std::string filePath);
        void findAllfileInDir(std::string rootDir);
        int getFileLen(std::string file);
        void checkVector();
        bool readFileAndFolderFromTar();
        int isFindVector(int i, std::string name, int len);

        std::vector<std::string> fileDirVector;
        std::vector<std::string> fileDirOutVector;
        std::vector<std::string> dirOutVector;
        std::string fileDirNow;

        std::vector<std::string> checkDirVector;
        int encodeType = 1;
        int nowDirLeN = 0;

        void lz77(std::string filename,int opType);
        void huf(std::string filename,int opType);
    public:
        void initial(std::string fileDirNow1){fileDirNow = fileDirNow1;}
        void adddir(std::string fileDir){fileDirVector.push_back(fileDir);}
        void switchEncodeType(int type){encodeType = type;}
        int MAXLEN = 1024;
        bool filePath2tar(std::string inputPath, std::string outputTarDir);
        bool files2tar(std::string outputTarDir);
        bool tar2file(std::string inputTarFileDir, std::string outputTarDir);
        void recordFileAndFolder(std::string dir,int select);
        void checkDecode(std::string inputTar,std::string inputDir);
};

class  consoleUI{
    private:
        TarContents tar;
        void welcome();
    public:
        void run();
};
void consoleUI::welcome() {
	std::cout<<"欢迎使用，输入数字以使用功能"<<std::endl;
    std::cout<<"1:压缩指定文件夹内全部文件"<<std::endl;
    std::cout<<"2:压缩指定文文件或文件夹"<<std::endl;
    std::cout<<"3:解压缩指定文件夹"<<std::endl;
    std::cout<<"4:修改压缩方式"<<std::endl;
    std::cout<<"5:检查压缩正确性"<<std::endl;
    std::cout<<"9:EXIT"<<std::endl;
}

void consoleUI::run() {
    int input;input = -1;
	int input2;int typ = 1;
    std::string str;
    std::string out;
    welcome();
    while(std::cin>>input){
        switch (input) {
            case 1:
                std::cout<<"请输入需要压缩的文件夹 例如C://user"<<std::endl;
                std::cin>>str;
                if(str.length() != 0) {
                    tar.initial(str);
                }
                std::cout<<"请输入输出路径 例如D://2.tar"<<std::endl;
                std::cin>>out;
				if(tar.filePath2tar(str,out) == true)
                    std::cout<<"完成!"<<std::endl;
                break;
            case 2:
            	std::cout<<"请输入需要压缩文件/文件夹的根目录 例如C://user"<<std::endl;
				std::cin>>str;
				if(str.length() != 0) {
                    tar.initial(str);
                }

				std::cout<<"输入压缩目标的类型 1：文件 0：文件夹 -1：退出"<<std::endl;
				while(std::cin>>input2){
					if(input2 != -1) {
						std::cout<<"输入文件/文件夹路径"<<std::endl;
						std::string inputPath;
						std::cin.sync();
						std::cin>>inputPath;
						tar.recordFileAndFolder(inputPath, input2);
                        std::cin.sync();
                        std::cout<<"输入压缩目标的类型 1：文件 0：文件夹 -1：退出"<<std::endl;
					}else{
						std::cout<<"请输入输出路径 例如D://2.tar"<<std::endl;
                		std::cin>>out;
						tar.files2tar(out);
						std::cin.sync();
						break;
					};
				}
            	break;
            case 3:
                std::cout<<"请输入需要解压文件的路径 例如C://user//1.tar"<<std::endl;
                std::cin.sync();
                std::cin>>str;
                std::cin.sync();
                if(str.length() != 0) {
                    std::string str2;
                    std::cout<<"请输入解压的目标路径 例如D://user"<<std::endl;
                    std::cin>>str2;
                    std::cin.sync();
                    if(tar.tar2file(str, str2) == true)
                        std::cout<<"完成!"<<std::endl;
                }
                break;
            case 4:
                std::cout<<"输入压缩算法的类型 1：lz77 0：霍夫曼"<<std::endl;
                std::cin.sync();
                std::cin>>typ;
                tar.switchEncodeType(typ % 2);
                std::cout<<"完成!"<<std::endl;
                break;
            case 5:
                std::cout<<"请输入需要tar文件的路径 例如C://user//1.tar"<<std::endl;
                std::cin.sync();
                std::cin>>str;
                std::cin.sync();
                 if(str.length() != 0) {
                    std::string str2;
                    std::cout<<"请输入解压后的路径 例如D://user"<<std::endl;
                    std::cin>>str2;
                    std::cin.sync();
                    tar.checkDecode(str, str2);
                    //    std::cout<<"完成!"<<std::endl;
                }
                break;

        }
		if(input == 9)exit(0);
		welcome();
    	std::cin.sync();

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


    fread(&encodeType, sizeof(int), 1, inputTar);
    fread(&headNum, sizeof(int), 1, inputTar);
    fread(&fileNum, sizeof(int), 1, inputTar);
    /*                */
    fread(&nowDirLeN, sizeof(int), 1, inputTar);nowDirLeN += 2;
    /*                */
    struct fileHead fh;
    for(int hc = 0; hc != headNum; hc++) {
        fread(&fh, sizeof(fileHead), 1,inputTar);
        if(fh.isFolder)
            dirOutVector.push_back(fh.fileName);
        else{
            fileDirVector.push_back(fh.fileName);
            fileLenVector.push_back(fh.fileLenZip);
            std::cout<<fh.fileName<<std::endl;
        }
    }
    FILE *newFile;
    int blocks = 0, remains = 0;
    getOutputDir(outputTarDir);
    int cNum = fileDirOutVector.size();
    for(int fc = 0; fc != cNum; fc++) {
        std::string testFileName = fileDirOutVector.at(fc);
        if(encodeType == 1)
            testFileName = testFileName + ".zip";
        else
            testFileName = testFileName + ".gr";

        makeNewDirFromFilePath(fileDirOutVector.at(fc).c_str());
        newFile = fopen(testFileName.c_str(), "wb");
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

        if(encodeType == 0){
            huf(testFileName, 0);
            remove(testFileName.c_str());
        }else{
            lz77(testFileName, 0);
            remove(testFileName.c_str());
        }

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
    //写入文件头数量 文件数量  当前路径长度
    char nowDirInput[255];
    fwrite(&encodeType, sizeof(int), 1, outputFile);
    fwrite(&headNum, sizeof(int), 1, outputFile);
    fwrite(&fileNum, sizeof(int), 1, outputFile);
    int fileDirLen = fileDirNow.length();
    fwrite(&fileDirLen, sizeof (int), 1, outputFile);
    //检查输出文件名是否已经存在，若存在则退出
   /*for(const auto& item:fileDirVector) {
        if(item == outputTarDir) {
            std::cout<<"输出路径存在相同文件名"<<std::endl;
            exit(-2);
        }
    }*/

    //将每个文件压缩处理



    //写入文件头   文件部分
    for(int fileCounter = 0; fileCounter != fileNum; fileCounter++) {
        std::string newFileName;
        if(encodeType == 0){
            huf(fileDirVector.at(fileCounter).c_str(),1);
             newFileName = fileDirVector.at(fileCounter) + ".gr";
        }else{
            lz77(fileDirVector.at(fileCounter).c_str(),1);
            newFileName = fileDirVector.at(fileCounter) + ".zip";
        }
        std::cout<<newFileName<<std::endl;
        memset(&fh, '\0', sizeof(fileHead));
        fh.isFolder = 0;
        strcpy(fh.fileName, fileDirVector.at(fileCounter).c_str());
        fh.fileLen = getFileLen(fh.fileName);
        fh.fileLenZip = getFileLen(newFileName);
        fwrite(&fh, sizeof(fileHead), 1, outputFile);
    }
    fileNum = dirOutVector.size();
    //写入文件头  文件夹部分
    for(int folderCounter = 0;folderCounter != fileNum; folderCounter++) {
        memset(&fh, '\0', sizeof(fileHead));
        fh.isFolder = 1;
        fh.fileLen = 0;
        strcpy(fh.fileName, dirOutVector.at(folderCounter).c_str());
        fwrite(&fh, sizeof(fileHead), 1, outputFile);
    }

    //写入文件
    fileNum = fileDirVector.size();
    for(int fileCounter = 0; fileCounter != fileNum; fileCounter++) {
        if(encodeType == 0){
            nowFile = fopen(fileDirVector.at(fileCounter).append(".gr").c_str(), "rb");
        }else{
            nowFile = fopen(fileDirVector.at(fileCounter).append(".zip").c_str(), "rb");
        }

        if(nowFile == NULL)
            return false;
        while((re = fread(buf, sizeof(unsigned char), MAXLEN, nowFile)) != 0)
            fwrite(buf, sizeof(unsigned char), re, outputFile);
        memset(buf, '\0', sizeof(buf));
        fclose(nowFile);
        std::cout<<fileDirVector.at(fileCounter).c_str()<<std::endl;
        std::cout<<remove(fileDirVector.at(fileCounter).c_str());
        std::cout<<std::endl;
    }
    fclose(outputFile);
    fileDirVector.clear();
    return true;
}

bool TarContents::getOutputDir(std::string outputRoot) {
    if(outputRoot.length() == 0 || nowDirLeN == 0)
        return false;
    int fileCount = fileDirVector.size();
    for(int count = 0; count != fileCount; count++) {
        if(fileDirVector[count].size() < nowDirLeN)
            return false;
        std::string dirStrAfter = fileDirVector[count].substr(nowDirLeN);
        std::string dirStr = outputRoot + dirStrAfter;
        std::cout<<dirStr<<std::endl;
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
    if(rootDir.length() == 0) return;
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
        pFile=fopen(file.c_str(),"rb");  //获取已打开文件的指针
        fseek(pFile,0,SEEK_END);  //先用fseek将文件指针移到文件末尾
        int n=ftell(pFile);
        fclose(pFile);
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
   //输入为为0时候 视为输入一个文件夹
    if(select == 0) {
        dirOutVector.push_back(dir);
    }else{
        fileDirVector.push_back(dir);
    }
}

// 0解压  1压缩
void TarContents::lz77(std::string name, int op){
    lzCompress lz;
    if(op == 0) {
        lz.UnCompress(name);
    }else{
        lz.Compress(name);
    }

}
void TarContents::huf(std::string name, int op){
    huff lz;
    if(op == 0) {
        lz.decompress(name);
    }else{
        lz.compress(name);
    }
}



void TarContents::checkDecode(std::string tar,std::string dir) {
    findAllfileInDir(dir);
    FILE* file = fopen(tar.c_str(), "rb");
    if(!file)
        exit(-3);
    int filenum = 0, foldernum = 0, dirlen = 0;
    fread(&foldernum,sizeof(int), 1, file);
    fread(&foldernum,sizeof(int), 1, file);
    fread(&filenum,sizeof(int), 1, file);
    fread(&dirlen,sizeof(int), 1, file);

    foldernum -= filenum;
    fileHead fh;
    fh.fileLen = 0;fh.isFolder = 0;
    for(int i = 0; i < filenum; i++) {
        fread(&fh,sizeof(fileHead), 1, file);
        if(isFindVector(0, fh.fileName, dirlen) != fh.fileLen) {
            std::cout<<"第"<<i+1<<"个文件不匹配！"<<std::endl;
            std::cout<<"文件名:"<<fh.fileName<<std::endl;
        }
    }
    for(int k = 0; k < foldernum; k++) {
        fread(&fh,sizeof(fileHead), 1, file);
        if(isFindVector(1, fh.fileName, dirlen) != 0){
            std::cout<<"第"<<k+1<<"个文件夹不匹配！"<<std::endl;
            std::cout<<"文件夹:"<<fh.fileName<<std::endl;
        }
    }

}

//0是文件 1是文件夹
int TarContents::isFindVector(int i, std::string name, int len) {
    if(i == 0) {
        std::string shortname(name.substr(len,name.length()));
        std::string::size_type st;
        for(int k = 0; k != fileDirVector.size(); k++){
            st = fileDirVector.at(k).find(shortname);
            if(st != std::string::npos){
                return getFileLen(fileDirVector.at(k));
            }
        }
    }else{
        std::string shortname(name.substr(len,name.length()));
        std::string::size_type st;
        for(int k = 0; k != dirOutVector.size(); k++){
            st = dirOutVector.at(k).find(shortname);
            if(st != std::string::npos)
                return 0;
    }
    return -1;
    }
}

int main(){
    consoleUI cui;
    cui.run();
    return 0;
}
