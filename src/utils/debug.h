#ifndef UTILS_ASSERT_H
#define UTILS_ASSERT_H

void oh_panic(const char* file, const char* func, int line);

#define PANIC() oh_panic(__FILE__, __FUNCTION__, __LINE__)

#ifdef NDEBUG
#define ASSERT(CONDITION) ((void)0)
#else
#define ASSERT(CONDITION)  \
    if (CONDITION) {} else { PANIC(); }
#endif // NDEBUG

#endif  // UTILS_ASSERT_H
