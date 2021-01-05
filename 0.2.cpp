#include<iostream>
#include<ostream>
#include<string>
#include<io.h>
#include <direct.h>
#include <vector>


#define MAXLEN 512


struct fileInfo{
	std::string fileDirect;
	int fileLen;	
};

	std::vector<std::string> fileDirVector;
	std::vector<std::string> fileDirOutVector;
	std::string fileDirNow;
bool getShortDir(std::string outputRoot) {
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

bool makeNewDirFromFilePath(const std::string filePath) {
	int finalIndex = filePath.find_last_of("//");
	if(finalIndex == 0)
		return true;
		
	const std::string foldPath = filePath.substr(0, finalIndex-1);
	std::cout<<"foldPath:"<<foldPath<<std::endl;
	
	if(0 != access(foldPath.c_str(), 0)) {
		if(0 == mkdir(foldPath.c_str())) 
			return true;
	}
	
	return false;
}

int main(){
	std::string fileDir = "C://Users//tq979//Desktop//2//1.txt";
	std::string outputDir = "2.tar";
	
	/**/
	fileDirNow =  "C://Users//tq979//Desktop//2";
	std::string fileDir1 = "C://Users//tq979//Desktop//2//31.txt";
	std::string fileDir2 = "C://Users//tq979//Desktop//2//32.txt";
	std::string fileDir3 = "C://Users//tq979//Desktop//2//33.txt";
	fileDirVector.push_back(fileDir1);
	fileDirVector.push_back(fileDir2);
	fileDirVector.push_back(fileDir3);
	
	getShortDir("D://users");
	
	std::cout<<"dir:"<<fileDirVector.at(0)<<"dirShort:"<<fileDirOutVector.at(0)<<std::endl;
	std::cout<<"dir:"<<fileDirVector.at(1)<<"dirShort:"<<fileDirOutVector.at(1)<<std::endl;
	std::cout<<"dir:"<<fileDirVector.at(2)<<"dirShort:"<<fileDirOutVector.at(2)<<std::endl;
	/**/
	makeNewDirFromFilePath(fileDirOutVector.at(0));
	//makeNewDirFromFilePath("D:");
	
	
	FILE *file,*outputFile;
	file = fopen(fileDir.c_str(),"rb");
	outputFile = fopen(outputDir.c_str(),"wb");
	if(!file || !outputFile){
		std::cout<<"oops"<<std::endl;
	}
	
	int fileHandle = open(fileDir.c_str(),0x0100);
	int fileLen = (int)filelength(fileHandle);
	
	struct fileInfo fInfo;
	fInfo.fileDirect = fileDir;
	fInfo.fileLen = fileLen;
	int headLen = sizeof(fileInfo);
	
	fwrite(&headLen, sizeof(int), 1, outputFile);
	fwrite(&fInfo, sizeof(fileInfo), 1, outputFile);
	unsigned char buf[MAXLEN];
	int re = 1;
	while((re = fread(buf,sizeof(unsigned char),MAXLEN,file)) != 0){
		fwrite(buf,sizeof(unsigned char),re,outputFile);
		for(int temp = 0; temp != re; temp++) {
		std::cout<<buf[temp]<<temp<<std::endl; 
		}
	 }
	std::cout<<re<<std::endl;
	fclose(file);
	fclose(outputFile);
	return 0;
}
