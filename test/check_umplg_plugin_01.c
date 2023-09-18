/*
 *               _____  ____ __
 *   __ ____ _  /  _/ |/ / //_/
 *  / // /  ' \_/ //    / ,<
 *  \_,_/_/_/_/___/_/|_/_/|_|
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <umink_pkg_config.h>
#include <umink_plugin.h>

/*************/
/* Plugin ID */
/*************/
static const char *PLG_ID = "test_plugin_01";

/**********************************************/
/* list of command implemented by this plugin */
/**********************************************/
int COMMANDS[] = {
    // end of list marker
    -1
};

/*********************/
/* terminate handler */
/*********************/
int
terminate(umplg_mngr_t *pm, umplgd_t *pd, int phase)
{
    return 0;
}

/****************/
/* init handler */
/****************/
#ifdef FAIL_TESTS
static int
#else
int
#endif
init(umplg_mngr_t *pm, umplgd_t *pd)
{
    return 0;
}

/*************************/
/* local command handler */
/*************************/
int
run_local(umplg_mngr_t *pm, umplgd_t *pd, int cmd_id, umplg_idata_t *data)
{
    return 0;
}

/*******************/
/* command handler */
/*******************/
int
run(umplg_mngr_t *pm, umplgd_t *pd, int cmd_id, umplg_idata_t *data)
{
    // not used
    return 0;
}
