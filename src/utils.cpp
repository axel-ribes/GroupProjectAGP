//
// Created by baptiste on 04/10/2020.
//

#include "utils.h"
#include <string.h>

namespace utils {
    char* getObjectFilePath(const char* name) {
        return appendFileToFolder("../resources/object/" ,name);
    }

    char* getTextureFilePath(const char* name) {
        return appendFileToFolder("../resources/texture/" ,name);
    }

    char* getShaderFilePath(const char* name) {
        return appendFileToFolder("../resources/shader/" ,name);
    }

    char* getSkyboxFilePath(const char* name) {
        return appendFileToFolder("../resources/skybox/" ,name);
    }

    char* appendFileToFolder(const char* folder, const char* fname) {
        char* result = new char[strlen(folder) + strlen(fname)];
        strcpy(result, folder);
        strcat(result, fname);

        return result;
    }
}

