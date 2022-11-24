#ifndef UTILS_H
#define UTILS_H

#define MALLOC_CHECK(var)                                \
    do                                                   \
    {                                                    \
        if (var == NULL)                                 \
        {                                                \
            fprintf(stderr, "%s: malloc fail\n", __func__); \
            exit(99);                                    \
        }                                                \
    } while (0);

// TODO: Remove after generator done. Kept here so we can see some output even with debug disabled.
#define DEBUG_OUT(str)                \
    do                                \
    {                                 \
        printf("OUTPUT: " #str "\n"); \
    } while (0);

#define DEBUG_OUTF(fmt, ...)                       \
    do                                             \
    {                                              \
        printf("OUTPUT: " #fmt "\n", __VA_ARGS__); \
    } while (0);

#ifdef VERBOSE
#define DEBUG(format, ...)                      \
    do                                          \
    {                                           \
        printf("DEBUG: "##format, __VA_ARGS__); \
    } while (0);

#define DEBUG_ASSERT(expected, actual)                                                            \
    do                                                                                            \
    {                                                                                             \
        printf("\nASSERT: %s is %s\n", token_type_to_name(actual), token_type_to_name(expected)); \
    } while (0);

#define DEBUG_RULE()                        \
    do                                      \
    {                                       \
        printf("\n->RULE: %s\n", __func__); \
    } while (0);

#define DEBUG_STACK(stack_ptr)  \
    do                          \
    {                           \
        printf("\nDEBUG: ");    \
        stack_print(stack_ptr); \
    } while (0);

#define DEBUG_ARRAY(array_ptr)  \
    do                          \
    {                           \
        array_print(array_ptr); \
    } while (0);

#define DEBUG_STACK_TOP(stack_ptr, n)                            \
    do                                                           \
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
    } while (0);

#define DEBUG_TOKEN(token_ptr)  \
    do                          \
    {                           \
        printf("TOKEN: ");      \
        token_print(token_ptr); \
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