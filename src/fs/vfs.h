#pragma once
#include "klib/kstring.h"
#include "req.h"
#include <stdint.h>

void list_files(kstring* path);
// TODO: return some descriptor
void *read_file(kstring *path, u64 *out_size);
kstring *vfs_split_path(const kstring *path);
bool valid_path(kstring *path);
