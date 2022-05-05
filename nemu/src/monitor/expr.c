#include "nemu.h"
#include "cpu/reg.h"
#include "memory/memory.h"

#include <stdlib.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

#define MAX_TOKEN_STR_LEN 32
#define MAX_TOKEN_NUM 32

enum
{
	NOTYPE = 256,
	EQ,
	NUM,
	REG,
	SYMB

	/* TODO: Add more token types */

};

static struct rule
{
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +", NOTYPE}, // white space
	{"[0-9]{1,10}", NUM},
	{"\\+", '+'},
	{"-", '-'},
	{"\\*", '*'},
	{"\\(", '('},
	{"\\)", ')'},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX];

/* Rules are used for more times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
	int i;
	char error_msg[128];
	int ret;

	for (i = 0; i < NR_REGEX; i++)
	{
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if (ret != 0)
		{
			regerror(ret, &re[i], error_msg, 128);
			assert(ret != 0);
		}
	}
}

typedef struct
{
	int type;
	char str[MAX_TOKEN_STR_LEN];
} Token;

Token tokens[MAX_TOKEN_NUM];
int nr_token;

static bool make_token(char *e)
{
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

	while (e[position] != '\0')
	{
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i++)
		{
			if (nr_token >= MAX_TOKEN_NUM)
			{
				printf("Exceeded max token number!\n");
				return false;
			}
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
			{
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				printf("match regex[%d] at position %d with len %d: %.*s\n", i, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i].
				 * Add codes to perform some actions with this token.
				 */

				if (substr_len >= MAX_TOKEN_STR_LEN)
				{
					printf("Exceeded max token string length!\n");
					return 0;
				}

				strncpy(tokens[nr_token].str, substr_start, substr_len);

				switch (rules[i].token_type)
				{
				default:
					tokens[nr_token].type = rules[i].token_type;
					nr_token++;
				}

				break;
			}
		}

		if (i == NR_REGEX)
		{
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true;
}

static bool check_valid_parentheses(uint32_t start, uint32_t end, bool *success)
{
	if (*success == false)
	{
		return 0;
	}
	// printf("start: %d, end: %d.\n", start, end);
	uint32_t parentheses_depth = 0;
	int i = start;
	for (; i <= end && parentheses_depth >= 0; i++)
	{
		// printf("parentheses_depth: %d\n",parentheses_depth);
		if (tokens[i].type == '(')
			parentheses_depth++;
		else if (tokens[i].type == ')')
			parentheses_depth--;
	}
	if (parentheses_depth != 0 || i != end + 1)
	{
		printf("Syntax ERROR: unpaired parentheses!\n");
		*success = false;
		return false;
	}
	else
	{
		return true;
	}
}

static bool check_pair_parentheses(uint32_t start, uint32_t end, bool *success)
{
	if (*success == false)
	{
		return 0;
	}
	if (check_valid_parentheses(start, end, success) == false)
	{
		return false;
	}
	if (tokens[start].type == '(' && tokens[end].type == ')') // pair of parentheses
	{
		return true;
	}
	return false;
}

inline static uint32_t skip_parentheses(uint32_t start, uint32_t end, bool *success)
{
	if (*success == false)
	{
		return 0;
	}
	if (tokens[start].type != '(')
	{
		printf("Internal Eval ERROR!\n");
		assert(0);
	}
	uint32_t parentheses_depth = 1;
	int i = start + 1;
	for (; i <= end && parentheses_depth > 0; i++)
	{
		// printf("parentheses_depth: %d\n",parentheses_depth);
		if (tokens[i].type == '(')
			parentheses_depth++;
		else if (tokens[i].type == ')')
			parentheses_depth--;
	}
	if (parentheses_depth != 0)
	{
		printf("Syntax ERROR: unpaired parentheses!\n");
		*success = false;
		return false;
	}
	return i;
}

static uint32_t find_specific_last_op(uint32_t start, uint32_t end, char *op, bool *success, bool *found)
{
	if (*success == false)
	{
		return 0;
	}
	uint32_t result = start;
	for (int i = start; i <= end; i++)
	{
		if (tokens[i].type == '(')
		{
			i = skip_parentheses(i, end, success);
		}
		if (strcmp(tokens[i].str, op) == 0)
		{
			result = i;
			*found = true;
		}
	}
	return result;
}

static inline uint32_t max(uint32_t a, uint32_t b)
{
	if (a > b)
	{
		return a;
	}
	else
	{
		return b;
	}
}

static uint32_t find_dom_op(uint32_t start, uint32_t end, bool *success)
{
	if (*success == false)
	{
		return 0;
	}
	uint32_t result = start;
	bool found = false;

	// level 1
	result = max(find_specific_last_op(start, end, "+", success, &found), find_specific_last_op(start, end, "-", success, &found));
	if (found == true)
	{
		printf("Dom_op level 1: index: %d, oprand: %c\n", result, tokens[result].type);
		return result;
	}

	// level 2
	result = max(result, find_specific_last_op(start, end, "*", success, &found));
	result = max(result, find_specific_last_op(start, end, "/", success, &found));
	if (found == true)
	{
		printf("Dom_op level 2: index: %d, oprand: %c\n", result, tokens[result].type);
		return result;
	}

	printf("ERROR: no valid oprand found!\n");
	*success = false;
	return 0;
}

static uint32_t eval_tokens(uint32_t start, uint32_t end, bool *success)
{
	if (*success == false)
	{
		return 0;
	}
	if (start > end)
	{
		/* Bad expression */
		printf("Token eval ERROR: start index > end index!\n");
		*success = false;
		return 0;
	}
	else if (start == end)
	{
		if (*success == false)
		{
			return 0;
		}
		/* Single token.
		 * For now this token should be a number.
		 * Return the value of the number.
		 */
		if (tokens[start].type != NUM)
		{
			printf("Syntax ERROR: wrong single token type!\n");
			*success = false;
			return 0;
		}
		return atoi(tokens[start].str);
	}
	else if (check_pair_parentheses(start, end, success) == true)
	{
		/* The expression is surrounded by a matched pair of parentheses.
		 * If that is the case, just throw away the parentheses.
		 */

		return eval_tokens(start + 1, end - 1, success);
	}
	else
	{
		if (*success == false)
		{
			return 0;
		}

		uint32_t dom_op_index = find_dom_op(start, end, success);
		uint32_t value1 = eval_tokens(start, dom_op_index - 1, success);
		uint32_t value2 = eval_tokens(dom_op_index + 1, end, success);

		if (*success == false)
		{
			return 0;
		}

		switch (tokens[dom_op_index].type)
		{
		case '+':
			return value1 + value2;
		case '-':
			return value1 - value2;
		case '*':
			return value1 * value2;
		case '/':
			return value1 / value2;
		default:
			printf("Syntax ERROR: Invalid oprand!\n");
			*success = false;
			return 0;
		}
	}
	printf("Interval eval ERROR: func should not reach here!\n");
	*success = false;
	assert(0);
	return 0;
}

uint32_t expr(char *e, bool *success)
{
	*success = true;
	if (!make_token(e))
	{
		*success = false;
		return 0;
	}

	return eval_tokens(0, nr_token - 1, success);
}
