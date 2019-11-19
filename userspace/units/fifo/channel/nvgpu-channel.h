/*
 * Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */


#ifndef UNIT_NVGPU_CHANNEL_H
#define UNIT_NVGPU_CHANNEL_H

#include <nvgpu/types.h>

struct unit_module;
struct gk20a;

/** @addtogroup SWUTS-fifo-channel
 *  @{
 *
 * Software Unit Test Specification for fifo/channel
 */

/**
 * Test specification for: test_channel_setup_sw
 *
 * Description: Branch coverage for nvgpu_channel_setup/cleanup_sw.
 *
 * Test Type: Feature
 *
 * Targets: nvgpu_channel_setup_sw, nvgpu_channel_init_support,
 *          nvgpu_channel_destroy, nvgpu_channel_cleanup_sw
 *
 * Input: None
 *
 * Steps:
 * - Check valid case for nvgpu_channel_setup_sw.
 * - Check valid case for nvgpu_channel_cleanup_sw.
 * - Check invalid case for nvgpu_channel_setup_sw.
 *   - Failure to allocate channel contexts (by using fault injection for
 *     vzalloc).
 *
 * Output: Returns PASS if all branches gave expected results. FAIL otherwise.
 */
int test_channel_setup_sw(struct unit_module *m,
		struct gk20a *g, void *vargs);

/**
 * Test specification for: test_channel_open
 *
 * Description: Branch coverage for nvgpu_channel_open_new.
 *
 * Test Type: Feature
 *
 * Targets: nvgpu_channel_open_new
 *
 * Input: test_fifo_init_support() run for this GPU
 *
 * Steps:
 * - Check that channel can be allocated with nvgpu_channel_open_new:
 *    - Allocate channel w/ valid runlist_id.
 *    - Allocate channel w/ invalid runlist_id (nvgpu_channel_open_new
 *      should set it to GR runlist_id).
 *    - Allocate w/ or w/o is_privileged_channel set.
 *    - Check that aggresive_sync_destroy is set to true, if used channels
 *      is above threshold (by setting threshold and forcing used_channels
 *      to a greater value).
 *    - Check that nvgpu_channel_open_new returns a non NULL value,
 *      and that ch->g is initialized.
 * - Check channel allocation failures cases:
 *   - Failure to acquire unused channel (by forcibly emptying f->free_chs).
 *   - Failure to allocate channel instance (by using stub for
 *     g->ops.channel.alloc_inst).
 *   - Channel is not referenceable (by forcing ch->referenceable = false and
 *     checking that WARN occurs).
 *   - Channel is in use (by forcing ch->ref_count > 0 and checking that
 *     WARN occurs).
 *   - Allocated channel invalid (by forcing ch->g to NULL value
 *     and checking that BUG occurs).
 *   In negative testing case, original state is restored after checking
 *   that nvgpu_channel_open_new failed.
 *
 * Output: Returns PASS if all branches gave expected results. FAIL otherwise.
 */
int test_channel_open(struct unit_module *m,
		struct gk20a *g, void *vargs);

/**
 * Test specification for: test_channel_close
 *
 * Description: Branch coverage for nvgpu_channel_close/kill.
 *
 * Test Type: Feature
 *
 * Targets: nvgpu_channel_close, nvgpu_channel_kill, channel_free,
 *          channel_free_invoke_unbind, channel_free_wait_for_refs,
 *          channel_free_invoke_deferred_engine_reset,
 *          channel_free_invoke_sync_destroy,
 *          channel_free_put_deterministic_ref_from_init,
 *          channel_free_unlink_debug_session
 *
 * Input: test_fifo_init_support() run for this GPU
 *
 * Steps:
 * - Check valid cases for nvgpu_channel_close/kill:
 *    - Closing channel w/ force = false (nvgpu_channel_close).
 *    - Closing channel w/ force = true (nvgpu_channel_kill).
 *    - Check that g->os_channel.close is called when defined (by using stub).
 *    - Closing a channel bound to TSG.
 *    - Closing a channel with bound AS (by bounding it to dummy VM, and
 *      checking that ref count is decremented).
 *    - Check that g->ops.gr.setup.free_subctx is called when defined.
 *    - Once closed, check that ch->g is NULL, channel is in list of free
 *      channels, and that it is not referenceable.
 * - Check invalid cases:
 *    - Closing a channel while driver is dying (unbind is skipped when
 *      driver is dying).
 *    - Channel already freed (by closing it twice, and checking that BUG
 *      occurs for second invokation).
 *
 * Output: Returns PASS if all branches gave expected results. FAIL otherwise.
 */
int test_channel_close(struct unit_module *m, struct gk20a *g, void *vargs);

/**
 * Test specification for: test_channel_setup_bind
 *
 * Description: Branch coverage for nvgpu_channel_setup_bind.
 *
 * Test Type: Feature
 *
 * Targets: nvgpu_channel_setup_bind, nvgpu_channel_setup_usermode
 *
 * Input: test_fifo_init_support() run for this GPU
 *
 * Steps:
 * - Check valid cases for nvgpu_channel_setup_bind:
 *    - Allocate channel and TSG.
 *    - Bind channel to TSG.
 *    - Allocate dummy pdb_mem, and set dummy VM for ch->vm
 *    - Call nvgpu_channel_setup_bind.
 *    - Check that g->os_channel.alloc_usermode_buffers is called (by using
 *      stub), and that ch->usermode_submit_enabled is true.
 *    - Check that g->ops.runlist.update_for_channel is called for this
 *      channel (by using stub).
 *    - Check that channel is bound (ch->bound = true).
 * - Check invalid cases for nvgpu_channel_setup_bind:
 *    - Channel does not have address space (by setting ch->vm = NULL).
 *    - Channel already has GPFIFO set up (by allocating dummy ch->gpfifo.mem).
 *    - Usermode submit is already set for this channel (by forcing
 *      ch->usermode).
 *    - Closing a channel while driver is dying (unbind is skipped when
 *      drive is dying).
 *    - Channel already freed (by closing it twice, and checking that BUG
 *      occurs for second invokation).
 *   For invalid cases, check that error is returned, and that channel does not
 *   have valid userd or gpfifo.
 *
 * Output: Returns PASS if all branches gave expected results. FAIL otherwise.
 */
int test_channel_setup_bind(struct unit_module *m,
						struct gk20a *g, void *vargs);

/**
 * Test specification for: test_channel_alloc_inst
 *
 * Description: Branch coverage for nvgpu_channel_alloc_inst.
 *
 * Test Type: Feature
 *
 * Targets: nvgpu_channel_alloc_inst, nvgpu_channel_free_inst
 *
 * Input: test_fifo_init_support() run for this GPU
 *
 * Steps:
 * - Check valid cases for nvgpu_channel_alloc_inst:
 *    - Open a channel with nvgpu_channel_open_new, and check that
 *      nvgpu_channel_alloc_inst returns valid DMA memory for ch->inst_block
 *      (aperture != INVALID).
 *    - Free channel instance with nvgpu_channel_free_inst and check
 *      that ch->inst_block has now an invalid aperture.
 * - Check invalid cases for nvgpu_channel_alloc_inst:
 *    - Enable fault injection for DMA allocation, check that
 *      nvgpu_channel_alloc_inst fails and that ch->inst_block.aperture
 *      is invalid.
 *
 * Output: Returns PASS if all branches gave expected results. FAIL otherwise.
 */
int test_channel_alloc_inst(struct unit_module *m,
						struct gk20a *g, void *vargs);

/**
 * Test specification for: test_channel_from_inst
 *
 * Description: Branch coverage for nvgpu_channel_refch_from_inst_ptr.
 *
 * Test Type: Feature
 *
 * Targets: nvgpu_channel_refch_from_inst_ptr
 *
 * Input: test_fifo_init_support() run for this GPU
 *
 * Steps:
 * - Check valid cases for nvgpu_channel_refch_from_inst_ptr:
 *   - Allocate 2 channels each with its instance block.
 *   - Check that chA is retrieved from instA.
 *   - Check that chB is retrieved from instB.
 *   - Check that refcount is incremented for channel.
 * - Check invalid cases for nvgpu_channel_refch_from_inst_ptr:
 *   - Pass invalid inst_ptr and check that no channel is found.
 *
 * Output: Returns PASS if all branches gave expected results. FAIL otherwise.
 */
int test_channel_from_inst(struct unit_module *m,
						struct gk20a *g, void *vargs);

/**
 * Test specification for: test_channel_enable_disable_tsg
 *
 * Description: Branch coverage for nvgpu_channel_enable/disable_tsg.
 *
 * Test Type: Feature
 *
 * Targets: nvgpu_channel_enable_tsg, nvgpu_channel_disable_tsg
 *
 * Input: test_fifo_init_support() run for this GPU
 *
 * Steps:
 * - Allocate channel and TSG, and bind them.
 * - Check that g->ops.tsg.enable is called for TSG when
 *   nvgpu_channel_enable_tsg is called for ch (by using stub).
 * - Check that g->ops.tsg.disable is called for TSG when
 *   nvgpu_channel_disable_tsg is called for ch (by using stub).
 * - Unbind channel from TSG, and check that nvgpu_channel_enable_tsg
 *   and nvgpu_channel_disable_tsg return an error.
 *
 * Output: Returns PASS if all branches gave expected results. FAIL otherwise.
 */
int test_channel_enable_disable_tsg(struct unit_module *m,
						struct gk20a *g, void *vargs);

/**
 * Test specification for: test_channel_abort
 *
 * Description: Test channel TSG abort
 *
 * Test Type: Feature based
 *
 * Targets: nvgpu_channel_abort
 *
 * Input: test_fifo_init_support() run for this GPU
 *
 * Steps:
 * - Test that TSG abort is invoked for TSG bound channel.
 *
 * Output: Returns PASS if all branches gave expected results. FAIL otherwise.
 */
int test_channel_abort(struct unit_module *m, struct gk20a *g, void *vargs);

/**
 * Test specification for: test_channel_mark_error
 *
 * Description: Mark channel as unserviceable
 *
 * Test Type: Feature based
 *
 * Targets: nvgpu_channel_mark_error, nvgpu_channel_set_unserviceable,
 *          nvgpu_channel_ctxsw_timeout_debug_dump_state,
 *          nvgpu_channel_set_has_timedout_and_wakeup_wqs
 *
 * Input: test_fifo_init_support() run for this GPU
 *
 * Steps:
 * - Test that the channel can be marked with error (unserviceable).
 * - Test broadcast condition fail cases.
 *
 * Output: Returns PASS if all branches gave expected results. FAIL otherwise.
 */
int test_channel_mark_error(struct unit_module *m,
						struct gk20a *g, void *vargs);

/**
 * Test specification for: test_channel_sw_quiesce
 *
 * Description: Test emergency quiescing of channels
 *
 * Test Type: Feature based
 *
 * Targets: nvgpu_channel_sw_quiesce, nvgpu_channel_set_error_notifier
 *
 * Input: test_fifo_init_support() run for this GPU
 *
 * Steps:
 * - Check if channel can be placed in quiesce state.
 *
 * Output: Returns PASS if all branches gave expected results. FAIL otherwise.
 */
int test_channel_sw_quiesce(struct unit_module *m,
						struct gk20a *g, void *vargs);

/**
 * Test specification for: test_channel_deterministic_idle_unidle
 *
 * Description: Stop and allow deterministic channel activity
 *
 * Test Type: Feature based
 *
 * Targets: nvgpu_channel_deterministic_idle, nvgpu_channel_deterministic_unidle
 *
 * Input: test_fifo_init_support() run for this GPU
 *
 * Steps:
 * - Execute deterministic idle and unidle functions and check if gpu usage
 *   usage count is updated corresponding to input conditions.
 *
 * Output: Returns PASS if all branches gave expected results. FAIL otherwise.
 */
int test_channel_deterministic_idle_unidle(struct unit_module *m,
						struct gk20a *g, void *vargs);

/**
 * Test specification for: test_channel_suspend_resume_serviceable_chs
 *
 * Description: Test suspend resume of all servicable channels
 *
 * Test Type: Feature based
 *
 * Targets: nvgpu_channel_suspend_all_serviceable_ch,
 *          nvgpu_channel_resume_all_serviceable_ch,
 *          nvgpu_channel_check_unserviceable
 *
 * Input: test_fifo_init_support() run for this GPU
 *
 * Steps:
 * - Check if channels can be suspended and resumed.
 *
 * Output: Returns PASS if all branches gave expected results. FAIL otherwise.
 */
int test_channel_suspend_resume_serviceable_chs(struct unit_module *m,
						struct gk20a *g, void *vargs);

/**
 * Test specification for: test_channel_debug_dump
 *
 * Description: Dump channel debug information
 *
 * Test Type: Feature based
 *
 * Targets: nvgpu_channel_debug_dump_all
 *
 * Input: test_fifo_init_support() run for this GPU
 *
 * Steps:
 * - Dump all debug information for channels.
 *
 * Output: Returns PASS if all branches gave expected results. FAIL otherwise.
 */
int test_channel_debug_dump(struct unit_module *m,
						struct gk20a *g, void *vargs);

/**
 * Test specification for: test_channel_semaphore_wakeup
 *
 * Description: Wake up threads waiting for semaphore
 *
 * Test Type: Feature based
 *
 * Targets: nvgpu_channel_semaphore_wakeup
 *
 * Input: test_fifo_init_support() run for this GPU
 *
 * Steps:
 * - Execute semaphore_wakeup for deterministic/non-deterministic channels.
 *
 * Output: Returns PASS if all branches gave expected results. FAIL otherwise.
 */
int test_channel_semaphore_wakeup(struct unit_module *m,
						struct gk20a *g, void *vargs);

/**
 * Test specification for: test_channel_from_invalid_id
 *
 * Description: Test channel reference extracted using channel id
 *
 * Test Type: Feature based
 *
 * Targets: nvgpu_channel_from_id
 *
 * Input: test_fifo_init_support() run for this GPU
 *
 * Steps:
 * - Test corner case to retrieve channel with invalid channel id.
 *
 * Output: Returns PASS if all branches gave expected results. FAIL otherwise.
 */
int test_channel_from_invalid_id(struct unit_module *m, struct gk20a *g,
								void *vargs);

/**
 * Test specification for: test_channel_put_warn
 *
 * Description: Test channel dereference
 *
 * Test Type: Feature based
 *
 * Targets: nvgpu_channel_put__func
 *
 * Input: test_fifo_init_support() run for this GPU
 *
 * Steps:
 * - Test corner cases using referenceable channel and condition broadcast fail
 *   cases.
 *
 * Output: Returns PASS if all branches gave expected results. FAIL otherwise.
 */
int test_channel_put_warn(struct unit_module *m, struct gk20a *g, void *vargs);

/**
 * Test specification for: test_ch_referenceable_cleanup
 *
 * Description: Test channel cleanup corner case
 *
 * Test Type: Feature based
 *
 * Targets: nvgpu_channel_cleanup_sw
 *
 * Input: test_fifo_init_support() run for this GPU
 *
 * Steps:
 * - Open a channel. Test how referenceable channel is cleaned-up/freed.
 *
 * Output: Returns PASS if all branches gave expected results. FAIL otherwise.
 */
int test_ch_referenceable_cleanup(struct unit_module *m,
						struct gk20a *g, void *vargs);

/**
 * Test specification for: test_channel_abort_cleanup
 *
 * Description: Test channel abort cleanup with user_sync available
 *
 * Test Type: Feature based
 *
 * Targets: nvgpu_channel_abort_clean_up
 *
 * Input: test_fifo_init_support() run for this GPU
 *
 * Steps:
 * - Bind channel to TSG and allocate channel user_sync. Test channel abort
 *   cleanup while unbinding from TSG.
 *
 * Output: Returns PASS if all branches gave expected results. FAIL otherwise.
 */
int test_channel_abort_cleanup(struct unit_module *m, struct gk20a *g,
								void *vargs);
/**
 * @}
 */

#endif /* UNIT_NVGPU_CHANNEL_H */