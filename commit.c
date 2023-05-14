#include "commit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "hash.h"
#include "list.h"
#include "work.h"
#define SIZECOMMIT 30

// crée un nouveau key value pair
kvp *createKeyVal(char *key, char *val) {
  kvp *k = malloc(sizeof(kvp));
  k->key = strdup(key);
  k->value = strdup(val);
  return k;
}

// libère la mémoire allouée pour un key value pair
void freeKeyVal(kvp *kv) {
  free(kv->key);
  free(kv->value);
  free(kv);
}

// affiche un key value pair sous la forme "key : value"
char *kvts(kvp *k) {
  char *buff = malloc(sizeof(char) * 256);
  sprintf(buff, "%s : %s", k->key, k->value);
  return buff;
}

// crée un key value pair à partir d'une chaîne de caractères
kvp *stkv(char *str) {
  char key[100], val[100];
  sscanf(str, "%s : %s", key, val);
  return createKeyVal(key, val);
}

// crée un nouveau commit et l'initialise
Commit *initCommit() {
  Commit *c = malloc(sizeof(Commit));
  c->T = malloc(SIZECOMMIT * sizeof(kvp *));
  c->size = SIZECOMMIT;
  for (int i = 0; i < c->size; i++) {
    c->T[i] = NULL;
  }
  c->n = 0;
  return c;
}

// fonction de hashage
unsigned long hash(char *str) {
  unsigned long hash = 5381;
  int c;
  while ((c = *str++) != '\0')
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  return hash;
}

// set un key value pair dans un commit
void commitSet(Commit *c, char *key, char *value) {
  int p = hash(key) % c->size;
  while (c->T[p] != NULL) {
    p = (p + 1) % c->size;  // probing lineaire
  }
  c->T[p] = createKeyVal(key, value);
  c->n++;
}

// libère la mémoire allouée pour un commit
void freeCommit(Commit *c) {
  for (int i = 0; i < c->size; i++) {
    if (c->T[i] != NULL) {
      freeKeyVal(c->T[i]);
    }
  }
  free(c->T);
  free(c);
}

// crée un commit à partir d'un hash
Commit *createCommit(char *hash) {
  Commit *c = initCommit();
  commitSet(c, "tree", hash);
  return c;
}

// Récupere la valeur associée à une clé dans un commit
char *commitGet(Commit *c, char *key) {
  int p = hash(key) % c->size;
  int attempt = 0;
  while (c->T[p] != NULL && attempt < c->size) {
    if (strcmp(c->T[p]->key, key) == 0) {
      return c->T[p]->value;
    }
    p = (p + 1) % c->size;
    attempt = attempt + 1;
  }
  return NULL;
}

// convertit un commit en chaîne de caractères
char *cts(Commit *c) {
  char *str = malloc(sizeof(char) * 100 * c->n);
  for (int i = 0; i < c->size; i++) {
    if (c->T[i] != NULL) {
      char *tmp = kvts(c->T[i]);
      strcat(str, tmp);
      strcat(str, "\n");
      free(tmp);
    }
  }
  return str;
}

// convertit une chaîne de caractères en commit
Commit *stc(char *ch) {
  int pos = 0;
  int n_pos = 0;
  char sep = '\n';
  char *ptr;
  char *result = malloc(sizeof(char) * 10000);
  Commit *c = initCommit();
  while (pos < strlen(ch)) {
    ptr = strchr(ch + pos, sep);
    if (ptr == NULL)
      n_pos = strlen(ch) + 1;
    else
      n_pos = ptr - ch + 1;
    memcpy(result, ch + pos, n_pos - pos - 1);
    result[n_pos - pos - 1] = '\0';
    pos = n_pos;
    kvp *elem = stkv(result);
    commitSet(c, elem->key, elem->value);
    if(elem!=NULL) freeKeyVal(elem);
  }
  free(result);
  return c;
}

// convertit un commit en fichier
void ctf(Commit *c, char *file) {
  FILE *fp = fopen(file, "w");
  if (fp != NULL) {
    char *cts_var = cts(c);
    fputs(cts_var, fp);
    fclose(fp);
    free(cts_var);
  }
}

// convertit un fichier en commit
Commit *ftc(char *file) {
  char buff[256];
  char *all = malloc(sizeof(char) * 256);
  all[0] = '\0';
  FILE *f = fopen(file, "r");
  if (f == NULL) {
    printf("fichier inexistant %s \n", file);
    return NULL;
  }
  while (fgets(buff, 256, f) != NULL) {
    strcat(all, buff);
  }
  Commit *c = stc(all);
  free(all);
  fclose(f);
  return c;
}

// fais l'enregistrement d'un commit
char *blobCommit(Commit *c) {
  char fname[100] = "/tmp/myfileXXXXXX";
  int fd = mkstemp(fname);
  ctf(c, fname);
  char *hash = sha256file(fname);
  char *ch = hashToFile(hash);
  strcat(ch, ".c");
  cp(ch, fname);
  free(ch);
  return hash;
}

// initialise les références
void initRefs() {
  if (!file_exists(".refs")) {
    system("mkdir .refs");
    system("touch .refs/master ");
    system("touch .refs/HEAD");
  }
}

// crée une référence
void createUpdateRef(char *ref_name, char *hash) {
  char buff[100];
  sprintf(buff, "echo %s > .refs/%s", hash, ref_name);
  system(buff);
}

// suppression d'une référence
void deleteRef(char *ref_name) {
  char buff[256];
  if (!file_exists_rep(ref_name, ".refs/")) {
    printf("La référence %s n'existe pas\n", ref_name);
  } else {
    sprintf(buff, "rm .refs/%s", ref_name);
    system(buff);
  }
}

// récupère le hash d'une référence
char *getRef(char *ref_name) {
  int SIZE = 65;
  FILE *fp;
  char *result = malloc(sizeof(char) * SIZE);

  if (!file_exists_rep(ref_name, ".refs/")) {
    printf("La référence %s n'existe pas\n", ref_name);
    return NULL;
  }
  char buff[256];
  sprintf(buff, ".refs/%s", ref_name);
  fp = fopen(buff, "r");
  if (fp == NULL) {
    printf("Erreur à l'ouverture du fichier  \n");
    return NULL;
  }
  fgets(result, SIZE, fp);
  fclose(fp);
  return result;
}

void myGitAdd(char *file_or_folder) {
  WorkTree *wt;
  if (!file_exists(".add")) {
    system("touch .add");
    wt = initWorkTree();
  } else {
    wt = ftwt(".add");
  }
  if (file_exists(file_or_folder)) {
    appendWorkTree(wt, file_or_folder, NULL, 0);
    wttf(wt, ".add");
  } else {
    printf("Le fichier/repertoire %s n'existe pas\n", file_or_folder);
  }

  freeWorkTree(wt);
}

void myGitCommit(char *branch_name, char *message) {
  if (!file_exists(".refs")) {
    printf("Il faut d’abord initialiser les références du projet\n");
    return;
  }
  if (!file_exists_rep(branch_name, ".refs/")) {
    printf("La branche %s n’ existe pas\n", branch_name);
    return;
  }
  char *last_hash = getRef(branch_name);
  char *head_hash = getRef("HEAD");
  if (strcmp(last_hash, head_hash) != 0) {
    printf("HEAD doit pointer sur le dernier commit de la branche\n");
    return;
  }
  free(head_hash);
  free(last_hash);
  Commit *c;
  if (!file_exists(".add")) {
    c = createCommit("vide");
  } else {
    WorkTree *wt = ftwt(".add");
    char *hashwt = saveWorkTree(wt, ".");
    c = createCommit(hashwt);
    freeWorkTree(wt);
    free(hashwt);
  }
  if (strlen(last_hash) != 0) {
    commitSet(c, "predecessor", last_hash);
  }
  if (message != NULL) {
    commitSet(c, "message", message);
  }
  char *hashc = blobCommit(c);
  createUpdateRef(branch_name, hashc);
  createUpdateRef("HEAD", hashc);
  if (file_exists(".add")) system("rm .add ");
  free(hashc);
  freeCommit(c);
}

// Fonction void initBranch() qui crée le fichier caché .current branch
// contenant la chaîne de caractères "master"
void initBranch() {
  FILE *fp = fopen(".current_branch", "w");
  if (fp != NULL) {
    fputs("master", fp);
    fclose(fp);
  }
}

// Fonction int branchExists(char* branch) qui vérifie l’existence d'une branche
int branchExists(char *branch) {
  List *refs = listdir(".refs");
  Cell *c = searchList(refs, branch);
  libererListe(refs);
  return c != NULL;
}

// Fonction void createBranch(char* branch) qui crée une branche qui pointe vers
// le même commit que HEAD
void createBranch(char *branch) {
  char *hash = getRef("HEAD");
  createUpdateRef(branch, hash);
  free(hash);
}

// Fonction char* getCurrentBranch() qui lit le fichier caché .current branch
// pour retourner le nom de la branche courante
char *getCurrentBranch() {
  FILE *f = fopen(".current_branch", "r");
  char *buff = malloc(sizeof(char) * 100);
  fscanf(f, "%s", buff);
  return buff;
}

void printBranch(char *branch) {
  char *commit_path = malloc(sizeof(char) * 100);
  char *commit_hash = getRef(branch);
  char *path = hashToPath(commit_hash);
  sprintf(commit_path, "%s.c", path);
  Commit *c = ftc(commit_path);
  while (c != NULL) {
    if (commitGet(c, "message") != NULL)
      printf("%s -> %s\n", commit_hash, commitGet(c, "message"));
    else {
      printf("%s\n", commit_hash);
    }
    if (commitGet(c, "predecessor") != NULL) {
      commit_path = NULL;
      commit_hash = commitGet(c, "predecessor");
      sprintf(commit_path, "%s.c", path);
      c = ftc(commit_path);
    } else {
      c = NULL;
      free(commit_path);
    }
  }
  if (c != NULL) freeCommit(c);
  free(path);
}

List *branchList(char *branch) {
  List *l = initList();
  char *commit_hash = getRef(branch);
  char *commit_path = malloc(sizeof(char) * 68);
  char *path = hashToPath(commit_hash);
  sprintf(commit_path, "%s.c", path);
  Commit *c = ftc(commit_path);

  while (c != NULL) {
    insertFirst(l, buildCell(commit_hash));
    if (commitGet(c, "predecessor") != NULL) {
      strcpy(commit_path, "");
      commit_hash = commitGet(c, "predecessor");
      sprintf(commit_path, "%s.c", path);
      c = ftc(commit_path);
    } else {
      c = NULL;
      free(commit_path);
    }
  }
  if (c != NULL) freeCommit(c);
  free(path);
  return l;
}

List *getAllCommits() {
  List *listCommits = initList();
  List *listBranchName = listdir(".refs");

  for (Cell *branchName = *listBranchName; branchName != NULL;
       branchName = branchName->next) {
    if (strcmp(branchName->data, ".") == 0 ||
        strcmp(branchName->data, "..") == 0) {
      continue;
    }
    List *tmpList = branchList(branchName->data);

    Cell *cell = *tmpList;
    while (cell != NULL) {
      if (searchList(listCommits, cell->data) == NULL) {
        insertFirst(listCommits, buildCell(cell->data));
      }
      cell = cell->next;
    }
    libererListe(tmpList);
  }

  libererListe(listBranchName);
  return listCommits;
}

// Permet de restaurer le worktree associé à un commit dont le hash est donné en
// paramètre. Indication : Il faut utiliser la fonction restoreWorkTree
void restoreCommit(char *hash_commit) {
  if (hash_commit == NULL) {
    return;
  }
  char *commit_path = malloc(sizeof(char) * 68);

  char *path1 = hashToPath(hash_commit);
  sprintf(commit_path, "%s.c", path1);
  free(path1);
  Commit *c = ftc(commit_path);
  char *hashwt = commitGet(c, "tree");
  if (strcmp(hashwt, "vide") == 0) {
    if(c!=NULL)freeCommit(c);
    free(commit_path);
    return;
  }
  char *path2 = hashToPath(hashwt);
  sprintf(path2, "%s.t", path2);
  WorkTree *wt = ftwt(path2);
  restoreWorkTree(wt, ".");
  if(c!=NULL) freeCommit(c);
  if(wt!=NULL) freeWorkTree(wt);
  free(commit_path);
  free(path2);
  
}

void myGitCheckoutCommit(char *pattern) {
  // On récupère la liste de tous les commits existants.
  List *listCommits = getAllCommits();
  // On filtre cette liste pour ne garder que ceux qui commencent par pattern.
  List *List = filterlist(listCommits, pattern);
  int nbHash = nbElements(List);

  if (nbHash == 0) {
    // S'il ne reste plus aucun hash après le filtre, la fonction affiche un
    // message d'erreur à l'utilisateur.
    printf("Aucun commit ne correspond à votre requête.\n");
  } else if (nbHash == 1) {
    // S'il ne reste plus qu’un hash après ce filtre, alors on met à jour la
    // référence HEAD pour pointer sur ce hash, et on restaure le worktree
    // correspondant.
    char *hash = (*List)->data;
    createUpdateRef("HEAD", hash);
    restoreCommit(hash);
  } else {
    // S'il reste plusieurs hash après le filtre, la fonction les affiche tous
    // et demande à l'utilisateur de préciser sa requête.
    printf("Plusieurs commits correspondent à votre requête :\n");
    for (Cell *cell = *List; cell != NULL; cell = cell->next) {
      printf("%s\n", cell->data);
    }
    printf("Veuillez préciser votre requête.\n");
  }
  libererListe(listCommits);
  libererListe(List);
}

void myGitCheckoutBranch(char *branch) {
  // On modifie le fichier .current branch pour contenir le nom de la branche
  // donnée en paramètre
  FILE *f = fopen(".current_branch", "w");
  if (f != NULL) {
    fputs(branch, f);
    fclose(f);
  }
  // On modifie la référence HEAD pour contenir le hash du dernier commit de
  // branch (on rappelle que ce hash est contenu dans le fichier branch).
  char *hash = getRef(branch);
  createUpdateRef("HEAD", hash);
  // On restaure le worktree correspondant au dernier commit de branch.
  restoreCommit(hash);
  free(hash);
}

// crée un commit a partir de la référence vers un commit
Commit *htc(char *ref) {
  char *hash = getRef(ref);
  char *path = hashToPath(hash);
  sprintf(path, "%s.c", path);
  Commit *c = ftc(path);
  free(hash);
  free(path);
  return c;
}

// crée un worktree a partir d'un commit
WorkTree *ctwt(Commit *c) {
  char *hashwt = commitGet(c, "tree");
  if (strcmp(hashwt, "vide") == 0) {
    return NULL;
  }
  char *path = hashToPath(hashwt);
  sprintf(path, "%s.t", path);
  WorkTree *wt = ftwt(path);
  free(path);
  return wt;
}

List *merge(char *remote_branch, char *message) {
  if (!file_exists_rep(remote_branch, ".refs/")) {
    printf("La branche %s n'existe pas\n", remote_branch);
    return 0;
  }
  // On récupère le nom de la branche courante
  char *current_branch = getCurrentBranch();
  // On récupère les commits correspondants aux deux hash

  Commit *commit_current_branch = htc(current_branch);
  Commit *commit_remote_branch = htc(remote_branch);

  // On récupère les chemins des fichiers contenant les worktrees correspondant
  // aux deux commits

  // On récupère les worktrees correspondants aux deux commits
  WorkTree *wt_current_branch = ctwt(commit_current_branch);
  WorkTree *wt_remote_branch = ctwt(commit_remote_branch);

  List *list_conflicts = initList();

  // On fusionne les deux worktrees
  WorkTree *mergeWT =
      mergeWorkTrees(wt_current_branch, wt_remote_branch, &list_conflicts);

  if (*list_conflicts == NULL) {
    // créer le commit associé à ce nouveau worktree, en indiquant qui sont
    // ses prédécesseurs, et en lui ajoutant le message descriptif passé en
    // paramètre,
    if (mergeWT == NULL) {
      deleteRef(remote_branch);
      free(current_branch);
      if (commit_current_branch != NULL) freeCommit(commit_current_branch);
      if (commit_remote_branch != NULL) freeCommit(commit_remote_branch);
      if (wt_current_branch != NULL) freeWorkTree(wt_current_branch);
      if (wt_remote_branch != NULL) freeWorkTree(wt_remote_branch);
      if (list_conflicts != NULL) libererListe(list_conflicts);
      return NULL;
    }
    char *hash_mergeWT = blobWorkTree(mergeWT);
    Commit *new_commit = createCommit(hash_mergeWT);
    commitSet(new_commit, "message", message);
    char *hash_current_branch = getRef(current_branch);
    char *hash_remote_branch = getRef(remote_branch);
    commitSet(new_commit, "predecessor", hash_current_branch);
    commitSet(new_commit, "merged_predecessor", hash_remote_branch);
    free(hash_current_branch);
    free(hash_remote_branch);
    free(hash_mergeWT);

    // — réaliser un enregistrement instantané du worktree de fusion et de ce
    // nouveau commit,
    hash_mergeWT = saveWorkTree(mergeWT, ".");
    char *hash_new_commit = blobCommit(new_commit);
    // ajouter le nouveau commit à la branche courante
    // mettre à jour la référence de la branche courante et la référence HEAD
    // pour pointer vers ce nouveau commit,
    createUpdateRef(current_branch, hash_new_commit);
    createUpdateRef("HEAD", hash_new_commit);
    // supprimer la référence de la branche passée en paramètre,
    deleteRef(remote_branch);
    // restaurer le projet correspondant au worktree de fusion.
    restoreWorkTree(mergeWT, ".");
    free(hash_mergeWT);
    free(hash_new_commit);
    if (mergeWT != NULL) freeWorkTree(mergeWT);
    if (new_commit != NULL) freeCommit(new_commit);
    free(current_branch);
    if (list_conflicts != NULL) libererListe(list_conflicts);
    if (commit_current_branch != NULL) freeCommit(commit_current_branch);
    // if (commit_remote_branch != NULL) freeCommit(commit_remote_branch);
    // if (wt_current_branch != NULL) freeWorkTree(wt_current_branch);
    // if (wt_remote_branch != NULL) freeWorkTree(wt_remote_branch);

    return NULL;
  }
  free(current_branch);
  if (mergeWT != NULL) freeWorkTree(mergeWT);
  if (commit_current_branch != NULL) freeCommit(commit_current_branch);
  if (commit_remote_branch != NULL) freeCommit(commit_remote_branch);
  if (wt_current_branch != NULL) freeWorkTree(wt_current_branch);
  if (wt_remote_branch != NULL) freeWorkTree(wt_remote_branch);
  return list_conflicts;
}

void createDeletionCommit(char *branch, List *conflicts, char *message) {
  // On récupère le nom de la branche courante
  char *start_branch = getCurrentBranch();
  // On se déplace sur la branche en question
  myGitCheckoutBranch(branch);
  // On récupère le dernier commit de cette branche, et le worktree associé
  Commit *commit_current_branch = htc(branch);
  WorkTree *wt_current_branch = ctwt(commit_current_branch);
  // On vide la zone de préparation
  if (file_exists(".add")) system("rm .add");

  // On ajoute les fichiers/répertoires du worktree qui ne font pas partie de la
  // liste des conflits
  for (int i = 0; i < wt_current_branch->n; i++) {
    if (searchList(conflicts, wt_current_branch->tab[i].name) == NULL) {
      myGitAdd(wt_current_branch->tab[i].name);
    }
  }
  // On appelle la fonction myGitCommit pour créer le commit de suppression
  myGitCommit(branch, message);
  // On revient sur la branche de départ
  myGitCheckoutBranch(start_branch);

  // libération de la mémoire :
  freeWorkTree(wt_current_branch);
  free(start_branch);
  freeCommit(commit_current_branch);
}