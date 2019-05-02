//
// Created by anastasia on 13.04.19.
//

#include <string>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <dirent.h>
#include <sys/stat.h>
#include "shell.h"

struct Worker {
    std::string Directory;
    ino_t Inode;
    std::string Name;
    uint64_t Size;
    nlink_t Nlinks;
    std::string Exec;

    bool hasInode = false;
    bool hasName = false;
    int SizeLimit = 2;
    bool hasNlinks = false;

    void parseArgs(int argc, char *argv[]) {
        if (argc < 2) {
            throw std::invalid_argument("Path argument not found");
        } else if (argc % 2) {
            throw std::invalid_argument("Wrong number of arguments");
        }

        Directory = argv[1];
        if (Directory.back() == '/') {
            Directory.pop_back();
        }

        for (int i = 2; i < argc; ++i) {
            if (strcmp(argv[i], "-inum") == 0) {
                try {
                    Inode = std::stoull(argv[++i]);
                    hasInode = true;
                } catch (std::logic_error const &e) {
                    throw std::invalid_argument(std::string("Wrong -inum argument: ") + std::string(e.what()));
                }
            } else if (strcmp(argv[i], "-name") == 0) {
                Name = argv[++i];
                hasName = true;
            } else if (strcmp(argv[i], "-size") == 0) {
                char c = argv[++i][0];
                if (c == '=') {
                    SizeLimit = 0;
                } else if (c == '-') {
                    SizeLimit = -1;
                } else if (c == '+') {
                    SizeLimit = 1;
                } else {
                    throw std::invalid_argument("Wrong -size argument (+/-/= expected)");
                }
                try {
                    std::string strSize = argv[i];
                    strSize = strSize.substr(1);
                    Size = std::stoull(strSize);
                } catch (std::logic_error const &e) {
                    throw std::invalid_argument(std::string("Wrong -size argument: ") + std::string(e.what()));
                }
            } else if (strcmp(argv[i], "-nlinks") == 0) {
                try {
                    Nlinks = std::stoull(argv[++i]);
                    hasNlinks = true;
                } catch (std::logic_error const &e) {
                    throw std::invalid_argument(std::string("Wrong -nlinks argument: ") + std::string(e.what()));
                }
            } else if (strcmp(argv[i], "-exec") == 0) {
                Exec = argv[++i];
            } else {
                throw std::invalid_argument(std::string("No such modifier ") + argv[i]);
            }
        }
    }

    void walk(std::string const &path) {
        auto dir = opendir(path.c_str());
        if (dir == nullptr)
            return;
        while (auto file = readdir(dir)) {
            std::string filename = file->d_name;
            if (filename == "" || filename == "." || filename == "..")
                continue;
            auto type = file->d_type;
            std::string nPath = path + "/" + filename;
            if (type == DT_DIR) {
                walk(nPath);
            } else if (type == DT_REG) {
                struct stat curStat;
                if (stat(nPath.data(), &curStat)) {
                    std::cerr << "Can't get stat about " << nPath << ": " << strerror(errno) << std::endl;
                } else {
                    if (checkFile(nPath, curStat)) {
                        if (Exec.size()) {
                            shell::execute({ Exec, nPath });
                        } else {
                            std::cout << nPath << std::endl;
                        }
                    }
                }
            }
        }
        closedir(dir);
    }

    bool checkFile(std::string const &path, struct stat const &fileStat) {
        auto check = [](bool has, auto const &file, auto const &need) {
            return !has || file == need;
        };
        auto checkSize = [&](auto const &size) {
            return SizeLimit == 2 ||
                   SizeLimit == 0 && size == Size ||
                   SizeLimit == 1 && size > Size ||
                   SizeLimit == -1 && size < Size;
        };
        return check(hasInode, fileStat.st_ino, Inode) &&
               check(hasNlinks, fileStat.st_nlink, Nlinks) &&
               check(hasName, path.substr(path.find_last_of('/') + 1), Name) &&
               checkSize(fileStat.st_size);
    }

};

int main(int argc, char *argv[]) {
    try {
        Worker worker;
        worker.parseArgs(argc, argv);
        worker.walk(worker.Directory);
    } catch (std::invalid_argument &e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
