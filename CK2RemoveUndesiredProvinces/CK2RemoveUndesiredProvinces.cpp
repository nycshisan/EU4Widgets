#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>

#include "PGConf.hpp"

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::unordered_set;

int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "没有指定删除列表文件！" << endl;
    }
    
    unordered_set<string> remove_empire, remove_kingdom;

    for (int i = 1; i < argc; ++i) {
        std::ifstream ifs(argv[i]);
        if (!ifs.good()) {
            cout << "打开\"" + string(argv[i]) + "\"失败！" << endl;
        } else {
            cout << "处理文件：" + string(argv[i]) << "..." << endl;
        }
        while (!ifs.eof()) {
            string line;
            ifs >> line;
            switch (line[0]) {
            case 'e':
                remove_empire.emplace(line);
                break;
            case 'k':
                remove_kingdom.emplace(line);
                break;
            default:
                cout << "错误的删除项：" + line << endl;
            }
        }
    }
    cout << "读取删除列表完成。" << endl;

    PGConf::ConfFile ltFile;
    ltFile.readFromFile("landed_titles.txt");
    ltFile.writeToFile("landed_titles2.txt");
}