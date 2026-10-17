#ifndef RELEASE_ASSERT_H_
#define RELEASE_ASSERT_H_

#ifdef __cplusplus
#include <cassert>
#include <cstdlib>
#else
#include <assert.h>
#include <stdlib.h>
#endif

#ifdef assert
#undef assert
#endif

#ifdef __cplusplus
#define assert(expr) ((expr) ? (void)0 : std::abort())
#else
#define assert(expr) ((expr) ? (void)0 : abort())
#endif

#endif /* RELEASE_ASSERT_H_ */
