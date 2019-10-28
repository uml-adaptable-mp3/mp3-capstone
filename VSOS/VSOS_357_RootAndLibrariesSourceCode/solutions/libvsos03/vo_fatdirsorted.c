#include <vo_stdio.h>
#include <vo_fatdirops.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sysmemory.h>

auto s_int16 ResetFindFileSorted(register FILE *fp, 
				 register struct SortedFiles *sortedFiles,
				 register s_int16 *sortCache,
				 register u_int16 sortCacheSize,
				 register u_int16 flags) {
  sortedFiles->firstIdx = -0x8000;
  sortedFiles->currIdx = 0;
  sortedFiles->n = sortCacheSize;
  sortedFiles->index = sortCache;
  return ResetFindFile(fp);
}

struct SortedNames {
  s_int16 index;
  char *longName;
};

int NameCompare(const void *c1, const void *c2) {
  struct SortedNames *sn1 = c1, *sn2 = c2;
  return strcasecmp(sn1->longName, sn2->longName);
}

auto s_int16 FindFileSorted(register FILE *fp,
			    register struct SortedFiles *sf,
			    register char *name, register s_int16 nameSize,
			    const char *suffixes,
			    register enum EntryType entryType) {
  int i;
  struct SortedNames *sortedNames = NULL;
  int err = 0;
  int sfn = sf->n;

  if (++sf->currIdx >= sf->firstIdx+sfn) {
    struct SortedNames *sn;
    int n = -1;
    int lastUsedIdx = -1;
    if (!(sortedNames = calloc(sfn+1, sizeof(struct SortedNames)))) {
      err = 1;
      goto finally;
    }
#if 0
    fprintf(stderr, "sortedNames = 0x%04x\n", sortedNames);
#endif

    if (sf->firstIdx <= 0) {
      sf->firstIdx = 1;
    } else {
      lastUsedIdx = sf->index[sfn-1];
      sf->firstIdx += sfn;
    }

    memset(sf->index, -1, sfn);

    sn = sortedNames;
    i=0;
    while (lastUsedIdx < 0 ||
	   FindFile(fp, name, nameSize, suffixes, lastUsedIdx, entryType)>= 0) {
      char *longName = "";
      int longLen = 0;
      if (lastUsedIdx >= 0) {
	longName = name+strlen(name)+1;
	longLen = strlen(longName);
      }
      sn = NULL;
      /* Is either master element or after master element? */
      if (n < 0 || strcasecmp(longName, sortedNames[0].longName) > 0) {
	if (n < sfn) {
	  /* List is so short we add the new entry any case */
	  sn = sortedNames + ++n;
	} else if (strcasecmp(longName, sortedNames[n].longName) < 0) {
	  /* Is earlier than last element in list. */
	  sn = sortedNames+n;
	}
      }
      /* There was a new element to add */
      if (sn) {
	if (!(sn->longName = realloc(sn->longName, longLen+1))) {
	  err = 1;
	  goto finally;
	}
	strcpy(sn->longName, longName);
	sn->index = i;
	qsort(sortedNames+1, n, sizeof(struct SortedNames), NameCompare);
      }
      lastUsedIdx = ++i;
    }

    /* Copy the indexes we've got */
    {
      s_int16 *idx = sf->index;
      sn = sortedNames+1;
      for (i=0; i<sfn; i++) {
	*idx++ = sn->index;
	sn++;
      }
    }
  }
 finally:
  if (sortedNames) {
    struct SortedNames *sn = sortedNames;
    for (i=0; i<sfn+1; i++) {
      if (sn->longName)
	free(sn->longName);
      sn++;
    }
    free(sortedNames);
  }
  if (!err) {
    s_int16 fileIdx = sf->index[sf->currIdx-sf->firstIdx];
    if (fileIdx > 0) {
      ResetFindFile(fp);
      return FindFile(fp, name, nameSize, suffixes, fileIdx, entryType);
    }
  }
  return -1;
}
