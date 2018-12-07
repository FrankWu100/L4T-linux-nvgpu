/*
 * Copyright (c) 2018, NVIDIA CORPORATION.  All rights reserved.
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

#include <unit/io.h>
#include <unit/unit.h>

#include <nvgpu/kmem.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/nvgpu_sgt.h>
#include <nvgpu/sizes.h>
#include <os/posix/os_posix.h>

/* nvgpu_sgt_ops overrides for basic api testing */
#define EXPECTED_U64 0x123456789ABCDEF0ULL
static u64 ops_sgl_gpu_addr(struct gk20a *g, struct nvgpu_sgl *sgl,
				   struct nvgpu_gmmu_attrs *attrs)
{
	return EXPECTED_U64;
}

static u64 ops_sgl_ipa(struct gk20a *g, struct nvgpu_sgl *sgl)
{
	return EXPECTED_U64;
}

static u64 ops_sgl_ipa_to_pa(struct gk20a *g, struct nvgpu_sgl *sgl,
			     u64 ipa, u64 *pa_len)
{
	return EXPECTED_U64;
}

static struct nvgpu_sgt_ops nvgpu_sgt_ops = {
	.sgl_gpu_addr = ops_sgl_gpu_addr,
	.sgl_ipa = ops_sgl_ipa,
	.sgl_ipa_to_pa = ops_sgl_ipa_to_pa,
};

/*
 * Test: test_nvgpu_sgt_basic_apis
 * Tests for the simple APIs provided by nvgpu_sgt unit
 * APIs tested:
 *      nvgpu_sgt_create_from_mem
 *      nvgpu_sgt_get_dma
 *      nvgpu_sgt_get_phys
 *      nvgpu_sgt_iommuable
 *      nvgpu_sgt_get_gpu_addr
 *      nvgpu_sgt_get_ipa
 *      nvgpu_sgt_ipa_to_pa
 *      nvgpu_sgt_free
 */
static int test_nvgpu_sgt_basic_apis(struct unit_module *m, struct gk20a *g,
				     void *args)
{
	int ret = UNIT_SUCCESS;
	struct nvgpu_os_posix *p = nvgpu_os_posix_from_gk20a(g);
	struct nvgpu_mem mem = { };
	struct nvgpu_sgt *sgt;
	struct nvgpu_mem_sgl *sgl;
	struct nvgpu_sgt_ops const *saved_ops_ptr;
	struct nvgpu_gmmu_attrs attrs;

	mem.size = SZ_1M;
	mem.cpu_va = (void *)0x10000000;
	sgt = nvgpu_sgt_create_from_mem(g, &mem);
	if (sgt == NULL) {
		unit_err(m, "%s: nvgpu_sgt_create_from_mem failed\n", __func__);
		ret = UNIT_FAIL;
		goto end;
	}
	sgl = (struct nvgpu_mem_sgl *)sgt->sgl;
	sgl->dma = 0x200000000;
	if (nvgpu_sgt_get_phys(g, sgt, sgt->sgl) != U64(mem.cpu_va)) {
		unit_err(m, "%s: bad phys returned\n", __func__);
		ret = UNIT_FAIL;
	}

	if (nvgpu_sgt_get_dma(sgt, sgt->sgl) != U64(sgl->dma)) {
		unit_err(m, "%s: bad dma address returned\n", __func__);
		ret = UNIT_FAIL;
	}

	p->mm_sgt_is_iommuable = false;
	if (nvgpu_sgt_iommuable(g, sgt) != p->mm_sgt_is_iommuable) {
		unit_err(m, "%s: nvgpu_sgt_iommuable wrong, expected %s\n",
			 __func__, p->mm_sgt_is_iommuable ? "true" : "false");
		ret = UNIT_FAIL;
	}
	p->mm_sgt_is_iommuable = true;
	if (nvgpu_sgt_iommuable(g, sgt) != p->mm_sgt_is_iommuable) {
		unit_err(m, "%s: nvgpu_sgt_iommuable wrong, expected %s\n",
			 __func__, p->mm_sgt_is_iommuable ? "true" : "false");
		ret = UNIT_FAIL;
	}

	/* use our op for this API, and restore default later */
	saved_ops_ptr = sgt->ops;
	sgt->ops = &nvgpu_sgt_ops;

	/* this tests the case where sgt_iommuable op is NULL */
	if (nvgpu_sgt_iommuable(g, sgt)) {
		unit_err(m, "%s: nvgpu_sgt_iommuable wrong, expected %s\n",
			 __func__, p->mm_sgt_is_iommuable ? "true" : "false");
		ret = UNIT_FAIL;
	}
	/* set POSIX IOMMU state back to default */
	p->mm_sgt_is_iommuable = false;

	/* the underlying op is overridden to return an expected value */
	if (nvgpu_sgt_get_gpu_addr(g, sgt, sgt->sgl, &attrs) != EXPECTED_U64) {
		unit_err(m, "%s: nvgpu_sgt_get_gpu_addr incorrect\n", __func__);
		ret = UNIT_FAIL;
	}

	/* the underlying op is overridden to return an expected value */
	if (nvgpu_sgt_get_ipa(g, sgt, sgt->sgl) != EXPECTED_U64) {
		unit_err(m, "%s: nvgpu_sgt_get_ipa incorrect\n", __func__);
		ret = UNIT_FAIL;
	}

	/* the underlying op is overridden to return an expected value */
	if (nvgpu_sgt_ipa_to_pa(g, sgt, sgt->sgl, 0ULL, NULL) != EXPECTED_U64) {
		unit_err(m, "%s: nvgpu_sgt_get_ipa incorrect\n", __func__);
		ret = UNIT_FAIL;
	}

	/* test free with NULL sgt for error checking path */
	nvgpu_sgt_free(g, NULL);
	/* test free with NULL free op for error checking path */
	nvgpu_sgt_free(g, sgt);

	/* restore default ops */
	sgt->ops = saved_ops_ptr;

	nvgpu_sgt_free(g, sgt);

end:
	return ret;
}

/*
 * Test: test_nvgpu_sgt_get_next
 * Tests test_nvgpu_sgt_get_next API by building sgl's and verifying
 * correct pointers returned by calling the API
 */
static int test_nvgpu_sgt_get_next(struct unit_module *m, struct gk20a *g,
				   void *args)
{
	int ret = UNIT_SUCCESS;
	struct nvgpu_mem mem;
	struct nvgpu_sgt *sgt;
	struct nvgpu_sgl *created_sgl, *api_ptr;
	struct nvgpu_mem_sgl *sgl_ptr, *test_sgl = NULL;
	int i;
#define SGL_LEN 100

	/* create sgl for this test */
	for (i = 0; i < SGL_LEN; i++) {
		struct nvgpu_mem_sgl *tptr;

		tptr = (struct nvgpu_mem_sgl *)nvgpu_kzalloc(g,
					sizeof(struct nvgpu_mem_sgl));
		if (tptr == NULL) {
			unit_err(m, "%s: failed to alloc sgl\n", __func__);
			ret = UNIT_FAIL;
			goto dealloc_sgls;
		}

		if (i == 0) {
			sgl_ptr = tptr;
			test_sgl = sgl_ptr;
		} else {
			sgl_ptr->next = tptr;
			sgl_ptr = sgl_ptr->next;
		}
		sgl_ptr->next = NULL;
		sgl_ptr->phys = i;
		sgl_ptr->dma = i;
		sgl_ptr->length = i;
	}

	sgt = nvgpu_sgt_create_from_mem(g, &mem);
	if (sgt == NULL) {
		unit_err(m, "%s: nvgpu_sgt_create_from_mem failed\n",
			 __func__);
		ret = UNIT_FAIL;
		goto dealloc_sgls;
	}
	/* save this for freeing later */
	created_sgl = sgt->sgl;
	sgt->sgl = (struct nvgpu_sgl *)test_sgl;

	api_ptr = sgt->sgl;
	sgl_ptr = test_sgl;
	for (i = 0; i < SGL_LEN; i++) {
		api_ptr = nvgpu_sgt_get_next(sgt, api_ptr);
		if ((struct nvgpu_mem_sgl *)api_ptr != sgl_ptr->next) {
			unit_err(m, "%s: sgl's out of sync\n", __func__);
			ret = UNIT_FAIL;
			goto free_sgt;
		}
		sgl_ptr = sgl_ptr->next;
	}

	if (api_ptr != NULL) {
		unit_err(m, "%s: sgl's didn't end with NULL\n", __func__);
		ret = UNIT_FAIL;
	}

free_sgt:
	/* free everything */
	sgt->sgl = created_sgl;
	nvgpu_sgt_free(g, sgt);

dealloc_sgls:
	while (test_sgl != NULL) {
		sgl_ptr = test_sgl->next;
		nvgpu_kfree(g, test_sgl);
		test_sgl = sgl_ptr;
	}

	return ret;
}

/* structures to make a table for testing the alignment API */
struct sgt_test_align_table {
	u64 addr;
	u64 length;
};

#define TEST_ALIGN_TABLE_MAX 100
struct sgt_test_align_args {
	u64 test_align_result;
	u32 test_align_table_len;
	struct sgt_test_align_table test_align_table[TEST_ALIGN_TABLE_MAX];
};

/* table of sgls for testing calculation of alignment */
static struct sgt_test_align_args sgt_align_test_array[] = {
	{
		.test_align_table = {
			{.addr = 0x00000000,	.length = SZ_1M},
			{.addr = 0x00400000,	.length = SZ_1M},
			{.addr = 0x00200000,	.length = SZ_1M},
		},
		.test_align_table_len = 3,
		.test_align_result = SZ_1M,
	},
	{
		.test_align_table = {
			{.addr = 0x00000000,	.length = SZ_4K},
			{.addr = 0x00200000,	.length = SZ_64K},
			{.addr = 0x00100000,	.length = SZ_1M},
		},
		.test_align_table_len = 3,
		.test_align_result = SZ_4K,
	},
	{
		.test_align_table = {
			{.addr = 0x00100000,	.length = SZ_1M},
			{.addr = 0x00010000,	.length = SZ_64K},
			{.addr = 0x00001000,	.length = SZ_4K},
		},
		.test_align_table_len = 3,
		.test_align_result = SZ_4K,
	},
	{
		.test_align_table = {
			{.addr = 0x00100000,	.length = SZ_1M},
			{.addr = 0x00010000,	.length = SZ_64K},
			{.addr = 0x00001000,	.length = SZ_128K},
		},
		.test_align_table_len = 3,
		.test_align_result = SZ_4K,
	},
};

static int test_table_nvgpu_sgt_alignment_non_iommu(struct unit_module *m,
					struct gk20a *g,
					struct sgt_test_align_args *args)
{
	int ret = UNIT_SUCCESS;
	struct nvgpu_mem mem;
	struct nvgpu_sgt *sgt;
	struct nvgpu_sgl *created_sgl;
	struct nvgpu_mem_sgl *sgl_ptr, *test_sgl = NULL;
	u32 i;
	u64 alignment;

	/* create sgl for this test */
	for (i = 0; i < args->test_align_table_len; i++) {
		struct nvgpu_mem_sgl *tptr;

		tptr = (struct nvgpu_mem_sgl *)nvgpu_kzalloc(g,
						sizeof(struct nvgpu_mem_sgl));
		if (tptr == NULL) {
			unit_err(m, "%s: failed to alloc sgl\n", __func__);
			ret = UNIT_FAIL;
			goto dealloc_sgls;
		}

		if (i == 0) {
			sgl_ptr = tptr;
			test_sgl = sgl_ptr;
		} else {
			sgl_ptr->next = tptr;
			sgl_ptr = sgl_ptr->next;
		}
		sgl_ptr->next = NULL;
		sgl_ptr->phys = args->test_align_table[i].addr;
		sgl_ptr->dma = args->test_align_table[i].addr;
		sgl_ptr->length = args->test_align_table[i].length;
	}

	sgt = nvgpu_sgt_create_from_mem(g, &mem);
	if (sgt == NULL) {
		unit_err(m, "%s: nvgpu_sgt_create_from_mem failed\n",
			 __func__);
		ret = UNIT_FAIL;
		goto dealloc_sgls;
	}
	/* save this for freeing later */
	created_sgl = sgt->sgl;
	sgt->sgl = (struct nvgpu_sgl *)test_sgl;

	alignment = nvgpu_sgt_alignment(g, sgt);
	if (alignment != args->test_align_result) {
		unit_err(m, "%s: incorrect alignment 0x%llx != 0x%llx\n",
			 __func__, alignment, args->test_align_result);
		ret = UNIT_FAIL;
	}

	/* free everything */
	sgt->sgl = created_sgl;
	nvgpu_sgt_free(g, sgt);

dealloc_sgls:
	while (test_sgl != NULL) {
		sgl_ptr = test_sgl->next;
		nvgpu_kfree(g, test_sgl);
		test_sgl = sgl_ptr;
	}

	return ret;
}

/*
 * Test: test_nvgpu_sgt_alignment_non_iommu
 * Walks table above to test a variety of different sgl's to test alignment
 * API calculates correctly when there is no IOMMU.
 */
static int test_nvgpu_sgt_alignment_non_iommu(struct unit_module *m,
					      struct gk20a *g, void *args)
{
	int ret = UNIT_SUCCESS;
	struct nvgpu_os_posix *p = nvgpu_os_posix_from_gk20a(g);
	int table_len = sizeof(sgt_align_test_array) /
			sizeof(sgt_align_test_array[0]);
	int i;

	p->mm_is_iommuable = false;
	for (i = 0; i < table_len; i++) {
		if (test_table_nvgpu_sgt_alignment_non_iommu(m, g,
					&sgt_align_test_array[i]) != 0) {
			unit_err(m, "%s: array index i=%d failed\n",
			 __func__, i);
			ret = UNIT_FAIL;
		}
	}

	return ret;
}

/*
 * Test the alignment API for the case where there is an IOMMU
 * For this case, we need:
 *  1. an IOMMU
 *  2. the sgt to be marked IOMMU'able
 *  3. DMA addr != 0
 * So, we check we don't use the IOMMU code path by not having these right.
 * To make sure we didn't we make the DMA address and size different.
 * Because if it's IOMMU'able the alignment is based on the DMA address.
 */
static int test_nvgpu_sgt_alignment_with_iommu(struct unit_module *m,
					       struct gk20a *g, void *args)
{
	int ret = UNIT_SUCCESS;
	struct nvgpu_os_posix *p = nvgpu_os_posix_from_gk20a(g);
	struct nvgpu_mem mem = { };
	struct nvgpu_sgt *sgt;
	struct nvgpu_mem_sgl *sgl;
	u64 alignment;

	/* use a bitmask to cycle thru bad combos */
	u8 bitmask;
#define IOMMU_BIT	0
#define SGT_IOMMU_BIT	1
#define DMA_ADDR_BIT	2

	mem.size = SZ_256M;
	mem.cpu_va = (void *)SZ_4K;
	sgt = nvgpu_sgt_create_from_mem(g, &mem);
	sgl = (struct nvgpu_mem_sgl *)sgt->sgl;

	for (bitmask = 0; bitmask < 7; bitmask++) {
		p->mm_is_iommuable = (bitmask & (1 << IOMMU_BIT)) != 0;
		p->mm_sgt_is_iommuable = (bitmask & (1 << SGT_IOMMU_BIT)) != 0;
		sgl->dma = ((bitmask & (1 << DMA_ADDR_BIT)) != 0) ?
				(2*SZ_256M) : 0;
		alignment = nvgpu_sgt_alignment(g, sgt);
		if (alignment == sgl->dma) {
			unit_err(m, "%s: should have incorrect alignment (0x%x)\n",
				__func__, bitmask);
			ret = UNIT_FAIL;
		}
	}

	p->mm_is_iommuable = true;
	p->mm_sgt_is_iommuable = true;
	sgl->dma = 2*SZ_256M;
	alignment = nvgpu_sgt_alignment(g, sgt);
	if (alignment != sgl->dma) {
		unit_err(m, "%s: incorrect alignment 0x%llx != 0x%llx\n",
			__func__, alignment, sgl->dma);
		ret = UNIT_FAIL;
	}

	p->mm_is_iommuable = false;
	p->mm_sgt_is_iommuable = false;
	nvgpu_sgt_free(g, sgt);

	return ret;
}

struct unit_module_test nvgpu_sgt_tests[] = {
	UNIT_TEST(sgt_basic_apis,		test_nvgpu_sgt_basic_apis,		NULL),
	UNIT_TEST(sgt_get_next,			test_nvgpu_sgt_get_next,		NULL),
	UNIT_TEST(sgt_alignment_non_iommu,	test_nvgpu_sgt_alignment_non_iommu,	NULL),
	UNIT_TEST(sgt_alignment_with_iommu,	test_nvgpu_sgt_alignment_with_iommu,	NULL),
};

UNIT_MODULE(nvgpu_sgt, nvgpu_sgt_tests, UNIT_PRIO_NVGPU_TEST);
