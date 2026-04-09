/*
 * papagaiocc_win.c — Native Windows Standalone Compiler Driver
 * 
 * This tool replaces the bash script on Windows, removing the dependency 
 * on a POSIX shell (Git Bash/MSYS2). It uses Windows Resources to store 
 * the embedded binaries and headers.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <process.h>

// Resource IDs (must match papagaiocc.rc)
#define IDR_PAPAGAIO_EXE  101
#define IDR_CLANG_EXE     102
#define IDR_SDK_H         103
#define IDR_PRE_PAP       104

BOOL ExtractResource(int resId, const char* outPath) {
    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(resId), RT_RCDATA);
    if (!hRes) return FALSE;

    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) return FALSE;

    void* pData = LockResource(hData);
    DWORD dwSize = SizeofResource(NULL, hRes);

    FILE* f = fopen(outPath, "wb");
    if (!f) return FALSE;

    fwrite(pData, 1, dwSize, f);
    fclose(f);
    return TRUE;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: papagaiocc <input.c> [output.wasm]\n");
        return 1;
    }

    const char* input_file = argv[1];
    char output_file[MAX_PATH];
    if (argc >= 3) {
        strcpy(output_file, argv[2]);
    } else {
        // Derive output name from input
        strcpy(output_file, input_file);
        char* ext = strrchr(output_file, '.');
        if (ext) *ext = '\0';
        strcat(output_file, ".wasm");
    }

    // 1. Create temporary directory
    char temp_dir[MAX_PATH];
    GetTempPath(MAX_PATH, temp_dir);
    strcat(temp_dir, "papagaio_");
#ifdef PAP_VERSION
    strcat(temp_dir, PAP_VERSION);
#else
    strcat(temp_dir, "unknown");
#endif
    CreateDirectory(temp_dir, NULL);

    // 2. Extract tools
    char pap_path[MAX_PATH], clang_path[MAX_PATH], lib_path[MAX_PATH], pre_path[MAX_PATH];
    sprintf(pap_path,   "%s\\papagaio.exe", temp_dir);
    sprintf(clang_path, "%s\\clang.exe",    temp_dir);
    sprintf(lib_path,   "%s\\lib.c",        temp_dir);
    sprintf(pre_path,   "%s\\preprocessor.pap", temp_dir);

    // Only extract if they don't exist (optimization)
    if (access(pap_path, 0) != 0) {
        printf("Prepare: Extracting Papagaio engine...\n");
        ExtractResource(IDR_PAPAGAIO_EXE, pap_path);
    }
    
    // Check if we have embedded clang
    BOOL has_embedded_clang = FindResource(NULL, MAKEINTRESOURCE(IDR_CLANG_EXE), RT_RCDATA) != NULL;
    if (has_embedded_clang && access(clang_path, 0) != 0) {
        printf("Prepare: Extracting Clang toolchain (this may take a few seconds)...\n");
        ExtractResource(IDR_CLANG_EXE, clang_path);
    } else if (!has_embedded_clang) {
        strcpy(clang_path, "clang.exe"); // Fallback to system clang
    }

    ExtractResource(IDR_SDK_H, lib_path);
    ExtractResource(IDR_PRE_PAP, pre_path);

    // 3. Prepare joined source
    char joined_path[MAX_PATH];
    sprintf(joined_path, "%s\\joined.pap", temp_dir);
    
    FILE* fpre = fopen(pre_path, "rb");
    FILE* fin  = fopen(input_file, "rb");
    FILE* fout = fopen(joined_path, "wb");
    
    if (fpre && fin && fout) {
        char buf[8192];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), fpre)) > 0) fwrite(buf, 1, n, fout);
        while ((n = fread(buf, 1, sizeof(buf), fin)) > 0)  fwrite(buf, 1, n, fout);
    }
    if (fpre) fclose(fpre);
    if (fin)  fclose(fin);
    if (fout) fclose(fout);

    // 4. Run Papagaio Preprocessor
    char ready_path[MAX_PATH];
    sprintf(ready_path, "%s\\ready.c", temp_dir);
    
    char cmd[4096];
    sprintf(cmd, "\"%s\" \"%s\" > \"%s\"", pap_path, joined_path, ready_path);
    if (system(cmd) != 0) {
        fprintf(stderr, "Error: Preprocessing failed.\n");
        return 1;
    }

    // 5. Run Clang
    printf("Compiling: %s -> %s\n", input_file, output_file);
    sprintf(cmd, "\"%s\" --target=wasm32-unknown-unknown -O2 -ffreestanding -fno-builtin -nostdlib -nostdinc "
                 "-mno-bulk-memory -mno-sign-ext -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined "
                 "-Wl,-z,stack-size=65536 \"%s\" -o \"%s\" -I \"%s\"",
                 clang_path, ready_path, output_file, temp_dir);
    
    int ret = system(cmd);
    if (ret == 0) {
        printf("Success: %s generated.\n", output_file);
    } else {
        fprintf(stderr, "Error: Compilation failed.\n");
    }

    return ret;
}
