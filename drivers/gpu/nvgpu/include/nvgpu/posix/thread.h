/*
 * Copyright (c) 2018-2020, NVIDIA CORPORATION.  All rights reserved.
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

#ifndef NVGPU_POSIX_THREAD_H
#define NVGPU_POSIX_THREAD_H

#include <pthread.h>

#include <nvgpu/types.h>
#include <nvgpu/atomic.h>

/*
 * For some reason POSIX only allows 16 bytes of name length.
 */

/**
 * Maximum length for thread name.
 */
#define NVGPU_THREAD_POSIX_MAX_NAMELEN		16

/**
 * @brief Macro to push the thread cancellation cleanup handler.
 *
 * @param handler [in]	Handler to be called during cleanup.
 * @param data [in]	Data to be passed as argument to the handler function.
 *
 * Pushes the specified cancellation cleanup handler routine onto the calling
 * thread's cancellation cleanup stack.
 */
#define nvgpu_thread_cleanup_push(handler, data) \
			pthread_cleanup_push(handler, data)

/**
 * @brief Macro to pop the thread cancellation cleanup handler.
 */
#define nvgpu_thread_cleanup_pop() pthread_cleanup_pop((bool)true)

/**
 * Returns the PID of the calling process.
 */
#define nvgpu_getpid getpid

/**
 * Returns the Id of the calling thread.
 */
#define nvgpu_gettid pthread_self

/**
 * Handles passing an nvgpu thread function into a posix thread.
 */
struct nvgpu_posix_thread_data {
	/**
	 * NVGPU Function to be called from the main thread handler.
	 */
	int (*fn)(void *data);
	/**
	 * Data to be passed to the nvgpu thread function.
	 */
	void *data;
#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
	struct nvgpu_posix_fault_inj_container *fi_container;
#endif
};

struct nvgpu_thread {
	/**
	 * Atomic variable to indicate thread running status.
	 */
	nvgpu_atomic_t running;
	/**
	 * Status variable to indicate whether the thread has to stop.
	 */
	bool should_stop;
	/**
	 * Thread identifier.
	 */
	pthread_t thread;
	/**
	 * Structure holding nvpgu specific thread details.
	 */
	struct nvgpu_posix_thread_data nvgpu;
	/**
	 * Name of the thread.
	 */
	char tname[NVGPU_THREAD_POSIX_MAX_NAMELEN];
};

#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
struct nvgpu_posix_fault_inj *nvgpu_thread_get_fault_injection(void);
struct nvgpu_posix_fault_inj
			*nvgpu_thread_running_true_get_fault_injection(void);
#endif

/**
 * @brief Create a thread with specified priority.
 *
 * @param thread [in]		Thread to create.
 * @param data [in]		Data to pass to threadfn.
 * @param threadfn [in]		Thread function.
 * @param priority [in]		Priority of the thread to be created.
 * @param name [in]		Name of the thread.
 *
 * Create a thread with requested priority and run threadfn in it.
 *
 * @return Return 0 on success, else return the error number to indicate the
 * error.
 */
int nvgpu_thread_create_priority(struct nvgpu_thread *thread,
			void *data, int (*threadfn)(void *data),
			int priority, const char *name);

#endif /* NVGPU_POSIX_THREAD_H */
