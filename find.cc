#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <time.h>
#include <regex>

using namespace std;

struct sizes {
    bool character = false;
    bool greater = false;
    bool less = false;
    bool exact = false;
    int count = -1;
};

string doName(int i, int argc, char *argv[]) {
    if (i >= argc) {
        cout << "-name requires argument" << endl;
        exit(-1);
    }
    return argv[i];
}

string doType(int i, int argc, char *argv[]) {
    if (i >= argc) {
        cout << "-type requires argument" << endl;
        exit(-1);
    }
    string type = argv[i];
    if (!(type == "f" || type == "d" || type == "l")) {
        cout << "Unsupported file type provided" << endl;
        exit(-1);
    } else {
        return argv[i];
    }
}

sizes doSize(int i, int argc, char *argv[]) {
    if (i >= argc) {
        cout << "-size requires argument" << endl;
        exit(-1);
    }
    string sizeString = argv[i];
    sizes size;
    regex numbers("[1-9][0-9]*");
    regex greaterThanCharacter("\\+[1-9][0-9]*c");
    regex lessThanCharacter("\\-[1-9][0-9]*c");
    regex exactCharacter("[1-9][0-9]*c");
    regex greaterThanBlock("\\+[1-9][0-9]*");
    regex lessThanBlock("\\-[1-9][0-9]*");
    regex exactBlock("[1-9][0-9]*");
    if (regex_match(sizeString, greaterThanCharacter)) {
        size.character = true;
        size.greater = true;
    } else if (regex_match(sizeString, lessThanCharacter)) {
        size.character = true;
        size.less = true;
    } else if (regex_match(sizeString, exactCharacter)) {
        size.character = true;
        size.exact = true;
    } else if (regex_match(sizeString, greaterThanBlock)) {
        size.greater = true;
    } else if (regex_match(sizeString, lessThanBlock)) {
        size.less = true;
    }else if (regex_match(sizeString, exactBlock)) {
        size.exact = true;
    } else {
        cout << sizeString << " invalid size specification" << endl;
        exit(-1);
    }
    smatch match;
    regex_search(sizeString, match, numbers);
    if (match.empty()) {
        cout << sizeString << " could not extract integer size value" << endl;
        exit(-1);
    }
    size.count = stoi(match.str());
    return size;
}

bool isDirectory(string file) {
    struct stat statbuf;
    int ret;
    int ftype;
	ret = lstat(file.c_str(), &statbuf);
    if (ret != 0) {
	    perror(file.c_str());
        exit(-1);
    }
    ftype = statbuf.st_mode & S_IFMT;
    return ftype == S_IFDIR;
}

DIR* openDirectory(string directory, DIR *dp) {
    if ((dp = opendir(directory.c_str())) == NULL) {
        perror(directory.c_str());
        exit(-1);
    }
    return dp;
}

bool isCurrentOrParentReference(char *fileName) {
    if (strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0) {
        return true;
    }
    return false;
}

bool isNameAllowed(string fileName, string nameFilter) {
    // default no name filter provided
    if (nameFilter == "") {
        return true;
    } else if (fileName == nameFilter) {
        return true;
    } else {
        return false;
    }
}

bool isTypeAllowed(string fileName, string typeFilter) {
    // default no type filter provided
    if (typeFilter == "") {
        return true;
    }
    struct stat statbuf;
    int ret;
    int ftype;
    ret = lstat(fileName.c_str(), &statbuf);
    if (ret != 0) {
	    perror(fileName.c_str());
	    exit(-1);
    }
    ftype = statbuf.st_mode & S_IFMT;
    if (typeFilter == "f") {
        return ftype == S_IFREG;
    } else if (typeFilter == "d") {
        return ftype == S_IFDIR;
    } else {
        return ftype == S_IFLNK;
    }
}

bool isSizeAllowed(string fileName, sizes size) {
    // default no size filter provided
    if (size.count == -1) {
        return true;
    }
    struct stat statbuf;
    int ret;
	ret = lstat(fileName.c_str(), &statbuf);
    if (ret != 0) {
	    perror(fileName.c_str());
	    exit(-1);
    }
    int charSize = sizeof(char);
    int charCount = statbuf.st_size / charSize;
    if (size.greater && size.character) {
        return charCount > size.count;
    } else if (size.less && size.character) {
        return charCount < size.count;
    } else if (size.exact && size.character) {
        return charCount == size.count;
    } else if (size.greater) {
        return statbuf.st_size > size.count * 512;
    } else if (size.less) {
        return statbuf.st_size < size.count * 512;
    } else {
        return statbuf.st_size == size.count * 512;
    }
}

void recurseDirectory(DIR *dp, string directory, string nameFilter, string typeFilter, sizes sizeFilter) {
    struct dirent *dirp;
    dirp = readdir(dp);
    if (dirp == NULL) {
        return;
    }
    string file = directory + "/" + dirp->d_name;
    if (!isCurrentOrParentReference(dirp->d_name) && isNameAllowed(dirp->d_name, nameFilter) && isTypeAllowed(file, typeFilter) && isSizeAllowed(file, sizeFilter)) {
        cout << file << endl;
    }
    if (isDirectory(file) && !isCurrentOrParentReference(dirp->d_name)) {
        DIR *subDp = NULL;
        subDp = openDirectory(file, subDp);
        recurseDirectory(subDp, file, nameFilter, typeFilter, sizeFilter);
    }
    recurseDirectory(dp, directory, nameFilter, typeFilter, sizeFilter);

}

void find(string directory, string nameFilter, string typeFilter, sizes sizeFilter) {
    DIR *dp = NULL;
    dp = openDirectory(directory, dp);
    recurseDirectory(dp, directory, nameFilter, typeFilter, sizeFilter);
    if (closedir(dp) == -1) {
        perror("closedir");
        exit(-2);
    }
}

void findWithFilters(string directory, int argc, char *argv[]) {
    int i = 2;
    string nameFilter = "";
    string typeFilter = "";
    sizes sizeFilter;
    while (i < argc) {
        string filter = argv[i];
        if (filter == "-name") {
            i++;
            nameFilter = doName(i, argc, argv);
        } else if (filter == "-type") {
            i++;
            typeFilter = doType(i, argc, argv);
        } else if (filter == "-size") {
            i++;
            sizeFilter = doSize(i, argc, argv);
        } else {
            cout << "Unknown filter: " << argv[i] << endl;
            exit(-1);
        }
        i++;
    }
    if (isNameAllowed(directory, nameFilter) && isTypeAllowed(directory, typeFilter) && isSizeAllowed(directory, sizeFilter)) {
        cout << directory << endl;
    }
    find(directory, nameFilter, typeFilter, sizeFilter);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Not enough arguments" << endl;
        exit(-1);
    }
    string directory = argv[1];
    if (argc < 3) {
        sizes noFilterSize;
        cout << directory << endl;
        find(directory, "", "", noFilterSize);
    } else {
        findWithFilters(directory, argc, argv);
    }
    return 0;
}
