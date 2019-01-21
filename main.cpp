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

    for (Jiter it=test.begin();it!=test.end();++it){
        if (it->Type()==JType::Object){
            cout << "Object: " << it->Name() << endl;

            Jiter inner=(it.getObj())->begin();

            if (inner.isNull()){
                cout << "fucked up shit" << endl;
                break;
            }

            for (;inner!=(it.getObj())->end();++inner){
                cout << inner->Name() << " : ";

                try {
                    cout << inner->getStr() << endl;
                } catch (...){
                    cout << "Not a string property" << endl;
                }
            }
        }
    }

    return 0;
}
