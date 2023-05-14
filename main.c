#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hash.h"
#include "list.h"
#include "work.h"

int main() {
  // hashFile("test.txt", "testHash.txt");
  // printf("%s\n", sha256file("test.txt"));
  // blobFile("test.txt");
  // WorkTree* wt = initWorkTree();
  // int i = appendWorkTree(wt, "test.txt", sha256file("test.txt"), 0);
  // blobWorkTree(wt);

  // char* s = saveWorkTree(
  //     wt,
  //     "e9/8b31830667df2118c35ded85ee565227350d3d3686dba5813582026c9deef0.t");
  // WorkTree* wt2 = ftwt(
  //     "e9/8b31830667df2118c35ded85ee565227350d3d3686dba5813582026c9deef0.t");
  // printf("%s\n",wtts(wt2));
  // restoreWorkTree(wt2, ".");

  /* TESTS */
  // Exo 1
  char *hash = sha256file("main.c");
  printf("Hash de main.c : ");
  printf("%s\n", hash);
  free(hash);

  // Exo 2
  List *L = initList();
  Cell *c1 = buildCell("chaine1");
  Cell *c2 = buildCell("chaine2");
  Cell *c3 = buildCell("chaine3");
  insertFirst(L, c3);
  insertFirst(L, c2);
  insertFirst(L, c1);
  assert(strcmp(ctos(c1), "chaine1") == 0);
  assert(strcmp(ctos(c2), "chaine2") == 0);
  assert(strcmp(ctos(c3), "chaine3") == 0);
  char *resL = ltos(L);
  assert(strcmp(resL, "chaine1|chaine2|chaine3|") == 0);
  Cell *c = listGet(L, 1);
  assert(strcmp(ctos(c), "chaine2") == 0);
  Cell *c4 = searchList(L, "chaine3");
  assert(strcmp(ctos(c4), "chaine3") == 0);
  ltof(L, "testMain.txt");
  List *L2 = stol("chaine1|chaine2|chaine3");
  char *resL2 = ltos(L2);
  assert(strcmp(resL2, "chaine3|chaine2|chaine1|") == 0);
  List *L3 = ftol("testMain.txt");
  char *resL3 = ltos(L3);
  assert(strcmp(resL3, "chaine3|chaine2|chaine1|") == 0);
  free(resL);
  free(resL2);
  free(resL3);
  libererListe(L);
  libererListe(L2);
  libererListe(L3);

  // Exo 3
  assert(file_exists("main.c"));
  assert(!file_exists("azerty"));
  // assert(strcmp(hashToPath("01abcdef"), "01/abcdef") == 0);
  List *l = listdir(".");
  printf("\nFichiers dans le répertoire courant : \n");
  printf("%s\n", ltos(l));
  cp("testMainCopy.txt", "testMain.txt");
  blobFile("testMainCopy.txt");
  libererListe(l);

  // Exo 4
  WorkTree *wt = initWorkTree();
  assert(inWorkTree(wt, "main.c") == -1);
  if (appendWorkTree(wt, "main.c", "0123abcdef", 777) == 0) {
    printf("\nmain.c ajouté au WorkTree\n");
  } else {
    printf("Erreur lors de l'ajout de main.c au WorkTree\n");
  }
  assert(inWorkTree(wt, "main.c") == 0);
  appendWorkTree(wt, "fichier2.txt", "hash2", 644);
  appendWorkTree(wt, "dossier1.txt", NULL, 700);
  char *wts = wtts(wt);
  printf("\n%s\n", wts);
  free(wts);
  freeWorkTree(wt);
  return 0;
}


// alpha 