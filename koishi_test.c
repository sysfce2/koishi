
#include <koishi.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

void *cofunc1(void *data) {
	assert(koishi_state(koishi_active()) == KOISHI_RUNNING);

	char *str = data;
	printf("C: start coroutine (got %s)\n", str);
	printf("C: yielding 1\n");
	str = koishi_yield("Reimu");
	printf("C: resumed 1 (got %s)\n", str);
	printf("C: yielding 2\n");
	str = koishi_yield("Marisa");
	printf("C: resumed 2 (got %s)\n", str);
	printf("C: yielding 3\n");
	str = koishi_yield("Youmu");
	printf("C: resumed 3 (got %s)\n", str);
	printf("C: done\n");
	return "Bye";
}

void *cofunc_nested(void *data) {
	koishi_coroutine_t *caller = data;
	assert(koishi_state(caller) == KOISHI_SUSPENDED);
	assert(koishi_state(koishi_active()) == KOISHI_RUNNING);
	return (void*)42;
}

void *cofunc2(void *data) {
	assert(koishi_state(koishi_active()) == KOISHI_RUNNING);
	koishi_coroutine_t co, *c2 = &co;
	koishi_init(c2, 0, cofunc_nested);
	int nested_return = (intptr_t)koishi_resume(c2, koishi_active());
	assert(koishi_state(koishi_active()) == KOISHI_RUNNING);
	assert(nested_return == 42);

	intptr_t i = 0;
	printf("C: start coroutine\n");
	while(1) {
		i += (intptr_t)data;
		i = (2 * i) * (3 * i);
		printf("C: yielding %zi\n", (size_t)i);
		assert(koishi_state(koishi_active()) == KOISHI_RUNNING);
		koishi_yield((void*)i);
		assert(koishi_state(koishi_active()) == KOISHI_RUNNING);
	}

	return NULL;
}

int main(int argc, char **argv) {
	if(argc != 1) {
		printf("%s takes no arguments.\n", argv[0]);
		return 1;
	}

	assert(koishi_state(koishi_active()) == KOISHI_RUNNING);

	char *str;
	koishi_coroutine_t co, *c = &co;
	koishi_init(c, 128 * 1024, cofunc1);
	printf("O: created coroutine\n");
	printf("O: resume 1\n");
	str = koishi_resume(c, "Hello");
	printf("O: post yield 1 (got %s)\n", str);
	printf("O: resume 2\n");
	str = koishi_resume(c, "Hakurei");
	printf("O: post yield 2 (got %s)\n", str);
	printf("O: resume 3\n");
	str = koishi_resume(c, "Kirisame");
	printf("O: post yield 3 (got %s)\n", str);
	printf("O: resume 4\n");
	str = koishi_resume(c, "Konpaku");
	printf("O: done (got %s)\n", str);
	assert(koishi_state(c) == KOISHI_DEAD);

	assert(koishi_state(koishi_active()) == KOISHI_RUNNING);

	intptr_t i;
	koishi_recycle(c, cofunc2);
	assert(koishi_state(c) == KOISHI_SUSPENDED);
	printf("O: recycled coroutine\n");
	printf("O: resume 1\n");
	i = (intptr_t)koishi_resume(c, (void*)1);
	printf("O: post yield 1 (got %zi)\n", (size_t)i);
	printf("O: resume 2\n");
	i = (intptr_t)koishi_resume(c, (void*)2);
	printf("O: post yield 2 (got %zi)\n", (size_t)i);
	assert(koishi_state(c) == KOISHI_SUSPENDED);

	koishi_deinit(c);

	return 0;
}