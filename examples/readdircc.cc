#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <iostream>

using namespace std;

int main(
    int argc,
    char *argv[])
{
    DIR *dp;
    struct dirent *dirp;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " directory_name\n";
        exit(-1);
    }

    if ((dp = opendir(argv[1])) == NULL) {
        perror(argv[1]);
        exit(-1);
    }

    while ((dirp = readdir(dp)) != NULL) {
        cout << dirp->d_name << endl;
    }

    if (closedir(dp) == -1) {
        perror("closedir");
        exit(-2);
    }

    exit(0);
}
