#include "util/unit_test.h"
#include "util/mock.h"
#include <stdio.h>


static int mocked_int_function1()
{
  MOCK_CALL(mocked_int_function1, int);
  return -1;
}


static int mocked_int_function2()
{
  MOCK_CALL(mocked_int_function2, int);
  return -2;
}


static int mocked_int_function3()
{
  MOCK_CALL(mocked_int_function3, int);
  return -3;
}


static void mocked_void_function1()
{
  MOCK_CALL(mocked_void_function1, void);
}


static void mocked_void_function2()
{
  MOCK_CALL(mocked_void_function2, void);
}

static int mockup_1(void * user_data)
{
  if(user_data != NULL) 
  {
    *((int*) user_data) = 1;
  }
  return 1;
}

static int mockup_2(void * user_data)
{
  if(user_data != NULL) 
  {
    *((int*) user_data) = 2;
  }
  return 2;
}

static int mockup_3(void * user_data)
{
  if(user_data != NULL) 
  {
    *((int*) user_data) = 3;
  }
  return 3;
}

static int mockup_4(void * user_data)
{
  if(user_data != NULL) 
  {
    *((int*) user_data) = 4;
  }
  return 4;
}

static int mockup_5(void * user_data)
{
  if(user_data != NULL) 
  {
    *((int*) user_data) = 5;
  }
  return 5;
}

static void mockup_void(void * user_data)
{
  if(user_data != NULL) 
  {
    *((int*) user_data) = 6;
  }
}


static void test_mock_get_expected_unregistered(unit_test_t * tst)
{
  ASSERT_EQ_PTR(tst, NULL, mock_get_expected(mocked_int_function1));
  return;
}

static void test_retire(void * user_data)
{
  if(user_data != NULL) 
  {
    *((int*)user_data) *= -1;
  }
}

static void test_mock_register(unit_test_t * tst)
{
  mock_expected_t * expected[25];
  /* [ 0 ] */
  expected[0] = mock_register(mocked_int_function1, mockup_1, NULL, NULL);
  ASSERT_EQ_PTR(tst, expected[0], mock_get_expected(mocked_int_function1));
  ASSERT_EQ_PTR(tst, expected[0]->prev, NULL);
  ASSERT_EQ_PTR(tst, expected[0]->next, NULL);

  /* [ 0 ], [1] */
  expected[1] = mock_register(mocked_int_function2, mockup_1, NULL, NULL);
  ASSERT_EQ_PTR(tst,  expected[0], mock_get_expected(mocked_int_function1));
  ASSERT_EQ_PTR(tst,  expected[1], mock_get_expected(mocked_int_function2));
  ASSERT_NEQ_PTR(tst, expected[0]->parent, expected[1]->parent);
  ASSERT_EQ_PTR(tst,  expected[0]->prev, NULL);
  ASSERT_EQ_PTR(tst,  expected[0]->next, NULL);
  ASSERT_EQ_PTR(tst,  expected[1]->prev, NULL);
  ASSERT_EQ_PTR(tst,  expected[1]->next, NULL);
  
  /* [ 0,2 ], [1] */
  expected[2] = mock_register(mocked_int_function1, mockup_1, NULL, NULL);
  ASSERT_EQ_PTR(tst,  expected[0], mock_get_expected(mocked_int_function1));
  ASSERT_EQ_PTR(tst,  expected[1], mock_get_expected(mocked_int_function2));
  ASSERT_NEQ_PTR(tst, expected[0]->parent, expected[1]->parent);
  ASSERT_EQ_PTR(tst,  expected[0]->parent, expected[2]->parent);
  ASSERT_NEQ_PTR(tst, expected[1]->parent, expected[2]->parent);
  ASSERT_EQ_PTR(tst,  expected[0]->prev, NULL);
  ASSERT_EQ_PTR(tst,  expected[0]->next, expected[2]);
  ASSERT_EQ_PTR(tst,  expected[2]->prev, expected[0]);
  ASSERT_EQ_PTR(tst,  expected[2]->next, NULL);
  ASSERT_EQ_PTR(tst,  expected[1]->prev, NULL);
  ASSERT_EQ_PTR(tst,  expected[1]->next, NULL);

  /* [ 0,2 ], [1, 3] */
  expected[3] = mock_register(mocked_int_function2, mockup_1, NULL, NULL);
  ASSERT_EQ_PTR(tst,  expected[0], mock_get_expected(mocked_int_function1));
  ASSERT_EQ_PTR(tst,  expected[1], mock_get_expected(mocked_int_function2));
  ASSERT_NEQ_PTR(tst, expected[0]->parent, expected[1]->parent);
  ASSERT_EQ_PTR(tst,  expected[0]->parent, expected[2]->parent);
  ASSERT_NEQ_PTR(tst, expected[1]->parent, expected[2]->parent);
  ASSERT_EQ_PTR(tst,  expected[1]->parent, expected[3]->parent);
  ASSERT_EQ_PTR(tst,  expected[0]->prev, NULL);
  ASSERT_EQ_PTR(tst,  expected[0]->next, expected[2]);
  ASSERT_EQ_PTR(tst,  expected[2]->prev, expected[0]);
  ASSERT_EQ_PTR(tst,  expected[2]->next, NULL);
  ASSERT_EQ_PTR(tst,  expected[1]->prev, NULL);
  ASSERT_EQ_PTR(tst,  expected[1]->next, expected[3]);
  ASSERT_EQ_PTR(tst,  expected[3]->prev, expected[1]);
  ASSERT_EQ_PTR(tst,  expected[3]->next, NULL);

  ASSERT_EQ_U(tst, 2u, mock_retire_all());
}

static void test_mock_remove(unit_test_t * tst)
{
  mock_expected_t * expected[5];
  size_t i;
  for(i = 0; i < 5; i++) 
  {
    expected[i] = mock_register(mocked_int_function1, mockup_1, NULL, NULL);
  }
  ASSERT_EQ_PTR(tst, expected[0]->prev, NULL);
  ASSERT_EQ_PTR(tst, expected[0]->next, expected[1]);
  for(i = 1; i < 4; i++) 
  {
    ASSERT_EQ_PTR(tst, expected[i]->prev,   expected[i-1]);
    ASSERT_EQ_PTR(tst, expected[i]->next,   expected[i+1]);
    ASSERT_EQ_PTR(tst, expected[i]->parent, expected[i-1]->parent);
  }
  ASSERT_EQ_PTR(tst, expected[4]->prev,   expected[3]);
  ASSERT_EQ_PTR(tst, expected[4]->next,   NULL);
  ASSERT_EQ_PTR(tst, expected[4]->parent, expected[3]->parent);

  /* remove first */
  mock_remove(expected[0]);
  ASSERT_EQ_PTR(tst, mock_get_expected(mocked_int_function1), expected[1]);
  ASSERT_EQ_PTR(tst, expected[1]->prev,  NULL);
  ASSERT_EQ_PTR(tst, expected[1]->next,  expected[2]);
  ASSERT_EQ_PTR(tst, expected[3]->prev,  expected[2]);
  ASSERT_EQ_PTR(tst, expected[3]->next,  expected[4]);

  /* remove last */
  mock_remove(expected[4]);
  ASSERT_EQ_PTR(tst, mock_get_expected(mocked_int_function1), expected[1]);
  ASSERT_EQ_PTR(tst, expected[1]->prev, NULL);
  ASSERT_EQ_PTR(tst, expected[1]->next, expected[2]);
  ASSERT_EQ_PTR(tst, expected[3]->prev, expected[2]);
  ASSERT_EQ_PTR(tst, expected[3]->next, NULL);

  /* remove bulk */
  mock_remove(expected[2]);
  ASSERT_EQ_PTR(tst, mock_get_expected(mocked_int_function1), expected[1]);
  ASSERT_EQ_PTR(tst, expected[1]->prev, NULL);
  ASSERT_EQ_PTR(tst, expected[1]->next, expected[3]);
  ASSERT_EQ_PTR(tst, expected[3]->prev, expected[1]);
  ASSERT_EQ_PTR(tst, expected[3]->next, NULL);

  /* remove last */
  mock_remove(expected[3]);
  ASSERT_EQ_PTR(tst, expected[1], mock_get_expected(mocked_int_function1));
  ASSERT_EQ_PTR(tst, expected[1]->prev, NULL);
  ASSERT_EQ_PTR(tst, expected[1]->next, NULL);  
  /* remove remaining */
  mock_remove(expected[1]);
  ASSERT_EQ_PTR(tst, mock_get_expected(mocked_int_function1), NULL);
}

static void test_mock_remove_different_functions(unit_test_t * tst)
{
  mock_expected_t * expected[5][5];
  size_t i,j;
  for(i = 0; i < 5; i++) 
  {
    expected[0][i] = mock_register(mocked_int_function1, mockup_1, 
				   NULL, NULL);
  }
  for(i = 0; i < 5; i++) 
  {
    expected[1][i] = mock_register(mocked_int_function2, mockup_1, 
				   NULL, NULL);
  }
  for(i = 0; i < 5; i++) 
  {
    expected[2][i] = mock_register(mocked_int_function3, mockup_1, 
				   NULL, NULL);
  }
  for(i = 0; i < 5; i++) 
  {
    expected[3][i] = mock_register(mocked_void_function1, mockup_void, 
				   NULL, NULL);
  }
  for(i = 0; i < 5; i++) 
  {
    expected[4][i] = mock_register(mocked_void_function2, mockup_void, 
				   NULL, NULL);
  }
  for(j = 0; j < 5; j++) 
  {
    ASSERT_EQ_PTR(tst, expected[j][0]->prev, NULL);
    ASSERT_EQ_PTR(tst, expected[j][0]->next, expected[j][1]);
    for(i = 1; i < 4; i++) 
    {
      ASSERT_EQ_PTR(tst, expected[j][i]->prev, expected[j][i-1]);
      ASSERT_EQ_PTR(tst, expected[j][i]->next, expected[j][i+1]);
    }
    ASSERT_EQ_PTR(tst, expected[j][4]->prev, expected[j][3]);
    ASSERT_EQ_PTR(tst, expected[j][4]->next, NULL);
  }
  /* remove first */
  mock_retire(mocked_int_function1);
  ASSERT_EQ_PTR(tst, mock_get_expected(mocked_int_function1), NULL);

  /* remove last */
  mock_retire(mocked_void_function2);
  ASSERT_EQ_PTR(tst, mock_get_expected(mocked_void_function2), NULL);
  
  /* remove bulk */
  mock_expected_t * current = mock_get_expected(mocked_int_function3);
  size_t nelem = 0;
  while(current) 
  {
    mock_remove(current);
    current = mock_get_expected(mocked_int_function3);
    nelem++;
  }
  ASSERT_EQ_U(tst, nelem, 5);

  ASSERT_EQ_U(tst, 2u, mock_retire_all());
}

static void test_mock_calls(unit_test_t * tst)
{
  ASSERT_EQ_I(tst, mocked_int_function1(), -1);
  ASSERT_EQ_I(tst, mocked_int_function2(), -2);
  ASSERT_EQ_I(tst, mocked_int_function3(), -3);
  ASSERT_EQ_I(tst, mockup_1(NULL), 1);
  ASSERT_EQ_I(tst, mockup_2(NULL), 2);
  ASSERT_EQ_I(tst, mockup_3(NULL), 3);
  ASSERT_EQ_I(tst, mockup_4(NULL), 4);
  ASSERT_EQ_I(tst, mockup_5(NULL), 5);
  int user_data1 = 0;
  int user_data2 = 0;
  int user_data3 = 0;
  int user_data4 = 0;
  int user_data5 = 0;
  int user_data6 = 0;

  mock_register(mocked_int_function1,  mockup_1,    &user_data1, NULL);
  mock_register(mocked_int_function1,  NULL,        NULL,        NULL);
  mock_register(mocked_int_function1,  mockup_2,    NULL,        NULL);
  mock_register(mocked_int_function2,  mockup_3,    &user_data2, NULL);
  mock_register(mocked_int_function2,  mockup_4,    &user_data3, test_retire);
  mock_register(mocked_int_function2,  NULL,        &user_data4, NULL);
  mock_register(mocked_int_function1,  mockup_5,    NULL, NULL);
  mock_register(mocked_void_function1, mockup_void, &user_data5, test_retire);
  mock_register(mocked_void_function2, mockup_void, NULL,        NULL);
  mock_register(mocked_void_function2, mockup_void, &user_data6, test_retire);
  ASSERT_EQ_I(tst, mocked_int_function1(), 1);
  ASSERT_EQ_I(tst, mocked_int_function1(), -1);
  ASSERT_EQ_I(tst, mocked_int_function1(), 2);
  ASSERT_EQ_I(tst, mocked_int_function1(), 5);
  ASSERT_EQ_I(tst, mocked_int_function2(), 3);
  ASSERT_EQ_I(tst, mocked_int_function2(), 4);
  ASSERT_EQ_I(tst, mocked_int_function2(), -2);
  mocked_void_function1();
  mocked_void_function2();
  mocked_void_function2();
  ASSERT_EQ_I(tst, user_data1, 1);
  ASSERT_EQ_I(tst, user_data2, 3);
  ASSERT_EQ_I(tst, user_data3, 4);
  ASSERT_EQ_I(tst, user_data5, 6);
  ASSERT_EQ_I(tst, user_data6, 6);
  ASSERT_EQ_U(tst, 0u, mock_retire_all());
}

static void test_mock_retire(unit_test_t * tst)
{
  int user_data[9];
  size_t i;
  for(i = 0; i < 9; i++) 
  {
    user_data[i] = i+1;
  }
  mock_register(mocked_int_function1,  mockup_1,    &user_data[0], test_retire);
  mock_register(mocked_int_function1,  NULL,        &user_data[1], test_retire);
  mock_register(mocked_int_function1,  mockup_2,    &user_data[2], NULL);

  mock_register(mocked_int_function2,  mockup_1,    &user_data[3], test_retire);
  mock_register(mocked_int_function2,  NULL,        &user_data[4], test_retire);
  mock_register(mocked_int_function2,  mockup_2,    &user_data[5], NULL);

  mock_register(mocked_int_function3,  mockup_1,    &user_data[6], test_retire);
  mock_register(mocked_int_function3,  NULL,        &user_data[7], test_retire);
  mock_register(mocked_int_function3,  mockup_2,    &user_data[8], NULL);

  mock_retire(mocked_int_function1);
  ASSERT_EQ_I(tst, user_data[0], -1);
  ASSERT_EQ_I(tst, user_data[1], -2);
  ASSERT_EQ_I(tst, user_data[2], 3);
  ASSERT_EQ_U(tst, mock_retire_all(), 2);
  ASSERT_EQ_I(tst, user_data[3], -4);
  ASSERT_EQ_I(tst, user_data[4], -5);
  ASSERT_EQ_I(tst, user_data[5], 6);
  ASSERT_EQ_I(tst, user_data[6], -7);
  ASSERT_EQ_I(tst, user_data[7], -8);
  ASSERT_EQ_I(tst, user_data[8], 9);
}

static void test_mock_retire_unused(unit_test_t * tst)
{
  int user_data = 1;
  mock_expected_t * registered, * expected;
  registered = mock_register(mocked_int_function1,  mockup_1, 
			     &user_data, test_retire);
  expected = mock_get_expected(mocked_int_function1);
  ASSERT_EQ_PTR(tst, registered, expected);
  mock_remove(expected);
  ASSERT_EQ_I(tst, user_data, -1);
  ASSERT_EQ_U(tst, mock_retire_all(), 0u);
}

void test_mock(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "mock");
  TEST(suite, test_mock_get_expected_unregistered);
  TEST(suite, test_mock_register);
  TEST(suite, test_mock_remove);
  TEST(suite, test_mock_remove_different_functions);
  TEST(suite, test_mock_calls);
  TEST(suite, test_mock_retire);
  TEST(suite, test_mock_retire_unused);
}
