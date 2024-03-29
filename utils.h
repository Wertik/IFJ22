/*
 * Project: IFJ22 language compiler
 *
 * @author xotrad00 Martin Otradovec
 * @author xdobes22 Kristián Dobeš
 */

#ifndef _UTILS_H
#define _UTILS_H

#include <string.h>

#define FAIL_LEXICAL 1
#define FAIL_SYNTAX 2
#define FAIL_SEMANTIC_FUNC_DEF 3
#define FAIL_SEMANTIC_BAD_ARGS 4
#define FAIL_SEMANTIC_VAR_UNDEFINED 5
#define FAIL_SEMANTIC_INVALID_RETURN_COUNT 6
#define FAIL_SEMANTIC_INVALID_RETURN_TYPE 4
#define FAIL_SEMANTIC_EXPRE 7
#define FAIL_SEMANTIC 8
#define FAIL_INTERNAL 99

#define NULL_FALSE(val) (val == NULL ? "0" : val)

#define LOG_ENABLED(log_type) (strcmp(NULL_FALSE(getenv("LOG_" #log_type)), "1") == 0 || strcmp(NULL_FALSE(getenv("LOG_ALL")), "1") == 0)

#define MALLOC_CHECK(var)                                   \
    do                                                      \
    {                                                       \
        if (var == NULL)                                    \
        {                                                   \
            fprintf(stderr, "%s: malloc fail\n", __func__); \
            exit(FAIL_INTERNAL);                            \
        }                                                   \
    } while (0);

#define DEBUG_PSEUDO(...) \
    do                            \
    {                             \
        if (LOG_ENABLED(PSEUDO))  \
        {                         \
            printf("PSEUDO: ");   \
            printf(__VA_ARGS__);  \
            printf("\n");         \
        }                         \
    } while (0);

#define DEBUG_STATE(curr, state)                    \
    do                                              \
    {                                               \
        if (LOG_ENABLED(STATE))                     \
        {                                           \
            printf("STATE: %d->%d\n", curr, state); \
        }                                           \
    } while (0);

#ifdef VERBOSE
#define DEBUG(...)               \
    do                           \
    {                            \
        if (LOG_ENABLED(DEBUG))  \
        {                        \
            printf("DEBUG: ");   \
            printf(__VA_ARGS__); \
            printf("\n");        \
        }                        \
    } while (0);

#define DEBUG_ASSERT(expected_type, token)                                                                                               \
    do                                                                                                                                   \
    {                                                                                                                                    \
        if (LOG_ENABLED(ASSERT))                                                                                                         \
            printf("ASSERT: %s is %s\n\n", token == NULL ? "null" : token_type_to_name(token->type), token_type_to_name(expected_type)); \
    } while (0);

#define DEBUG_RULE()                              \
    do                                            \
    {                                             \
        if (LOG_ENABLED(RULE))                    \
            printf("\n->RULE: %s\n\n", __func__); \
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

#define DEBUG(...) \
    do             \
    {              \
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