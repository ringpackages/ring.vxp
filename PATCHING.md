# Patching Ring for MRE

This document describes all modifications needed to build Ring on MRE (MediaTek Runtime Environment) devices like Nokia 225.

## Architecture

MRE configuration is split into two layers:

1. **`rconfig.h`** — All preprocessor flag overrides. Ring ships this file empty, so it's never touched by upstream updates. This is where most MRE customization lives.
2. **Small `#if` patches** in 4 upstream files — Minimal guards around code that can't be disabled via flags alone.

The only compiler define needed is `-D RING_VM_MRE=1` (in the Makefile). Everything else is derived from it inside `rconfig.h`.

## Quick Start

```bash
# 1. Copy new Ring source files (overwrites everything except rconfig.h)
cp ringsrc/language/src/*.c src/ring/src/
cp ringsrc/language/include/*.h src/ring/include/

# 2. Restore rconfig.h (step 1 overwrites it with upstream's empty version)
git checkout src/ring/include/rconfig.h

# 3. Re-apply patches 2–5 below

# 4. Delete non-MRE source files (patch 6)
rm src/ring/src/{dll_e.c,meta_e.c,vminfo_e.c,os_e.c,ring.c,ringw.c}

# 5. Build
make clean && make
```

## Patches

### 1. `src/ring/include/rconfig.h` — MRE overrides (never needs re-patching)

This file is the central MRE configuration. Upstream ships it empty, so it's never overwritten during updates — just restore it with `git checkout` after copying upstream files.

```c
/* Custom Configuration File (Could be modified when embedding Ring in other projects) */

/*
 * MRE (MediaTek Runtime Environment) overrides
 * These are defined here instead of ring.h so updating Ring doesn't lose them.
 * rconfig.h is included before all other checks in ring.h, so these take effect
 * via the existing #ifndef / #if guards.
 */
#ifdef RING_VM_MRE
	#define RING_LIMITEDENV 1
	#define RING_LIMITEDSYS 1
	#define RING_VM_OS 0
	#define RING_VM_REFMETA 0
	#define RING_VM_INFO 0
#endif
```

**What each flag does:**
- `RING_LIMITEDENV` — Enables `RING_LOWMEM`, `RING_NODLL`, disables extras
- `RING_LIMITEDSYS` — Disables `getcwd`/`chdir` (POSIX, unavailable on bare-metal)
- `RING_VM_OS` — Disables `os_e.c` (POSIX shell, env vars, process control)
- `RING_VM_REFMETA` — Disables `meta_e.c` (reflection, too much RAM)
- `RING_VM_INFO` — Disables `vminfo_e.c` (VM info functions)

### 2. `src/ring/include/ext.h` — Use `#ifndef` guards

Change 3 unconditional `#define` lines to `#ifndef` so `rconfig.h` values take effect:

```diff
-#define RING_VM_OS 1
-#define RING_VM_REFMETA 1
-#define RING_VM_INFO 1
+#ifndef RING_VM_OS
+	#define RING_VM_OS 1
+#endif
+#ifndef RING_VM_REFMETA
+	#define RING_VM_REFMETA 1
+#endif
+#ifndef RING_VM_INFO
+	#define RING_VM_INFO 1
+#endif
```

**Why:** Without `#ifndef`, `ext.h` unconditionally sets these to `1`, overriding `rconfig.h`.

### 3. `src/ring/include/ring.h` — Include MRE extension header

At the end of the includes section (after the `#include "dll_e.h"` block), add:

```c
	#if RING_VM_MRE
		#include "mre_e.h"
	#endif
```

**Why:** Loads MRE extension function declarations. Can't go in `rconfig.h` since it needs to be inside the header inclusion block.

### 4. `src/ring/src/ext.c` — Load MRE functions

Inside `ring_vm_extension()`, add at the top (after the opening `{`):

```c
#if RING_VM_MRE
	ring_vm_mre_loadfunctions(pRingState);
#endif
```

**Why:** Registers MRE-specific functions (graphics, input, file I/O, etc.).

### 5. `src/ring/src/genlib_e.c` — Disable unsupported functions

Wrap these sections in `ring_vm_generallib_loadfunctions()` with `#if !RING_VM_MRE` / `#endif`:

**eval** (compiles + runs code at runtime — too heavy for MRE RAM):
```c
#if !RING_VM_MRE
	RING_API_REGISTER("eval", ring_vm_generallib_eval);
#endif
```

**raise** and **prevfilename** (no POSIX signals or multi-file support):
```c
#if !RING_VM_MRE
	RING_API_REGISTER("raise", ring_vm_generallib_raise);
#endif
	RING_API_REGISTER("assert", ring_vm_generallib_assert);
	RING_API_REGISTER("filename", ring_vm_generallib_filename);
#if !RING_VM_MRE
	RING_API_REGISTER("prevfilename", ring_vm_generallib_prevfilename);
#endif
```

**ring_state_\*** (sub-states not practical on MRE due to RAM) — wrap the entire block:
```c
#if !RING_VM_MRE
	RING_API_REGISTER("ring_state_init", ring_vm_generallib_state_init);
	... (all 16 ring_state_* lines) ...
	RING_API_REGISTER("ring_state_resume", ring_vm_generallib_state_resume);
#endif
```

**Date/Time** (MRE has no `clock()`, `time()`, `mktime()`, etc.):
```c
#if !RING_VM_MRE
	RING_API_REGISTER("clock", ring_vm_generallib_clock);
	... (all 7 date/time lines) ...
	RING_API_REGISTER("diffdays", ring_vm_generallib_diffdays);
#endif
```

### 6. `src/ring/src/file_e.c` — Disable unsupported file ops

Wrap with `#if !RING_VM_MRE` / `#endif`:

```c
#if !RING_VM_MRE
	RING_API_REGISTER("tempfile", ring_vm_file_tempfile);
#endif
```

```c
#if !RING_VM_MRE
	RING_API_REGISTER("rename", ring_vm_file_rename);
	RING_API_REGISTER("remove", ring_vm_file_remove);
#endif
```

**Why:** `tmpfile()`, `rename()`, `remove()` don't exist on MRE bare-metal.

### 7. Delete non-MRE source files

Delete these from `src/ring/src/` (they're in upstream but not needed for MRE):
- `dll_e.c` — DLL loading (no shared libraries on MRE)
- `meta_e.c` — Reflection (disabled via `RING_VM_REFMETA 0`)
- `vminfo_e.c` — VM info (disabled via `RING_VM_INFO 0`)
- `os_e.c` — OS functions (disabled via `RING_VM_OS 0`)
- `ring.c` — Standalone main() entry point
- `ringw.c` — Windows main() entry point

## Files we own (never in upstream)

These files are MRE-specific and are never overwritten by upstream updates:
- `src/ring/src/mre_e.c` — MRE extension implementation
- `src/ring/include/mre_e.h` — MRE extension header
- `src/ring/include/rconfig.h` — MRE configuration overrides

## When upgrading Ring

```bash
# 1. Copy upstream files
cp ringsrc/language/src/*.c src/ring/src/
cp ringsrc/language/include/*.h src/ring/include/

# 2. Restore our files (overwritten by step 1)
git checkout src/ring/include/rconfig.h

# 3. Re-apply patches 2–6

# 4. Delete non-MRE files
rm src/ring/src/{dll_e.c,meta_e.c,vminfo_e.c,os_e.c,ring.c,ringw.c}

# 5. Build and fix any new issues
make clean && make
```

All patches are `#if`/`#ifndef` wraps around existing lines — no deletions or rewrites. New linker errors after an upgrade mean a new function was added that needs a guard.

## Summary of changes vs upstream

| File | Change | Survives update? |
|---|---|---|
| `rconfig.h` | All MRE flag overrides | ✅ `git checkout` after copy |
| `ext.h` | `#define` → `#ifndef` (3 macros) | ⚠️ Re-apply |
| `ring.h` | `#include "mre_e.h"` (3 lines) | ⚠️ Re-apply |
| `ext.c` | MRE load call (3 lines) | ⚠️ Re-apply |
| `file_e.c` | `#if !RING_VM_MRE` wraps (3 functions) | ⚠️ Re-apply |
| `genlib_e.c` | `#if !RING_VM_MRE` wraps (4 sections) | ⚠️ Re-apply |
| `mre_e.c` / `mre_e.h` | MRE-only files | ✅ Never in upstream |