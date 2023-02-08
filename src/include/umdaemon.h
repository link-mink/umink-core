/*
 *               _____  ____ __
 *   __ ____ _  /  _/ |/ / //_/
 *  / // /  ' \_/ //    / ,<
 *  \_,_/_/_/_/___/_/|_/_/|_|
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef UMDAEMON_H
#define UMDAEMON_H

#include <stdint.h>
#include <stdbool.h>
#include <umcounters.h>

/** Max daemon id/type size */
#define UMD_ID_MAX_SZ 30

/** UMD log level */
enum umd_log_level_t
{
    /** Error */
    UMD_LLT_ERROR = 3,
    /** Warning */
    UMD_LLT_WARNING = 4,
    /** Info */
    UMD_LLT_INFO = 6,
    /** Debug */
    UMD_LLT_DEBUG = 7
};

// UMD type
typedef struct umdaemon umdaemon_t;
/**
 * UMD on_terminate handler
 *
 * @param[in]   d   Daemon
 */
typedef void (*umd_on_terminate_t)(umdaemon_t *d);
/**
 *  UMD on_init handler
 *
 *  @param[in]  d   Daemon
 */
typedef void (*umd_on_init_t)(umdaemon_t *d);
/**
 *  UMD process arguments handler
 *
 *  @param[in]  d   Daemon
 *  @param[in]  c   Argument count
 *  @param[in]  a   Arguments
 */
typedef void (*umd_proc_args_t)(umdaemon_t *d, int c, char **a);

/** Daemon descriptor */
struct umdaemon {
    /** UMD termination flag */
    uint8_t is_terminated;
    /** UMD id */
    char id[UMD_ID_MAX_SZ];
    /** UMD type */
    char type[UMD_ID_MAX_SZ];
    /** UMD full name */
    char *fqn;
    /** UMD log level */
    enum umd_log_level_t log_level;
    /** on_terminate handler */
    umd_on_terminate_t on_terminate;
    /** on_init handler */
    umd_on_init_t on_init;
    /** cmd_args handler */
    umd_proc_args_t proc_args;
    /** Performance counters context */
    umc_ctx_t *perf;
    /** User data */
    void *data;
};

/** Current UMD */
extern umdaemon_t *UMD;

/**
 * Create new deaemon
 *
 * @param[in]   id      Daemon id string
 * @param[in]   type    Daemon type string
 *
 * @return      Pointer to daemon descriptor
 */
umdaemon_t *umd_create(const char *id, const char *type);

/**
 * Destroy daemon
 *
 * @param[in]   umd     Pointer to daemon descriptor
 */
void umd_destroy(umdaemon_t *umd);

/**
 * Signal handler
 *
 * @param[in]   signum  Signal number
 */
void umd_signal_handler(int signum);

/**
 * Initialize daemon
 *
 * @param[in]   umd     Pointer to daemon descriptor
 */
void umd_init(umdaemon_t *umd);

/**
 * Start daemon
 *
 * @param[in]   umd     Pointer to daemon descriptor
 */
void umd_start(umdaemon_t *umd);

/**
 * Set daemon id
 *
 * @param[in]   umd     Pointer to daemon descriptor
 * @param[in]   id      Daemon id string
 *
 * @return      0 for success or error code
 */
int umd_set_id(umdaemon_t *umd, const char *id);

/**
 * Set daemon log level
 *
 * @param[in]   umd     Pointer to daemon descriptor
 * @param[in]   ll      Log level
 */
void umd_set_log_level(umdaemon_t *umd, enum umd_log_level_t ll);

/**
 * Terminate daemon
 *
 * @param[in]   umd     Pointer to daemon descriptor
 */
void umd_terminate(umdaemon_t *umd);

/**
 * Daemon loop method
 *
 * @param[in]   umd     Pointer to daemon descriptor
 */
void umd_loop(umdaemon_t *umd);

/**
 * Process daemon command line arguments
 *
 * @param[in]   argc    Argument count
 * @param[in]   argv    List of arguments
 */
void umd_proc_args(int argc, char **argv);

/**
 * Log message via syslog
 *
 * @param[in]   umd     Pointer to daemon descriptor
 * @param[in]   level   Log level
 * @param[in]   msg     Log message
 * @param[in]   ...     Extra message arguments (printf style)
 */
void umd_log(umdaemon_t *umd, enum umd_log_level_t level, const char *msg, ...);

/**
 * Check if current daemon is terminating
 *
 * @return      true if terminating
 */
bool umd_is_terminating();

#endif /* ifndef UMDAEMON_H */
