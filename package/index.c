#define _GNU_SOURCE

#include "../deps/hash/hash.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <libgen.h>

#include "package.h"
#include "../parser/grammer.h"
#include "../parser/parser.h"
#include "import.h"
#include "export.h"
#include "../deps/stream/stream.h"
#include "atomic-stream.h"
#include "../deps/stream/file.h"

static void init_cache() {
  package_path_cache = hash_new();
  package_id_cache   = hash_new();
}

static char * abs_path(const char * rel_path) {
  return realpath(rel_path, NULL);
}

static char * package_name(const char * rel_path) {
  char * buffer   = strdup(rel_path);
  char * filename = basename(buffer);

  filename[strlen(filename) - 2] = 0; // trim the .c extension

  char * c;
  for (c = filename; *c != 0; c++){
    switch(*c) {
      case '.':
      case '-':
      case ' ':
        *c = '_';
        break;
      default:
        break;
    }
  }

  char * name = strdup(filename);
  free(buffer);
  return name;
}

static bool assert_name(const char * relative_path, char ** error) {
  ssize_t len      = strlen(relative_path);
  size_t  suffix_l = strlen(".module.c");

  if (len < suffix_l || strcmp(".module.c", relative_path + len - suffix_l) != 0) {
    if (error) asprintf(error, "Unsupported input filename '%s', Expecting '<file>.module.c'", relative_path);
    return true;
  }
  return false;
}

char * index_generated_name(const char * path) {
  ssize_t len      = strlen(path);
  size_t  suffix_l = strlen(".module.c");
  char * buffer    = malloc(len - suffix_l + strlen(".c") + 1);
  strncpy(buffer, path, len - suffix_l);
  strcpy(buffer + (len - suffix_l), ".c");
  return buffer;
}

package_t * index_new(const char * relative_path, char ** error);

package_t * index_parse(
    stream_t * input,
    stream_t * out,
    const char * rel,
    char * key,
    char * generated,
    char ** error
) {
  if (package_path_cache == NULL) init_cache();
  if (package_new        == NULL) package_new = index_new;

  package_t * p = calloc(1, sizeof(package_t));
  p->deps       = hash_new();
  p->exports    = hash_new();
  p->ordered    = NULL;
  p->n_exports  = 0;
  p->symbols    = hash_new();
  p->source_rel = strdup(rel);
  p->source_abs = key;
  p->generated  = generated;
  p->out        = out;
  p->name       = package_name(p->generated);


  hash_set(package_path_cache, key, p);

  if (grammer_parse(input, rel, p, error) != 0) return NULL;
  return p;
}

package_t * index_new(const char * relative_path, char ** error) {
  if (package_path_cache == NULL) init_cache();
  if (package_new        == NULL) package_new = index_new;

  if (assert_name(relative_path, error)) return NULL;
  char * key = abs_path(relative_path);

  if (key == NULL) {
    *error = strerror(errno);
    return NULL;
  }

  package_t * cached = hash_get(package_path_cache, key);
  if (cached != NULL) return cached;

  stream_t * input = file_open(relative_path, O_RDONLY);
  if (input->error.code != 0) {
    *error = strdup(input->error.message);
    return NULL;
  }

  char * generated = index_generated_name(key);

  stream_t * out = atomic_stream_open(generated);
  if (out->error.code != 0) {
    if (error) *error = strdup(out->error.message);
    return NULL;
  }

  package_t * p = index_parse(input, out, relative_path, key, generated, error);

  if (p == NULL || *error != NULL) {
    atomic_stream_abort(out);
    return NULL;
  }

  stream_close(out);
  return p;
}

void index_debug_print(package_t * pkg) {
  printf("Imports:\n");
  hash_each(pkg->deps, {
      package_import_t * imp = (package_import_t *) val;
      printf("    import %s %s\n", key, imp->filename);
  })
  printf("\n");

  printf("Exports:\n");
  hash_each(pkg->exports, {
      package_export_t * exp = (package_export_t *) val;
      printf("    export %s %s\n", key, exp->declaration);
  })
  printf("\n");
}