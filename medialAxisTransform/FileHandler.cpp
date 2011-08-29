//
//  FileHandler.h
//  openCLImageLoad
//
//  Created by Beau Johnston on 25/08/11.
//  Copyright 2011 University Of New England. All rights reserved.
//

#include "FileHandler.h"

using namespace std;

/* hidden private variables */
vector<string> files;
string * orderedFiles;
int traverser;
string path;

template <class T> bool from_string(T& t, 
                                    const std::string& s, 
                                    std::ios_base& (*f)(std::ios_base&))
{
    std::istringstream iss(s);
    return !(iss >> f >> t).fail();
}



int getdir (string dir, string ext, vector<string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        cout << "Error(" << errno << ") opening " << dir << endl;
        return errno;
    }
    
    while ((dirp = readdir(dp)) != NULL) {
        if (string(dirp->d_name).find(ext) != -1) {
            files.push_back(string(dirp->d_name));
            
        }
    }
    closedir(dp);
    return 0;
}

int getFilesInDirectoryWithName (string dir, string name, vector<string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        cout << "Error(" << errno << ") opening " << dir << endl;
        return errno;
    }
    
    while ((dirp = readdir(dp)) != NULL) {
        if (string::npos != string(dirp->d_name).find(name)) {
            files.push_back(string(dirp->d_name));
        }
    }
    closedir(dp);
    return 0;
}

void generateListOfAssociatedFiles(char* filename)  
{
    
    string handedInString = filename;
    string file = handedInString.substr(handedInString.find_last_of('/')+1);
    
    path = handedInString.substr(0, handedInString.find_last_of('/')+1);
    
    string cutDownFile = file.substr(0, file.find_last_of('.'));
    string extension = file.substr(file.find_last_of('.'));
    
    //    cout << "file has name: " << file << endl;
    //    cout << "path looks like this: " << path << endl;
    //    cout << "cut down file looks like this: " << cutDownFile << endl;
    //    cout << "extension looks like: " << extension << endl;
    
    string dir = string(path);
    files = vector<string>();
    
    if (cutDownFile == "") {
        getdir(dir, extension, files);
    } else {
        getFilesInDirectoryWithName(dir,cutDownFile,files);
    }
    
    
    traverser = 0;
    sortFilesNumerically(cutDownFile, extension);
    //    for (unsigned int i = 0;i < files.size();i++) {
    //        cout << files[i] << endl;
    //    }
    return;
}


string remove(string str, string substr){
	for( int i = str.find(substr,0); i != string::npos; i=str.find(substr,i) )
    {
		str.erase( i,substr.length() );
		++i;
	}
	return str;
}

bool doesNumberingStartAt0(string name, string ext){
    int lowest = numberOfFiles();
    
    for (int j=0; j < numberOfFiles(); j++) {
        int i;
        string tmp = remove(files[j], name);
        string inNumber = remove(tmp, ext);
        
        if(from_string<int>(i, std::string(inNumber), std::dec))
        {
            if (i < lowest) {
                lowest = i;
            }
        }
    }
    
    if (lowest == 0) {
        return true;
    }
    else{
        return false;
    }
}

bool doesNumberingStartAt1(string name, string ext){
    int lowest = numberOfFiles();
    
    for (int j=0; j < numberOfFiles(); j++) {
        int i;
        string tmp = remove(files[j], name);
        string inNumber = remove(tmp, ext);
        
        if(from_string<int>(i, std::string(inNumber), std::dec))
        {
            if (i < lowest) {
                lowest = i;
            }
        }
    }
    
    if (lowest == 1) {
        return true;
    }
    else{
        return false;
    }
}

bool noFilesAreMissing(string name, string ext){
    int lowest = numberOfFiles();
    int highest = 0;
    vector<int> tempVec = vector<int>();
    
    for (int j=0; j < numberOfFiles(); j++) {
        int i;
        string tmp = remove(files[j], name);
        string inNumber = remove(tmp, ext);
        
        if(from_string<int>(i, std::string(inNumber), std::dec))
        {
            if (i < lowest) {
                lowest = i;
            }
            if (i > highest) {
                highest = i;
            }
            tempVec.push_back(i);
        }
    }
    
    int tempArray[highest-lowest];
    //populate with garbage
    for (int j = 0; j < tempVec.size(); j++) {
        tempArray[j] = -1;
    }
    
    //populate with real values
    for (int j = 0; j < tempVec.size(); j++) {
        tempArray[tempVec[j]] = tempVec[j];
    }
    
    for (int k = lowest; k < highest; k++) {
        if(tempArray[k]==-1){
            cout << " Image number : " << k << " dosn't exist!" << endl;
            cout << " It is compulsory that the image stack be fully populated" << endl;
            cout << " Please fix this and try again!" << endl;
            return false;
        }
    }
    return true;
}

void sortFilesNumerically(string name, string ext){
    
    if (!doesNumberingStartAt1(name, ext)&&!doesNumberingStartAt0(name, ext)){
        cout << "Error, numbering for image stack doesn't start at 0 or 1" << endl;
        return;
    }
    
    bool startingAtZero = doesNumberingStartAt0(name, ext);
    
    
    if (!noFilesAreMissing(name, ext)) {
        cout << "ERROR: files are missing, returning!"<< endl;
        return;
    }
    
    orderedFiles =  new string [numberOfFiles()];
    
    for (int j=0; j < numberOfFiles(); j++) {
        int i;
        string tmp = remove(files[j], name);
        string inNumber = remove(tmp, ext);
        
        if(from_string<int>(i, std::string(inNumber), std::dec))
        {
            cout << i << endl;
            if (startingAtZero) {
                orderedFiles[i] = files[j];
            } else{
                orderedFiles[i-1] = files[j];
            }
        }
    }
}

void printFiles(void){
    for (unsigned int i = 0;i < files.size();i++) {
        //        cout << files[i] << endl;
        cout << orderedFiles[i] << endl;
    }
    return;
}

char* getNextFileName(void){
    traverser ++;
    //    return (char*)(path+files[traverser-1]).c_str();
    return (char*)(path+orderedFiles[traverser-1]).c_str();
}

bool areFilesLeft(void){
    if (traverser < files.size()+1) {
        return true;
    }
    return false;
}

int numberOfFiles(void){
    return (int)files.size();
}
