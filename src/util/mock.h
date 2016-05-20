#ifndef __MOCK_H__
#define __MOCK_H__
#include <stdlib.h>

#ifdef MOCK
#define MOCK_CALL(FUNC, RET_TYPE)				\
{								\
  mock_expected_t * expected = mock_get_expected((FUNC));	\
  if(expected != NULL)						\
  {								\
    if(expected->call_back != NULL)				\
    {								\
      typedef RET_TYPE(*callback_t)(void * user_data);		\
      callback_t cb = (callback_t)expected->call_back;		\
      void * user_data= expected->user_data;			\
      expected->retire_func = NULL;				\
      mock_remove(expected);					\
      return cb(user_data);					\
    }								\
    else							\
    {								\
      mock_remove(expected);					\
    }								\
  }								\
}
#else
#define MOCK_CALL(FUNC, RET_TYPE)
#endif

typedef void(*mock_user_data_retire_t)(void * user_data);

typedef struct mock_expected_t
{
  void                    * func;
  void                    * call_back;
  void                    * user_data;
  mock_user_data_retire_t   retire_func;
  struct mock_expected_t  * next;
  struct mock_expected_t  * prev;
  void * parent;
} mock_expected_t;


/*
 * user_data is either free'd by mock_function or retire_func, or not
 * at all.
 */
mock_expected_t * mock_register(void * func, 
				void * mock_function,
				void * user_data, 
				mock_user_data_retire_t retire_func);
mock_expected_t * mock_get_expected(void * func);
void mock_remove(mock_expected_t * ret);
size_t mock_retire(void * func);
size_t mock_retire_all();

#endif
