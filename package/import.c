

#include <stdlib.h>
#include "../deps/hash/hash.h"

#include <stdbool.h>


#include "package.h"
#include "export.h"


typedef struct {
  char      * alias;
  char      * filename;
  bool        c_file;
  package_t * pkg;
} package_import_t;

package_import_t * package_import_add(char * alias, char * filename, package_t * parent, char ** error) {
  package_import_t * imp = malloc(sizeof(package_import_t));

  hash_set(parent->deps, alias, imp);

  imp->alias    = alias;
  imp->filename = filename;
  imp->c_file   = false;
  imp->pkg      = package_new(filename, error);

  if (imp->pkg == NULL) return NULL;

  package_export_write_headers(imp->pkg);
  return imp;
}

package_import_t * package_import_add_c_file(package_t * parent, char * filename, char ** error) {
  char * alias   = realpath(filename, NULL);

  package_import_t * imp = malloc(sizeof(package_import_t));

  hash_set(parent->deps, alias, imp);

  imp->alias    = alias;
  imp->filename = filename;
  imp->c_file   = true;
  imp->pkg      = package_c_file(alias, error);

  return imp;
}