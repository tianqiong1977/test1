#include<iostream>
#include<ostream>
#include<string>
#include<io.h>
#include <direct.h>
#define MAXLEN 512

struct fileInfo{
	std::string fileDirect;
	int fileLen;	
};

int main(){
	std::string fileDir = "C://Users//tq979//Desktop//2//1.txt";
	std::string outputDir = "2.tar";
	
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
	
	unsigned char buf[MAXLEN];
	int re = 1;
	while((re = fread(buf,sizeof(unsigned char),MAXLEN,file)) != 0){
		fwrite(buf,sizeof(unsigned char),re,outputFile);
			
	}
	
	std::cout<<re<<std::endl;
	fclose(file);
	fclose(outputFile);
	return 0;
}
