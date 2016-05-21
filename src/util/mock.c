#include "mock.h"

typedef struct mock_function_t 
{
  struct mock_function_t * next;
  struct mock_function_t * prev;
  mock_expected_t * first;
  mock_expected_t * last;
} mock_function_t;

mock_function_t * __first_func;
mock_function_t * __last_func;

static mock_function_t * _mock_find_or_insert(void * func);
static mock_function_t * _mock_find(void * func);
static size_t _mock_retire_func(mock_function_t * func);

static mock_function_t * _mock_find_or_insert(void * func) 
{
  mock_function_t * current = __first_func;
  while(current != NULL) 
  {
    if(current->first && current->first->func == func)
    {
      return current;
    }
    current = current->next;
  }
  current = malloc(sizeof(mock_function_t));
  current->first = NULL;
  current->last = NULL;
  current->prev = __last_func;
  current->next = NULL;
  if(current->prev != NULL) 
  {
    current->prev->next = current;
  }
  else 
  {
    __first_func = current;
  }
  __last_func = current;
  return current;
}

static mock_function_t * _mock_find(void * func)
{
  mock_function_t * current = __first_func;
  while(current != NULL) 
  {
    if(current->first && current->first->func == func)
    {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

static size_t _mock_retire_func(mock_function_t * func)
{
  size_t n = 0;
  while(func->first != NULL) 
  {
    mock_expected_t * nexte = func->first->next;
    if(func->first->retire_func) 
    {
      func->first->retire_func(func->first->user_data);
    }
    free(func->first);
    func->first = nexte;
    n++;
  }
  if(func->prev != NULL) 
  {
    func->prev->next = func->next;
  }
  else 
  {
    __first_func = func->next;
  }
  if(func->next != NULL) 
  {
    func->next->prev = func->prev;
  }
  else 
  {
    __last_func = func->prev;
  }
  free(func);
  return n;
}

mock_expected_t * mock_register(void *                  func, 
				void *                  mock_function,
				void *                  user_data,
				mock_user_data_retire_t retire_func)
{
  mock_function_t * mockfunc = _mock_find_or_insert(func);
  mock_expected_t * expected = malloc(sizeof(mock_expected_t));
  expected->func        = func;
  expected->call_back   = mock_function;
  expected->user_data   = user_data;
  expected->retire_func = retire_func;
  expected->parent      = mockfunc;
  expected->prev        = mockfunc->last;
  expected->next        = NULL;
  if(expected->prev != NULL) 
  {
    expected->prev->next = expected;
  }
  else 
  {
    mockfunc->first = expected;
  }
  mockfunc->last = expected;
  return expected;
}

mock_expected_t * mock_get_expected(void * func)
{
  mock_function_t * mockfunc = _mock_find(func);
  if(mockfunc == NULL) 
  {
    return NULL;
  }
  else 
  {
    return mockfunc->first;
  }
}

void mock_remove(mock_expected_t * item)
{
  if(item->parent) 
  {
    mock_function_t * mockfunc = (mock_function_t *) item->parent;
    if(item->prev) 
    {
      item->prev->next = item->next;
    }
    else 
    {
      mockfunc->first = item->next;
    }
    if(item->next) 
    {
      item->next->prev = item->prev;
    }
    else 
    {
      mockfunc->last = item->prev;
    }
    if(item->retire_func) 
    {
      item->retire_func(item->user_data);
    }
    free(item);
    if(mockfunc->last == NULL) 
    {
      if(mockfunc->prev) 
      {
	mockfunc->prev->next = mockfunc->next;
      }
      else 
      {
	__first_func = mockfunc->next;
      }
      if(mockfunc->next) 
      {
	mockfunc->next->prev = mockfunc->prev;
      }
      else
      {
	__last_func = mockfunc->prev;
      }
      free(mockfunc);
    }
  }
}

size_t mock_retire(void * func)
{
  mock_function_t * mockfunc = _mock_find(func);
  if(mockfunc) 
  {
    return _mock_retire_func(mockfunc);
  }
  return 0;
}

size_t mock_retire_all()
{

  size_t n = 0;
  while(__first_func != NULL) 
  {
    _mock_retire_func(__first_func);
    n++;
  }
  __first_func = NULL;
  __last_func = NULL;
  return n;
}

int mock_int_return(void * user_data)
{
  return *((int*)user_data);
}
