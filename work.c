#include "work.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "hash.h"
#include "list.h"
#define SIZETREE 10

// Fonction donnée dans le sujet
// Permet de récupérer le mode d'un fichier
int getChmod(const char* path) {
  struct stat ret;
  if (stat(path, &ret) == -1) {
    return -1;
  }
  return (ret.st_mode & S_IRUSR) | (ret.st_mode & S_IWUSR) |
         (ret.st_mode & S_IXUSR) | /* owner*/
         (ret.st_mode & S_IRGRP) | (ret.st_mode & S_IWGRP) |
         (ret.st_mode & S_IXGRP) | /* group*/
         (ret.st_mode & S_IROTH) | (ret.st_mode & S_IWOTH) |
         (ret.st_mode & S_IXOTH); /* other*/
}

// Fonction donnée dans le sujet
// Permet de modifier le mode d'un fichier
void setMode(int mode, char* path) {
  char buff[100];
  sprintf(buff, "chmod %o %s", mode, path);
  system(buff);
}

// Crée un WorkFile et l'initialise
WorkFile* createWorkFile(char* name) {
  WorkFile* w = (WorkFile*)malloc(sizeof(WorkFile));
  w->name = strdup(name);
  w->hash = NULL;
  w->mode = 0;
  return w;
}

// WorkFile to String
// Affiche un WorkFile sous la forme "name hash mode"
char* wfts(WorkFile* wf) {
  char* s = malloc(256 * sizeof(char));
  sprintf(s, "%s\t%s\t%d", wf->name, wf->hash, wf->mode);
  return s;
}

// String to WorkFile
// Crée un WorkFile à partir d'une chaîne de caractères
WorkFile* stwf(char* ch) {
  char* name = malloc(sizeof(char) * 1000);
  char* hash = malloc(sizeof(char) * 1000);
  int mode;
  sscanf(ch, "%s\t%s\t%d", name, hash, &mode);
  WorkFile* wf = createWorkFile(name);
  free(name);
  wf->mode = mode;
  wf->hash = hash;
  return wf;
}

// Initialise un WorkTree et le retourne
WorkTree* initWorkTree() {
  WorkTree* wt = (WorkTree*)malloc(sizeof(WorkTree));
  wt->size = SIZETREE;
  wt->n = 0;
  wt->tab = (WorkFile*)malloc(SIZETREE * sizeof(WorkFile));
  for (int i = 0; i < SIZETREE; i++) {
    wt->tab[i].name = NULL;
    wt->tab[i].hash = NULL;
    wt->tab[i].mode = 0;
  }
  return wt;
}

// Libère un WorkTree
void freeWorkTree(WorkTree* wt) {
  if (wt == NULL) return;
  if (wt->tab) {
    WorkFile* tab = wt->tab;
    for (int i = 0; i < wt->n; i++) {
      free(tab[i].name);
      if (tab[i].hash != NULL) free(tab[i].hash);
    }
    free(wt->tab);
  }
  free(wt);
}

// Vérifie la présence d'un fichier ou d'un répertoire dans un WorkTree
// Retourne l'indice du fichier dans le tableau si il est présent et -1 sinon
int inWorkTree(WorkTree* wt, char* name) {
  for (int i = 0; i < wt->n; i++) {
    if (strcmp(wt->tab[i].name, name) == 0) {
      return i;
    }
  }
  return -1;
}

// Ajoute un fichier ou un répertoire dans un WorkTree
int appendWorkTree(WorkTree* wt, char* name, char* hash, int mode) {
  int i = inWorkTree(wt, name);
  if (i != -1) {
    printf("Le fichier %s est déjà présent dans l'arbre\n", name);
    return -1;
  }

  if (wt->n >= wt->size) {
    printf("L'arbre est plein, impossible d'ajouter un autre fichier\n");
    return -1;
  }

  // Ajouter le nouveau WorkFile au WorkTree
  wt->tab[wt->n].name = strdup(name);
  if (hash != NULL) wt->tab[wt->n].hash = strdup(hash);
  wt->tab[wt->n].mode = mode;

  wt->n++;

  return 0;
}

// WorkTree to String
// Affiche un WorkTree sous la forme "name hash mode \n name hash mode \n ..."
char* wtts(WorkTree* wt) {
  char* s = malloc(10000 * sizeof(char));
  for (int i = 0; i < wt->n; i++) {
    char* w = wfts(&wt->tab[i]);
    strcat(s, w);
    strcat(s, "\n");
    free(w);
  }
  return s;
}

// String to WorkTree
// Crée un WorkTree à partir d'une chaîne de caractères
WorkTree* stwt(char* ch) {
  int pos = 0;
  int n_pos = 0;
  int sep = '\n';
  char* ptr;
  char* result = malloc(sizeof(char) * 10000);
  WorkTree* wt = initWorkTree();
  while (pos < strlen(ch)) {
    ptr = strchr(ch + pos, sep);
    if (ptr == NULL)
      n_pos = strlen(ch) + 1;
    else {
      n_pos = ptr - ch + 1;
    }
    memcpy(result, ch + pos, n_pos - pos - 1);
    result[n_pos - pos - 1] = '\0';
    pos = n_pos;
    WorkFile* wf = stwf(result);
    appendWorkTree(wt, wf->name, wf->hash, wf->mode);
    free(wf->name);
    free(wf->hash);
    free(wf);
  }
  free(result);
  return wt;
}

// WorkTree to File
// Ecrit un WorkTree dans un fichier
int wttf(WorkTree* wt, char* path) {
  FILE* fp = fopen(path, "w");
  if (fp != NULL) {
    char* wts = wtts(wt);
    fputs(wts, fp);
    fclose(fp);
    free(wts);
    return 0;
  }
  return -1;
}

// File to WorkTree
// Crée un WorkTree à partir d'un fichier
WorkTree* ftwt(char* file) {
  FILE* fp = fopen(file, "r");
  if (fp == NULL) {
    return NULL;
  }
  WorkTree* wt = initWorkTree();
  char buffer[256];
  while (fgets(buffer, 256, fp) != NULL) {
    WorkFile* wf = stwf(buffer);
    appendWorkTree(wt, wf->name, wf->hash, wf->mode);
    free(wf->name);
    if(wf->hash != NULL) free(wf->hash);
    free(wf);
  }
  fclose(fp);
  return wt;
}

// Crée un fichier temporaire représentant le WorkTree pour pouvoir ensuite
// créer l’enregistrement instantané du WorkTree (avec l’extension ".t").

char* blobWorkTree(WorkTree* wt) {
  static char template[] = "/tmp/testFileXXXXX";
  char fname[100];
  strcpy(fname, template);
  int fd = mkstemp(fname);
  wttf(wt, fname);
  char* hash = sha256file(fname);
  char* ch = hashToFile(hash);
  strcat(ch, ".t");
  cp(ch, fname);
  char command[256];
  sprintf(command, "rm %s", fname);
  system(command);
  free(ch);
  return hash;
}

char* concat_paths(char* path1, char* path2) {
  size_t result_size =
      strlen(path1) + strlen(path2) + 2;  // +2 pour le slash et le terminateur
  char* result = malloc(result_size);
  if (result == NULL) {
    printf("Erreur allocation\n");
    return NULL;
  }
  snprintf(result, result_size, "%s/%s", path1, path2);  // concatenation
  return result;
}

int isFile(const char* absPath) {
  struct stat path_stat;
  if (stat(absPath, &path_stat) != 0) {
    return 0;
  }
  return S_ISREG(path_stat.st_mode);
}

// Crée un enregistrement instantané du WorkTree
char* saveWorkTree(WorkTree* wt, char* path) {
  for (int i = 0; i < wt->n; i++) {
    // creation new chemin fichier
    char* pathf = concat_paths(path, wt->tab[i].name);

    // chemin correspond à un fichier ?
    if (isFile(pathf) != 0) {
      blobFile(pathf);
      char* hash = sha256file(pathf);  // sinon creation d'un hash
      wt->tab[i].hash = strdup(hash);
      wt->tab[i].mode = getChmod(pathf);
      free(hash);
    } else {
      List* l = listdir(pathf);  // new wt
      if (*l == NULL) {
        wt->tab[i].hash = NULL;
        wt->tab[i].mode = getChmod(pathf);
      } else {
        WorkTree* newWT = initWorkTree();
        Cell* c = *l;
        while (c) {
          appendWorkTree(newWT, c->data, NULL, 0);
          c = c->next;
        }
        char* hash = saveWorkTree(newWT, pathf);  // save wt dans new nom
        // maj des données
        wt->tab[i].hash = strdup(hash);
        wt->tab[i].mode = getChmod(pathf);
        // liberation memoire
        free(hash);
        freeWorkTree(newWT);
        libererListe(l);
      }
    }
    free(pathf);
  }
  return blobWorkTree(wt);
}

int isWorkTree(char* hash) {
  char* path = hashToPath(hash);
  char* directory_name = strtok(path, "/");
  char* file_name = strtok(NULL, "/");
  int result = file_exists_rep(file_name, directory_name);

  if (result == 1) {
    free(path);
    return 0;
  } else {
    strcat(file_name, ".t");
    result = file_exists_rep(file_name, directory_name);
  }
  free(path);
  if (result) return 1;
  return -1;
}

// Restaure un WorkTree à partir d'un fichier
void restoreWorkTree(WorkTree* wt, char* path) {
  for (int i = 0; i < wt->n; i++) {
    char* absPath = concat_paths(path, wt->tab[i].name);
    char* copyPath = hashToPath(wt->tab[i].hash);
    char* hash = wt->tab[i].hash;
    int is_wt = isWorkTree(hash);
    if (is_wt == 0) {  // si c'est un fichier
      cp(absPath, copyPath);
      setMode(getChmod(copyPath), absPath);
    } else if (is_wt == 1) {  // si c'est un repertoire
      strcat(copyPath, ".t");
      WorkTree* nwt = ftwt(copyPath);
      char* new_path = concat_paths(path, wt->tab[i].name);
      restoreWorkTree(nwt, new_path);
      setMode(getChmod(copyPath), absPath);
      free(new_path);
      freeWorkTree(nwt);
    }
    free(absPath);
    free(copyPath);
  }
}

WorkTree* mergeWorkTrees(WorkTree* wt1, WorkTree* wt2, List** conflicts) {
  if (wt1 == NULL) {
    return wt2;
  }
  if (wt2 == NULL) {
    return wt1;
  }
  WorkTree* result = initWorkTree();
  // on parcourt les deux worktrees
  for (int i = 0; i < wt1->n; i++) {
    for (int j = 0; j < wt2->n; j++) {
      // si les deux worktrees ont le meme nom
      if (strcmp(wt1->tab[i].name, wt2->tab[j].name) == 0) {
        // si les deux worktrees ont le meme hash
        if (strcmp(wt1->tab[i].hash, wt2->tab[j].hash) == 0) {
          // on ajoute le fichier dans le nouveau worktree
          appendWorkTree(result, wt1->tab[i].name, wt1->tab[i].hash,
                         wt1->tab[i].mode);
        } else {
          // on ajoute le fichier dans la liste des conflits
          insertFirst(*conflicts, buildCell(wt1->tab[i].name));
        }
      }
    }
  }
  return result;
}