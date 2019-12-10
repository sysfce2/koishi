
#include <koishi.h>
#include <emscripten.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

typedef void *fiber_handle_t;

typedef struct asyncify_fiber {
	fiber_handle_t handle;
	koishi_entrypoint_t entry;
} koishi_fiber_t;

#include "../fiber.h"

fiber_handle_t _koishi_impl_fiber_create(uint32_t stack_size, void (*entry)(void));
fiber_handle_t _koishi_impl_fiber_create_for_main_ctx(void);
void _koishi_impl_fiber_recycle(fiber_handle_t fiber, void (*entry)(void));
void _koishi_impl_fiber_free(fiber_handle_t fiber);
void _koishi_impl_fiber_swap(fiber_handle_t old, fiber_handle_t new);

KOISHI_NORETURN static void co_entry(void) {
	koishi_coroutine_t *co = co_current;
	co->userdata = co->fiber.entry(co->userdata);
	koishi_return_to_caller(co, KOISHI_DEAD);
	KOISHI_UNREACHABLE;
}

static void koishi_fiber_init(koishi_fiber_t *fiber, size_t min_stack_size, koishi_entrypoint_t entry_point) {
	size_t stack_size = koishi_util_real_stack_size(min_stack_size);
	assert(stack_size < UINT32_MAX);
	fiber->handle = _koishi_impl_fiber_create((uint32_t)stack_size, co_entry);
	assert(fiber->handle != 0);
	fiber->entry = entry_point;
}

static void koishi_fiber_deinit(koishi_fiber_t *fiber) {
	if(fiber->handle) {
		_koishi_impl_fiber_free(fiber->handle);
		fiber->handle = 0;
	}
}

static void koishi_fiber_init_main(koishi_fiber_t *fiber) {
	fiber->handle = _koishi_impl_fiber_create_for_main_ctx();
}

static void koishi_fiber_recycle(koishi_fiber_t *fiber, koishi_entrypoint_t entry_point) {
	_koishi_impl_fiber_recycle(fiber->handle, co_entry);
	fiber->entry = entry_point;
}

static void koishi_fiber_swap(koishi_fiber_t *from, koishi_fiber_t *to) {
	_koishi_impl_fiber_swap(from->handle, to->handle);
}