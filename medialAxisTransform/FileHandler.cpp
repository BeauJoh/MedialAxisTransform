/*
 *  FileHandler.cpp
 *  MedialAxisTransform
 *
 *
 *  Created by Beau Johnston on 25/08/11.
 *  Copyright (C) 2011 by Beau Johnston.
 *
 *  Please email me if you have any comments, suggestions or advice:
 *                              beau@inbeta.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */


#include "FileHandler.h"

FileHandler::FileHandler(){
    
}

FileHandler::~FileHandler(){
    
}

int FileHandler::getdir (string dir, string ext, vector<string> &files)
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

int FileHandler::getFilesInDirectoryWithName (string dir, string name, vector<string> &files)
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

void FileHandler::generateListOfAssociatedFiles(char* filename)  
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


string FileHandler::remove(string str, string substr){
	for( int i = (int)str.find(substr,0); i != string::npos; i=(int)str.find(substr,i) )
    {
		str.erase( i,substr.length() );
		++i;
	}
	return str;
}

bool FileHandler::doesNumberingStartAt0(string name, string ext){
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

bool FileHandler::doesNumberingStartAt1(string name, string ext){
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

bool FileHandler::noFilesAreMissing(string name, string ext){
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

void FileHandler::sortFilesNumerically(string name, string ext){
    
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

void FileHandler::printFiles(void){
    for (unsigned int i = 0;i < files.size();i++) {
        //        cout << files[i] << endl;
        cout << orderedFiles[i] << endl;
    }
    return;
}

char* FileHandler::getNextFileName(void){
    traverser ++;
    //    return (char*)(path+files[traverser-1]).c_str();
    return (char*)(path+orderedFiles[traverser-1]).c_str();
}

bool FileHandler::areFilesLeft(void){
    if (traverser < files.size()+1) {
        return true;
    }
    return false;
}

int FileHandler::numberOfFiles(void){
    return (int)files.size();
}
