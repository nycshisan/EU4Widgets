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
    string removeListPath = "remove_list.txt",
           inputPath = "landed_titles.bak",
           outputPath = "landed_titles.txt";

    if (argc > 1) {
        removeListPath = string(argv[1]);
    }
    if (argc > 2) {
        inputPath = string(argv[2]);
    }
    if (argc > 3) {
        outputPath = string(argv[3]);
    }
    
    unordered_set<string> remove_empire, remove_kingdom;

    std::ifstream ifs(removeListPath);
    if (!ifs.good()) {
        cout << "打开删除列表\"" + string(removeListPath) + "\"失败！" << endl;
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
    cout << "读取删除列表完成。" << endl;

    PGConf::ConfFile ltFile;
    cout << "读取" << inputPath << endl;
    ltFile.readFromFile(inputPath);

    for (const auto &content : ltFile.contents) {
        auto scope = std::dynamic_pointer_cast<PGConf::Scope>(content);
        if (remove_empire.count(scope->getKey()) > 0) {
            scope->clear();
        } else {
            for (const auto &child : scope->children) {
                if (remove_kingdom.count(child->getKey()) > 0) {
                    auto childScope = std::dynamic_pointer_cast<PGConf::Scope>(child);
                    childScope->clear();
                }
            }
        }
    }

    cout << "输出至" << outputPath << endl;
    ltFile.writeToFile(outputPath);


}