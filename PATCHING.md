# Patching Ring for MRE

## 1. `src/ring/include/rconfig.h`

Add the microcontroller define.

```diff
 /* Custom Configuration File (Could be modified when embedding Ring in other projects) */
+
+#define RING_MICROCONTROLLER 1
```

## 2. `src/ring/include/ext.h`

- Add `RING_VM_MRE 1`
- Set `RING_VM_REFMETA` and `RING_VM_INFO` to `0`
- Change `#if RING_NODLL` to `#if RING_LOWMEM`

```diff
 #ifndef ringext_h
 #define ringext_h
+#define RING_VM_MRE 1
 #define RING_VM_LISTFUNCS 1
 #define RING_VM_MATH 1
 #define RING_VM_FILE 1
 #define RING_VM_OS 1
-#define RING_VM_REFMETA 1
-#define RING_VM_INFO 1
-#if RING_NODLL
+#define RING_VM_REFMETA 0
+#define RING_VM_INFO 0
+#if RING_LOWMEM
 	#define RING_VM_DLL 0
 #else
 	#define RING_VM_DLL 1
```

## 3. `src/ring/include/ring.h`

Add the MRE extension header include after the `dll_e.h` block.

```diff
 	#if RING_VM_DLL
 		#include "dll_e.h"
 	#endif
+	#if RING_VM_MRE
+		#include "mre_e.h"
+	#endif
 #endif
 #endif
```

## 4. `src/ring/src/ext.c`

- Add the `ring_vm_mre_loadfunctions` call at the top of `ring_vm_extension`
- Delete the `#if RING_VM_DLL / ring_vm_dll_loadfunctions` block

```diff
 void ring_vm_extension(RingState *pRingState) {
+#if RING_VM_MRE
+	ring_vm_mre_loadfunctions(pRingState);
+#endif
 /* List */
 #if RING_VM_LISTFUNCS
 	ring_vm_list_loadfunctions(pRingState);
```

```diff
 #if RING_VM_OS
 	ring_vm_os_loadfunctions(pRingState);
 #endif
-/* DLL */
-#if RING_VM_DLL
-	ring_vm_dll_loadfunctions(pRingState);
-#endif
 /* Reflection and Meta-programming */
 #if RING_VM_REFMETA
 	ring_vm_refmeta_loadfunctions(pRingState);
```

## 5. `src/ring/src/file_e.c`

Comment out the `tempfile`, `rename`, `remove` registrations:

```diff
 	RING_API_REGISTER("freopen", ring_vm_file_freopen);
-	RING_API_REGISTER("tempfile", ring_vm_file_tempfile);
+	// RING_API_REGISTER("tempfile", ring_vm_file_tempfile);
 	RING_API_REGISTER("fseek", ring_vm_file_fseek);
```

```diff
 	RING_API_REGISTER("perror", ring_vm_file_perror);
-	RING_API_REGISTER("rename", ring_vm_file_rename);
-	RING_API_REGISTER("remove", ring_vm_file_remove);
+	// RING_API_REGISTER("rename", ring_vm_file_rename);
+	// RING_API_REGISTER("remove", ring_vm_file_remove);
 	RING_API_REGISTER("fgetc", ring_vm_file_fgetc);
```

Comment out the function bodies:

```diff
+/*
 void ring_vm_file_tempfile(void *pPointer) {
 	FILE *pFile;
 	pFile = tmpfile();
 	RING_API_RETMANAGEDCPOINTER(pFile, RING_VM_POINTER_FILE, ring_vm_file_freefunc);
 }
+*/
```

```diff
+/*
 void ring_vm_file_rename(void *pPointer) {
 	...
 }
+*/
+/*
 void ring_vm_file_remove(void *pPointer) {
 	...
 }
+*/
```

## 6. `src/ring/src/genlib_e.c`

Comment out the `ring_state_*` block by removing the closing `*/` from `/* Ring State */` and adding `*/` after `ring_state_resume`:

```diff
-	/* Ring State */
+	/* Ring State
 	RING_API_REGISTER("ring_state_init", ring_vm_generallib_state_init);
 	RING_API_REGISTER("ring_state_runcode", ring_vm_generallib_state_runcode);
 	RING_API_REGISTER("ring_state_delete", ring_vm_generallib_state_delete);
 	RING_API_REGISTER("ring_state_runfile", ring_vm_generallib_state_runfile);
 	RING_API_REGISTER("ring_state_findvar", ring_vm_generallib_state_findvar);
 	RING_API_REGISTER("ring_state_newvar", ring_vm_generallib_state_newvar);
 	RING_API_REGISTER("ring_state_runobjectfile", ring_vm_generallib_state_runobjectfile);
 	RING_API_REGISTER("ring_state_main", ring_vm_generallib_state_main);
 	RING_API_REGISTER("ring_state_setvar", ring_vm_generallib_state_setvar);
 	RING_API_REGISTER("ring_state_new", ring_vm_generallib_state_new);
 	RING_API_REGISTER("ring_state_mainfile", ring_vm_generallib_state_mainfile);
 	RING_API_REGISTER("ring_state_filetokens", ring_vm_generallib_state_filetokens);
 	RING_API_REGISTER("ring_state_stringtokens", ring_vm_generallib_state_stringtokens);
 	RING_API_REGISTER("ring_state_scannererror", ring_vm_generallib_state_scannererror);
 	RING_API_REGISTER("ring_state_runcodeatins", ring_vm_generallib_state_runcodeatins);
 	RING_API_REGISTER("ring_state_resume", ring_vm_generallib_state_resume);
+	*/
```

Comment out the Date/Time registrations:

```diff
 	/* Date and Time */
-	RING_API_REGISTER("clock", ring_vm_generallib_clock);
-	RING_API_REGISTER("clockspersecond", ring_vm_generallib_clockspersecond);
-	RING_API_REGISTER("time", ring_vm_generallib_time);
-	RING_API_REGISTER("timelist", ring_vm_generallib_timelist);
-	RING_API_REGISTER("date", ring_vm_generallib_date);
-	RING_API_REGISTER("adddays", ring_vm_generallib_adddays);
-	RING_API_REGISTER("diffdays", ring_vm_generallib_diffdays);
+	// RING_API_REGISTER("clock", ring_vm_generallib_clock);
+	// RING_API_REGISTER("clockspersecond", ring_vm_generallib_clockspersecond);
+	// RING_API_REGISTER("time", ring_vm_generallib_time);
+	// RING_API_REGISTER("timelist", ring_vm_generallib_timelist);
+	// RING_API_REGISTER("date", ring_vm_generallib_date);
+	// RING_API_REGISTER("adddays", ring_vm_generallib_adddays);
+	// RING_API_REGISTER("diffdays", ring_vm_generallib_diffdays);
```

## 7. Delete non-MRE source files

```bash
rm src/ring/src/dll_e.c
rm src/ring/src/meta_e.c
rm src/ring/src/vminfo_e.c
rm src/ring/src/ring.c
rm src/ring/src/ringw.c
```

## Files we own (not in upstream)

- `src/ring/src/mre_e.c`
- `src/ring/include/mre_e.h`
- `src/ring/include/rconfig.h`

Restore these after copying upstream files.