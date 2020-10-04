//
// Created by baptiste on 04/10/2020.
//

#ifndef UTILS_H
#define UTILS_H

namespace utils {
    char* getObjectFilePath(const char* name);
    char* getTextureFilePath(const char* name);
    char* getShaderFilePath(const char* name);
    char* getSkyboxFilePath(const char* name);
    char* appendFileToFolder(const char* folder, const char* fname);
}

#endif //UTILS_H
