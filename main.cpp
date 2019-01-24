#include <iostream>
#include <iomanip>
#include <sstream>
#include <streambuf>
#include <fstream>
#include <string.h>
#include "jsoner.h"

#define BUFF_SIZE 4096

using namespace std;

using namespace J;

int main(int argc, char **argv)
{
    fstream in;
    char buff[BUFF_SIZE];

    if (argc!=2){
        cout << "usage " << argv[0] << " [.json]" << endl;
        exit(1);
    }

    in.open(argv[1], ios_base::in);

    if (!in){
        cout << "cannot open file" << endl;
        exit(1);
    }

    in.read(buff, BUFF_SIZE);

    streamsize ret=in.gcount();

    string line(buff, ret);

    JSON test;

    test.Parse(line);

    cout << test.toStr() << endl;

    return 0;
}
