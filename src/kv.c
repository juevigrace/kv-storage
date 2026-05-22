#include "../inc/kv.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

kv_t *kv_init(size_t capacity) {
  if (capacity == 0)
    return NULL;

  kv_t *table = malloc(sizeof(kv_t));
  if (table == NULL) {
    return NULL;
  }

  table->capacity = capacity;
  table->count = 0;

  table->entries = calloc(sizeof(kv_entry_t), capacity);
  if (table->entries == NULL) {
    return NULL;
  }

  return table;
}

size_t hash(const char *val, int capacity) {
  size_t hash = 0x13371337deadbeef;
  while (*val) {
    hash ^= *val;
    hash = hash << 8;
    hash += *val;
    val++;
  }
  return hash % capacity;
}

// fn kv_put
// params:
//  - db: a pointer to the db
//  - key: a pointer to the key value
//  - value: a pointer to the value itself
// returns: the index of the key, otherwise on
// error, returns -1, on not found returns -2
int kv_put(kv_t *db, char *key, char *value) {
  if (!db || !key || !value) {
    return -1;
  }

  size_t idx = hash(key, db->capacity);

  for (int i = 0; i < (int)db->capacity - 1; i++) {
    size_t real_idx = (idx + i) % db->capacity;
    kv_entry_t *e = &db->entries[real_idx];

    // the key is already set, updating
    if (e->key && e->key != TOMBSTONE && !strcmp(e->key, key)) {
      char *newval = strdup(value);
      if (!newval) {
        return -1;
      }
      free(e->value);
      e->value = newval;
      return 0;
    }

    // land in a slot that is "empty"
    // null or tombstone
    if (!e->key || e->key == TOMBSTONE) {
      char *newval = strdup(value);
      char *newkey = strdup(key);
      if (!newval || !newkey) {
        free(newkey);
        free(newval);
        return -1;
      }
      e->value = newval;
      e->key = newkey;
      db->count++;
      return 0;
    }
  }

  return -2;
}

// fn kv_get
// params:
//  - db: a pointer to the db
//  - key: a pointer to the key value
// returns: the pointer to the key
// NULL if not found
char *kv_get(kv_t *db, char *key) {
  if (!db || !key) {
    return NULL;
  }

  size_t idx = hash(key, db->capacity);

  for (int i = 0; i < (int)db->capacity - 1; i++) {
    size_t real_idx = (idx + i) % db->capacity;
    kv_entry_t *e = &db->entries[real_idx];

    // there is no key, therefore return nothing
    if (e->key == NULL) {
      return NULL;
    }

    // find an entry and the keys match
    if (e->key && e->key != TOMBSTONE && !strcmp(e->key, key)) {
      return e->value;
    }
  }

  return NULL;
}

// fn kv_delete
// params:
//  - db: a pointer to the db
//  - key: a pointer to the key value
// returns: the index of the deletion
// -1 if not found
int kv_delete(kv_t *db, char *key) {
  if (!db || !key) {
    return -1;
  }

  size_t idx = hash(key, db->capacity);

  for (int i = 0; i < (int)db->capacity - 1; i++) {
    size_t real_idx = (idx + i) % db->capacity;
    kv_entry_t *e = &db->entries[real_idx];

    // there is no key, therefore return nothing
    if (e->key == NULL) {
      return -1;
    }

    if (e->key && e->key != TOMBSTONE && !strcmp(e->key, key)) {
      free(e->key);
      free(e->value);
      db->count--;
      e->key = TOMBSTONE;
      e->value = NULL;
      return real_idx;
    }
  }

  return -1;
}

// fn kv_free
// params:
//  - db: a pointer to the db
// returns: nothing
void kv_free(kv_t *db) {
  if (!db)
    return;
  for (int i = 0; i < (int)db->capacity - 1; i++) {
    kv_entry_t *e = &db->entries[i];
    if (e->key && e->key != TOMBSTONE) {
      free(e->key);
      free(e->value);
      e->key = NULL;
      e->value = NULL;
      db->count--;
    }
  }

  free(db->entries);
  free(db);

  return;
}
