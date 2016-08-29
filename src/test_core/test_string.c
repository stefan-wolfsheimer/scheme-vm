#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "core/lisp_vm.h" 
#include "test_core/lisp_assertion.h"

static void test_create_string(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t str;
  lisp_make_string(vm, &str, "abc");
  ASSERT(tst,         LISP_IS_OBJECT(&str));
  ASSERT(tst,         LISP_IS_STRING(&str));
  ASSERT(tst,         LISP_REFCOUNT(&str) == 1);
  ASSERT_EQ_U(tst,    lisp_string_length(LISP_AS(&str,
                                                 lisp_string_t)), 3u);
  ASSERT_EQ_CSTR(tst, lisp_c_string(&str), "abc");
  ASSERT_EQ_I(tst,    lisp_string_cmp_c_string(LISP_AS(&str, lisp_string_t), "abc"),0);
  ASSERT_LT_I(tst,    lisp_string_cmp_c_string(LISP_AS(&str, lisp_string_t), "def"),0);
  ASSERT_GT_I(tst,    lisp_string_cmp_c_string(LISP_AS(&str, lisp_string_t), "012"),0);
  ASSERT_IS_OK(tst,   lisp_unset_object(vm, &str));
  ASSERT(tst,         LISP_IS_NIL(&str));
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_sprintf(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t str;
  lisp_sprintf(vm, &str, "%d %x %s", 1,0xff, "abc");
  ASSERT(tst,         LISP_IS_OBJECT(&str));
  ASSERT(tst,         LISP_IS_STRING(&str));
  ASSERT_EQ_U(tst,    LISP_REFCOUNT(&str), 1u);
  ASSERT_EQ_CSTR(tst, lisp_c_string(&str), "1 ff abc");
  ASSERT_EQ_U(tst,    lisp_string_length(LISP_AS(&str,
                                                 lisp_string_t)), 8u);
  ASSERT_IS_OK(tst,   lisp_unset_object(vm, &str));
  ASSERT(tst,         LISP_IS_NIL(&str));
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_lisp_string_cmp_c_string(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t str;
  lisp_make_string(vm, &str, "abc");
  ASSERT_EQ_I(tst,    lisp_string_cmp_c_string(LISP_AS(&str, lisp_string_t), "abc"), 0);
  ASSERT_LT_I(tst,    lisp_string_cmp_c_string(LISP_AS(&str, lisp_string_t), "xxx"), 0);
  ASSERT_LT_I(tst,    lisp_string_cmp_c_string(LISP_AS(&str, lisp_string_t), "abcdef"), 0);
  ASSERT_GT_I(tst,    lisp_string_cmp_c_string(LISP_AS(&str, lisp_string_t), "000"), 0);
  ASSERT_GT_I(tst,    lisp_string_cmp_c_string(LISP_AS(&str, lisp_string_t), "ab"), 0);
  ASSERT_IS_OK(tst,   lisp_unset_object(vm, &str));
  lisp_free_vm(vm);
  memcheck_end();
}

static void test_copy_string(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t str, copy1, copy2;
  lisp_make_string(vm, &str, "abc");
  ASSERT_EQ_U(tst,    LISP_REFCOUNT(&str), 1u);
  
  ASSERT_IS_OK(tst,   lisp_copy_object(vm, &copy1, &str));
  ASSERT(tst,         LISP_IS_OBJECT(&copy1));
  ASSERT(tst,         LISP_IS_STRING(&copy1));
  ASSERT_EQ_U(tst,    LISP_REFCOUNT(&str), 2u);
  ASSERT_EQ_U(tst,    LISP_REFCOUNT(&copy1), 2u);
  ASSERT_EQ_PTR(tst,  lisp_c_string(&str), lisp_c_string(&copy1));

  ASSERT_IS_OK(tst,   lisp_copy_object(vm, &copy2, &copy1))
  ASSERT(tst,         LISP_IS_OBJECT(&copy2));
  ASSERT(tst,         LISP_IS_STRING(&copy2));
  ASSERT_EQ_U(tst,    LISP_REFCOUNT(&str), 3u);
  ASSERT_EQ_U(tst,    LISP_REFCOUNT(&copy1), 3u);
  ASSERT_EQ_U(tst,    LISP_REFCOUNT(&copy2), 3u);
  
  ASSERT_EQ_PTR(tst,  lisp_c_string(&str), lisp_c_string(&copy1));
  ASSERT_EQ_PTR(tst,  lisp_c_string(&str), lisp_c_string(&copy2));
  ASSERT_EQ_CSTR(tst, lisp_c_string(&str),   "abc");
  ASSERT_EQ_CSTR(tst, lisp_c_string(&copy1), "abc");
  ASSERT_EQ_CSTR(tst, lisp_c_string(&copy2), "abc");  

  ASSERT_IS_OK(tst,   lisp_unset_object(vm, &str));
  ASSERT(tst,         LISP_IS_NIL(&str));
  ASSERT_EQ_U(tst,    LISP_REFCOUNT(&copy1), 2u);
  ASSERT_EQ_U(tst,    LISP_REFCOUNT(&copy2), 2u);
  ASSERT_EQ_PTR(tst,  lisp_c_string(&copy2), lisp_c_string(&copy1));
  ASSERT_EQ_CSTR(tst, lisp_c_string(&copy1), "abc");

  ASSERT_IS_OK(tst,   lisp_unset_object(vm, &copy2));
  ASSERT(tst,         LISP_REFCOUNT(&copy1) == 1);

  ASSERT_IS_OK(tst,   lisp_copy_object(vm, &copy2, &copy1));

  ASSERT(tst,         LISP_REFCOUNT(&copy1) == 2);
  ASSERT_IS_OK(tst,   lisp_unset_object(vm, &copy1));
  ASSERT_IS_OK(tst,   lisp_unset_object(vm, &copy2));
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_make_substring_full(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t str, substr_full;
  ASSERT_IS_OK(tst,   lisp_make_string(vm, &str, "0123456789"));
  ASSERT_EQ_U(tst,    LISP_REFCOUNT(&str), 1u);
  ASSERT_EQ_I(tst,    lisp_make_substring(vm, 
                                          &substr_full,
                                          LISP_AS(&str, lisp_string_t),
                                          0,
                                          10), LISP_OK);
  ASSERT(tst,         LISP_IS_OBJECT(&substr_full));
  ASSERT(tst,         LISP_IS_STRING(&substr_full));
  ASSERT_EQ_PTR(tst,  lisp_c_string(&str), lisp_c_string(&substr_full));
  ASSERT_EQ_U(tst,    lisp_string_length(LISP_AS(&substr_full,
                                                 lisp_string_t)), 10u);
  ASSERT(tst,         LISP_REFCOUNT(&str) == 1);

  lisp_unset_object(vm, &str);
  lisp_unset_object(vm, &substr_full);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);

  memcheck_end();
}

static void test_make_substring_empty(unit_test_t * tst) 
{
  memcheck_begin();

  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t str;
  lisp_cell_t substr_empty1;
  lisp_cell_t substr_empty2;
  lisp_make_string(vm, &str, "0123456789");
  ASSERT_IS_OK(tst,    lisp_make_substring(vm, 
                                           &substr_empty1,
                                           LISP_AS(&str, lisp_string_t),
                                           10,
                                           10));
  ASSERT_EQ_I(tst,    lisp_string_cmp_c_string(LISP_AS(&substr_empty1, 
                                                       lisp_string_t), ""),0);
  ASSERT_EQ_U(tst,    lisp_string_length(LISP_AS(&substr_empty1,
                                                 lisp_string_t)), 0u);
  ASSERT_EQ_U(tst,    LISP_REFCOUNT(&str), 1u);

  ASSERT_IS_OK(tst,    lisp_make_substring(vm, 
                                           &substr_empty2,
                                           LISP_AS(&str, lisp_string_t),
                                           0,
                                           0));
  ASSERT_EQ_U(tst,    LISP_REFCOUNT(&str), 1u);
  ASSERT_EQ_I(tst,    lisp_string_cmp_c_string(LISP_AS(&substr_empty2, 
                                                       lisp_string_t), ""), 0);
  ASSERT_EQ_U(tst,    lisp_string_length(LISP_AS(&substr_empty2,
                                                 lisp_string_t)), 0u);

  ASSERT_IS_OK(tst,   lisp_unset_object(vm, &str));
  ASSERT_IS_OK(tst,   lisp_unset_object(vm, &substr_empty1));
  ASSERT_IS_OK(tst,   lisp_unset_object(vm, &substr_empty2));

  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_make_substring_begin(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t str, substr;
  ASSERT_IS_OK(tst,    lisp_make_string(vm, &str, "0123456789"));
  ASSERT_IS_OK(tst,    lisp_make_substring(vm, 
                                           &substr,
                                           LISP_AS(&str, lisp_string_t),
                                           0,
                                           4));
  ASSERT(tst,         LISP_IS_OBJECT(&substr));
  ASSERT(tst,         LISP_IS_STRING(&substr));
  ASSERT_EQ_U(tst,    LISP_REFCOUNT(&str), 1u);
  ASSERT_EQ_U(tst,    LISP_REFCOUNT(&substr), 1u);
  ASSERT_EQ_PTR(tst,  lisp_c_string(&str), lisp_c_string(&substr));
  ASSERT_EQ_I(tst,    lisp_string_cmp_c_string(LISP_AS(&substr,
                                                       lisp_string_t), 
                                               "0123"), 0);
  ASSERT_LT_I(tst,    lisp_string_cmp_c_string(LISP_AS(&substr,
                                                       lisp_string_t),
                                               "01234"),0);
  ASSERT_LT_I(tst,    lisp_string_cmp_c_string(LISP_AS(&substr,
                                                       lisp_string_t),
                                               "1234"), 0);
  ASSERT_GT_I(tst,    lisp_string_cmp_c_string(LISP_AS(&substr,
                                                       lisp_string_t),
                                               "0000"), 0);
  ASSERT_GT_I(tst,    lisp_string_cmp_c_string(LISP_AS(&substr,
                                                       lisp_string_t),
                                               "012"), 0);
  ASSERT_IS_OK(tst,   lisp_unset_object(vm, &str));
  ASSERT_IS_OK(tst,   lisp_unset_object(vm, &substr));

  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_make_substring_bulk(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t str, substr;
  lisp_make_string(vm, &str, "0123456789");
  ASSERT_IS_OK(tst,    lisp_make_substring(vm, 
                                           &substr,
                                           LISP_AS(&str, lisp_string_t),
                                           2,
                                           4));

  ASSERT_EQ_I(tst,    lisp_string_cmp_c_string(LISP_AS(&substr,
                                                       lisp_string_t), 
                                               "23"),
              0);
  ASSERT_LT_I(tst,    lisp_string_cmp_c_string(LISP_AS(&substr,
                                                       lisp_string_t),
                                               "234"),
              0);
  ASSERT_LT_I(tst,    lisp_string_cmp_c_string(LISP_AS(&substr,
                                                       lisp_string_t),
                                               "34"),
              0);
  ASSERT_GT_I(tst,    lisp_string_cmp_c_string(LISP_AS(&substr,
                                                       lisp_string_t),
                                               "00"),
              0);
  ASSERT_GT_I(tst,    lisp_string_cmp_c_string(LISP_AS(&substr,
                                                       lisp_string_t),
                                               "01"),
              0);

  ASSERT_IS_OK(tst,   lisp_unset_object(vm, &str));
  ASSERT_IS_OK(tst,   lisp_unset_object(vm, &substr));

  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_make_substring_end(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t str, substr;
  lisp_make_string(vm, &str, "0123456789");
  ASSERT_IS_OK(tst,    lisp_make_substring(vm, 
                                           &substr,
                                           LISP_AS(&str, lisp_string_t),
                                           4,
                                           10));

  ASSERT_EQ_I(tst,    lisp_string_cmp_c_string(LISP_AS(&substr,
                                                       lisp_string_t), 
                                               "456789"),
              0);
  ASSERT_LT_I(tst,    lisp_string_cmp_c_string(LISP_AS(&substr,
                                                       lisp_string_t),
                                               "4567890"),
              0);
  ASSERT_LT_I(tst,    lisp_string_cmp_c_string(LISP_AS(&substr,
                                                       lisp_string_t),
                                               "5678901"),
              0);
  ASSERT_GT_I(tst,    lisp_string_cmp_c_string(LISP_AS(&substr,
                                                       lisp_string_t),
                                               "4567"),
              0);
  ASSERT_GT_I(tst,    lisp_string_cmp_c_string(LISP_AS(&substr,
                                                       lisp_string_t),
                                               "345678"),
              0);

  ASSERT_IS_OK(tst,   lisp_unset_object(vm, &str));
  ASSERT_IS_OK(tst,   lisp_unset_object(vm, &substr));

  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_make_substring_range_errors(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t str, substr;
  lisp_make_string(vm, &str, "0123456789");
  ASSERT_IS_RANGE_ERROR(tst,    lisp_make_substring(vm, 
                                                    &substr,
                                                    LISP_AS(&str, lisp_string_t),
                                                    5,
                                                    4));
  /* @todo test that substr is expception */
  ASSERT_IS_OK(tst,             lisp_unset_object(vm, &substr));

  ASSERT_IS_RANGE_ERROR(tst,    lisp_make_substring(vm, 
                                                    &substr,
                                                    LISP_AS(&str, lisp_string_t),
                                                    11,
                                                    14));
  /* @todo test that substr is expception */
  ASSERT_IS_OK(tst,             lisp_unset_object(vm, &substr));

  ASSERT_IS_RANGE_ERROR(tst,    lisp_make_substring(vm, 
                                                    &substr,
                                                    LISP_AS(&str, lisp_string_t),
                                                    4,
                                                    14));
  /* @todo test that substr is expception */
  ASSERT_IS_OK(tst,             lisp_unset_object(vm, &substr));

  ASSERT_IS_OK(tst,             lisp_unset_object(vm, &str));
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

void test_string(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "string");

  TEST(suite, test_create_string);
  TEST(suite, test_lisp_string_cmp_c_string);
  TEST(suite, test_sprintf);
  TEST(suite, test_copy_string);
  TEST(suite, test_make_substring_full);
  TEST(suite, test_make_substring_empty);
  TEST(suite, test_make_substring_begin);
  TEST(suite, test_make_substring_bulk);
  TEST(suite, test_make_substring_end);
  TEST(suite, test_make_substring_range_errors);
}
