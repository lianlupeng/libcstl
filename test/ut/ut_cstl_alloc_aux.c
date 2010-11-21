#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <google/cmockery.h>

#include <stdlib.h>

#include "ut_cstl_alloc_aux.h"

#include "cstl/cstl_alloc.h"
#include "cstl_alloc_aux.h"

#ifndef _CSTL_USER_MODEL
/*
 * test _alloc_get_memory_chunk
 */
void test__alloc_get_memory_chunk__invalid_allocator(void** state)
{
    size_t t_alloccount = 2;
    expect_assert_failure(_alloc_get_memory_chunk(NULL, 8, &t_alloccount));
}

void test__alloc_get_memory_chunk__invalid_alloccount(void** state)
{
    alloc_t allocator;
    expect_assert_failure(_alloc_get_memory_chunk(&allocator, 8, NULL));
}

void test__alloc_get_memory_chunk__total_size_less_than_memory_pool(void** state)
{
    size_t t_alloccount = 2;
    alloc_t allocator;
    char* pc_malloc = NULL;
    size_t t_old_size = 0;
    _alloc_init(&allocator);

    allocator._t_mempoolsize = t_old_size = 64 * sizeof(char);
    allocator._pc_mempool = (char*)malloc(allocator._t_mempoolsize);
    allocator._ppc_allocatemempool[allocator._t_mempoolindex++] = allocator._pc_mempool;

    pc_malloc = _alloc_get_memory_chunk(&allocator, 8, &t_alloccount);

    assert_true(pc_malloc != NULL);
    assert_true(t_alloccount == 2);
    assert_true(pc_malloc == allocator._ppc_allocatemempool[0]);
    assert_true(allocator._t_mempoolsize == t_old_size - 2 * 8);
    assert_true(allocator._pc_mempool = allocator._ppc_allocatemempool[0] + 2 * 8);

    _alloc_destroy(&allocator);
}

void test__alloc_get_memory_chunk__memory_pool_less_than_total_size_greater_than_one_size(void** state)
{
    size_t t_alloccount = 100;
    alloc_t allocator;
    char* pc_malloc = NULL;
    size_t t_old_size = 0;
    _alloc_init(&allocator);

    allocator._t_mempoolsize = t_old_size = 64 * sizeof(char);
    allocator._pc_mempool = (char*)malloc(allocator._t_mempoolsize);
    allocator._ppc_allocatemempool[allocator._t_mempoolindex++] = allocator._pc_mempool;

    pc_malloc = _alloc_get_memory_chunk(&allocator, 8, &t_alloccount);

    assert_true(pc_malloc != NULL);
    assert_true(t_alloccount == 8);
    assert_true(pc_malloc == allocator._ppc_allocatemempool[0]);
    assert_true(allocator._t_mempoolsize == 0);
    assert_true(allocator._pc_mempool = allocator._ppc_allocatemempool[0] + t_old_size);

    _alloc_destroy(&allocator);
}

void test__alloc_get_memory_chunk__memory_pool_size_equal_to_0(void** state)
{
    size_t t_alloccount = 10;
    alloc_t allocator;
    char* pc_malloc = NULL;
    _alloc_init(&allocator);

    pc_malloc = _alloc_get_memory_chunk(&allocator, 8, &t_alloccount);

    assert_true(pc_malloc != NULL);
    assert_true(t_alloccount == 10);
    assert_true(pc_malloc == allocator._ppc_allocatemempool[0]);
    assert_true(allocator._t_mempoolsize == 80);
    assert_true(allocator._pc_mempool = allocator._ppc_allocatemempool[0] + t_alloccount * 8);

    _alloc_destroy(&allocator);
}

void test__alloc_get_memory_chunk__memory_pool_size_less_then_one_allocated_chunk(void** state)
{
    size_t t_alloccount = 2;
    alloc_t allocator;
    char* pc_malloc = NULL;
    size_t t_old_size = 0;
    _alloc_init(&allocator);

    allocator._t_mempoolsize = t_old_size = 8;
    allocator._pc_mempool = (char*)malloc(allocator._t_mempoolsize);
    allocator._ppc_allocatemempool[allocator._t_mempoolindex++] = allocator._pc_mempool;

    pc_malloc = _alloc_get_memory_chunk(&allocator, 16, &t_alloccount);

    assert_true(pc_malloc != NULL);
    assert_true(t_alloccount == 2);
    assert_true(pc_malloc == allocator._ppc_allocatemempool[1]);
    assert_true(allocator._t_mempoolsize == 32);
    assert_true(allocator._pc_mempool = allocator._ppc_allocatemempool[1] + 32);
    assert_true(allocator._ppc_allocatemempool[0] == (char*)allocator._apt_memlink[_MEMLIST_INDEX(t_old_size)]);

    _alloc_destroy(&allocator);
}

void test__alloc_get_memory_chunk__reallocate_memory_pool_container(void** state)
{
    size_t t_alloccount = 2;
    alloc_t allocator;
    char* pc_malloc = NULL;
    size_t t_old_size = 0;
    _alloc_init(&allocator);

    allocator._t_mempoolsize = t_old_size = 8;
    allocator._pc_mempool = (char*)malloc(allocator._t_mempoolsize);
    allocator._t_mempoolindex = _MEM_POOL_DEFAULT_COUNT - 1;
    allocator._ppc_allocatemempool[allocator._t_mempoolindex++] = allocator._pc_mempool;

    pc_malloc = _alloc_get_memory_chunk(&allocator, 16, &t_alloccount);

    assert_true(pc_malloc != NULL);
    assert_true(t_alloccount == 2);
    assert_true(pc_malloc == allocator._ppc_allocatemempool[_MEM_POOL_DEFAULT_COUNT]);
    assert_true(allocator._t_mempoolcount = 2 * _MEM_POOL_DEFAULT_COUNT);
    assert_true(allocator._t_mempoolsize == 32);
    assert_true(allocator._pc_mempool = allocator._ppc_allocatemempool[_MEM_POOL_DEFAULT_COUNT] + 32);
    assert_true(allocator._ppc_allocatemempool[_MEM_POOL_DEFAULT_COUNT - 1] == 
            (char*)allocator._apt_memlink[_MEMLIST_INDEX(t_old_size)]);

    _alloc_destroy(&allocator);
}

/*
 * test _alloc_apply_formated_memory
 */
void test__alloc_apply_formated_memory__invalid_allocator(void** state)
{
    expect_assert_failure(_alloc_apply_formated_memory(NULL, 8));
}

void test__alloc_apply_formated_memory__success(void** state)
{
    alloc_t allocator;
    _memlink_t* pt_link = NULL;
    _memlink_t* pt_link_prev = NULL;
    _alloc_init(&allocator);

    _alloc_apply_formated_memory(&allocator, 8);

    for(pt_link_prev = allocator._apt_memlink[_MEMLIST_INDEX(8)], pt_link = pt_link_prev->_pui_nextmem;
        pt_link != NULL; pt_link_prev = pt_link, pt_link = pt_link_prev->_pui_nextmem)
    {
        assert_true((char*)pt_link == (char*)pt_link_prev + 8);
    }
    assert_true((char*)allocator._pc_mempool == (char*)pt_link_prev + 8);

    _alloc_destroy(&allocator);
}
#endif /* _CSTL_USER_MODEL */

/*
 * test _alloc_free
 */
void test__alloc_free__invalid_memory(void** state)
{
    expect_assert_failure(_alloc_free(NULL));
}

/*
 * test _alloc_malloc
 */
void test__alloc_malloc__success(void** state)
{
    void* pv_allocmem = _alloc_malloc(8);
    assert_true(pv_allocmem != NULL);
    _alloc_free(pv_allocmem);
}

/*
 * test _alloc_malloc_out_of_memory
 */
static void malloc_handler(void){}

void test__alloc_malloc_out_of_memory__success(void** state)
{
    void* pv_allocmem = NULL;
    _gpfun_malloc_handler = malloc_handler;

    pv_allocmem = _alloc_malloc_out_of_memory(8);
    assert_true(pv_allocmem != NULL);
    _alloc_free(pv_allocmem);
}

