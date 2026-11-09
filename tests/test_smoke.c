#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

static void test_cmocka_works(void **state) {
  (void)state;
  assert_int_equal(1 + 1, 2);
}

int main() {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_cmocka_works),
  };

	return cmocka_run_group_tests(tests, NULL, NULL);
}
