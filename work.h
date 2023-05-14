#ifndef WORK_H
#define WORK_H
#include "list.h"
typedef struct {
  char* name;
  char* hash;
  int mode;
} WorkFile;

typedef struct {
  WorkFile* tab;
  int size;
  int n;
} WorkTree;

int getChmod(const char* path);
void setMode(int mode, char* path);
WorkFile* createWorkFile(char* name);
char* wfts(WorkFile* wf);
WorkFile* stwf(char* ch);
WorkTree* initWorkTree(void);
void freeWorkTree(WorkTree* wt);
int inWorkTree(WorkTree* wt, char* name);
int appendWorkTree(WorkTree* wt, char* name, char* hash, int mode);
char* wtts(WorkTree* wt);
WorkTree* stwt(char* s);
int wttf(WorkTree* wt, char* file);
WorkTree* ftwt(char* file);
char* blobWorkTree(WorkTree* wt);
char* saveWorkTree(WorkTree* wt, char* path);
int isWorkTree(char* hash);
void restoreWorkTree(WorkTree* wt, char* path);
char* concat_paths(char* path1, char* path2);
WorkTree* mergeWorkTrees(WorkTree* wt1, WorkTree* wt2, List** conflicts);
int isFile(const char* path);

#endif
