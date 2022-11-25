#ifndef UTILS_H
#define UTILS_H

#include <string.h>

#define NULL_FALSE(val) (val == NULL ? "0" : val)

#define LOG_ENABLED(log_type) (strcmp(NULL_FALSE(getenv("LOG_" #log_type)), "1") == 0 || strcmp(NULL_FALSE(getenv("LOG_ALL")), "1") == 0)

#define MALLOC_CHECK(var)                                   \
    do                                                      \
    {                                                       \
        if (var == NULL)                                    \
        {                                                   \
            fprintf(stderr, "%s: malloc fail\n", __func__); \
            exit(99);                                       \
        }                                                   \
    } while (0);

#define DEBUG_PSEUDO(str)                 \
    do                                    \
    {                                     \
        if (LOG_ENABLED(PSEUDO))          \
            printf("PSEUDO: " #str "\n"); \
    } while (0);

#define DEBUG_PSEUDOF(fmt, ...)                        \
    do                                                 \
    {                                                  \
        if (LOG_ENABLED(PSEUDO))                       \
            printf("PSEUDO: " #fmt "\n", __VA_ARGS__); \
    } while (0);

#ifdef VERBOSE
#define DEBUG(format, ...)                          \
    do                                              \
    {                                               \
        if (LOG_ENABLED(DEBUG))                     \
            printf("DEBUG: "##format, __VA_ARGS__); \
    } while (0);

#define DEBUG_ASSERT(expected, actual)                                                                \
    do                                                                                                \
    {                                                                                                 \
        if (LOG_ENABLED(ASSERT))                                                                      \
            printf("\nASSERT: %s is %s\n", token_type_to_name(actual), token_type_to_name(expected)); \
    } while (0);

#define DEBUG_RULE()                            \
    do                                          \
    {                                           \
        if (LOG_ENABLED(RULE))                  \
            printf("\n->RULE: %s\n", __func__); \
    } while (0);

#define DEBUG_STACK(stack_ptr)      \
    do                              \
    {                               \
        if (LOG_ENABLED(STACK))     \
        {                           \
            printf("\nSTACK: ");    \
            stack_print(stack_ptr); \
        }                           \
    } while (0);

#define DEBUG_ARRAY(array_ptr)      \
    do                              \
    {                               \
        if (LOG_ENABLED(ARRAY))     \
        {                           \
            printf("\nARRAY: ");    \
            array_print(array_ptr); \
        }                           \
    } while (0);

#define DEBUG_STACK_TOP(stack_ptr, n)                                \
    do                                                               \
    {                                                                \
        if (LOG_ENABLED(STACK_TOP))                                  \
        {                                                            \
            printf("STACK TOP: stack(%d) [", stack_size(stack_ptr)); \
            item_ptr item = stack_top(stack_ptr);                    \
            for (int i = 0; i < n && item != NULL; i++)              \
            {                                                        \
                char *s = symbol_to_string(item->symbol);            \
                printf("%s ", s);                                    \
                free(s);                                             \
                item = item->next;                                   \
            }                                                        \
            printf("...]\n");                                        \
        }                                                            \
    } while (0);

#define DEBUG_TOKEN(token_ptr)      \
    do                              \
    {                               \
        if (LOG_ENABLED(TOKEN))     \
        {                           \
            printf("TOKEN: ");      \
            token_print(token_ptr); \
        }                           \
    } while (0);

#else
#define UNUSED(x) (void)x

#define DEBUG(fmt, ...) \
    do                  \
    {                   \
        UNUSED(fmt);    \
    } while (0);

#define DEBUG_ASSERT(expected, actual) \
    do                                 \
    {                                  \
        UNUSED(expected);              \
        UNUSED(actual);                \
    } while (0);

#define DEBUG_RULE() \
    do               \
    {                \
    } while (0);

#define DEBUG_STACK(stack_ptr) \
    do                         \
    {                          \
        UNUSED(stack_ptr);     \
    } while (0);

#define DEBUG_STACK_TOP(stack_ptr, n) \
    do                                \
    {                                 \
        UNUSED(stack_ptr);            \
        UNUSED(n);                    \
    } while (0);

#define DEBUG_ARRAY(array_ptr) \
    do                         \
    {                          \
        UNUSED(array_ptr);     \
    } while (0);

#define DEBUG_TOKEN(token_ptr) \
    do                         \
    {                          \
        UNUSED(token_ptr);     \
    } while (0);
#endif

#endif