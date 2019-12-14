#ifdef __cplusplus
extern "C" {
#endif

typedef struct haiku_nm_t *haiku_nm_h;
haiku_nm_h haiku_init_nm();
void haiku_close_nm(haiku_nm_h hnd);
int haiku_watch_dir(haiku_nm_h hnd, const char *path);
int haiku_stop_watch(haiku_nm_h hnd);
int haiku_is_update_needed(haiku_nm_h hnd);

#ifdef __cplusplus
}
#endif
