#ifndef HAIKU_NM_INTEROP_H
#define HAIKU_NM_INTEROP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct haiku_nm_t *haiku_nm_h;

/**
 * Initialize the Haiku NM (Node Monitor) handler.
 * @return Handle to the NM instance, or NULL on failure.
 */
haiku_nm_h haiku_init_nm();

/**
 * Close and free the NM handler.
 * @param hnd The NM handle to close.
 */
void haiku_close_nm(haiku_nm_h hnd);

/**
 * Start watching a directory for changes.
 * @param hnd The NM handle.
 * @param path The directory path to watch.
 * @return 0 on success, -1 on error.
 */
int haiku_watch_dir(haiku_nm_h hnd, const char *path);

/**
 * Stop watching the current directory.
 * @param hnd The NM handle.
 * @return 0 on success, -1 on error.
 */
int haiku_stop_watch(haiku_nm_h hnd);

/**
 * Check if an update is needed (directory changed).
 * @param hnd The NM handle.
 * @return 1 if update needed, 0 if not, -1 on error.
 */
int haiku_is_update_needed(haiku_nm_h hnd);

#ifdef __cplusplus
}
#endif

#endif /* HAIKU_NM_INTEROP_H */
