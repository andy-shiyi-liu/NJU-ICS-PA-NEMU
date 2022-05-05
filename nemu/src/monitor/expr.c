#include "nemu.h"
#include "cpu/reg.h"
#include "memory/memory.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

#define MAX_TOKEN_STR_LEN 11
#define MAX_TOKEN_NUM 32

extern CPU_STATE cpu;

enum
{
	NOTYPE = 256,
	EQ,
	NUM,
	REG32,
	REG16,
	REG8,
	SYMB,
	NEQ,
	LEQ,
	GEQ,
	AND,
	OR,
	SHL,
	SHR,
	NUM8_1,
	NUM8_2,
	NUM16_1,
	NUM16_2,
	NUM2_1,
	NUM2_2,
	DREF,
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
	{"\\$eax|\\$ecx|\\$edx|\\$ebx|\\$esp|\\$ebp|\\$esi|\\$edi|\\$eip", REG32},
	{"\\$ax|\\$cx|\\$dx|\\$bx|\\$sp|\\$bp|\\$si|\\$di", REG16},
	{"\\$ah|\\$al|\\$ch|\\$cl|\\$dh|\\$dl|\\$bh|\\$bl", REG8},
	{"\\+", '+'},
	{"-", '-'},
	{"\\*", '*'},
	{"\\/", '/'},
	{"\\(", '('},
	{"\\)", ')'},
	{"==", EQ},
	{"!=", NEQ},
	{"<=", LEQ},
	{">=", GEQ},
	{"&&", AND},
	{"\\|\\|", OR},
	{"<<", SHL},
	{">>", SHR},
	{"&", '&'},
	{"\\|", '|'},
	{"\\^", '^'},
	{"!", '!'},
	{"~", '~'},
	{"<", '<'},
	{">", '>'},
	{"%", '%'},
	{"0[Xx][0-9a-fA-F]+", NUM16_1},
	{"[0-9a-fA-F]+[Hh]", NUM16_2},
	{"0[Oo][0-9a-fA-F]+", NUM8_1},
	{"[0-9a-fA-F]+[Oo]", NUM8_2},
	{"0[Bb][0-9a-fA-F]+", NUM2_1},
	{"[0-9a-fA-F]+[Bb]", NUM2_2},
	{"[0-9]+", NUM},
	{"[a-zA-Z_]*", SYMB},
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

				// printf("match regex[%d] at position %d with len %d: %.*s\n", i, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i].
				 * Add codes to perform some actions with this token.
				 */

				if (substr_len >= MAX_TOKEN_STR_LEN)
				{
					printf("Exceeded max token string length!\n");
					return 0;
				}

				switch (rules[i].token_type)
				{
				case NOTYPE:
					break;
				case '*':
					if (nr_token == 0 || (tokens[nr_token - 1].type != REG32 && tokens[nr_token - 1].type != REG16 && tokens[nr_token - 1].type != REG8 && tokens[nr_token - 1].type != ')' &&
										  tokens[nr_token - 1].type != NUM16_1 && tokens[nr_token - 1].type != NUM16_2 && tokens[nr_token - 1].type != NUM8_1 && tokens[nr_token - 1].type != NUM8_2 && tokens[nr_token - 1].type != NUM2_1 && tokens[nr_token - 1].type != NUM2_2 && tokens[nr_token - 1].type != NUM && tokens[nr_token - 1].type != SYMB))
					{
						tokens[nr_token].type = DREF;
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						nr_token++;
						break;
					}
					else
					{
						tokens[nr_token].type = rules[i].token_type;
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						nr_token++;
						break;
					}
				case '-': // minus
					if (nr_token == 0 || tokens[nr_token - 1].type == '(')
					{
						tokens[nr_token].type = NUM;
						strncpy(tokens[nr_token].str, "0", 1);
						nr_token++;
					}
					tokens[nr_token].type = rules[i].token_type;
					strncpy(tokens[nr_token].str, substr_start, substr_len);
					nr_token++;
					break;
				default:
					tokens[nr_token].type = rules[i].token_type;
					strncpy(tokens[nr_token].str, substr_start, substr_len);
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

static uint32_t find_specific_last_op(uint32_t start, uint32_t end, uint32_t op, bool *success, bool *found)
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
		if (tokens[i].type == op)
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

	// level 1: ||
	result = find_specific_last_op(start, end, OR, success, &found);
	if (found == true)
	{
		// printf("Dom_op level 1: index: %d, oprand: %c\n", result, tokens[result].type);
		return result;
	}

	// level 2: &&
	result = find_specific_last_op(start, end, AND, success, &found);
	if (found == true)
	{
		// printf("Dom_op level 2: index: %d, oprand: %c\n", result, tokens[result].type);
		return result;
	}

	// level 3: |
	result = find_specific_last_op(start, end, '|', success, &found);
	if (found == true)
	{
		// printf("Dom_op level 3: index: %d, oprand: %c\n", result, tokens[result].type);
		return result;
	}

	// level 4: ^
	result = find_specific_last_op(start, end, '^', success, &found);
	if (found == true)
	{
		// printf("Dom_op level 4: index: %d, oprand: %c\n", result, tokens[result].type);
		return result;
	}

	// level 5: &
	result = find_specific_last_op(start, end, '&', success, &found);
	if (found == true)
	{
		// printf("Dom_op level 5: index: %d, oprand: %c\n", result, tokens[result].type);
		return result;
	}

	// level 6: == !=
	result = max(find_specific_last_op(start, end, EQ, success, &found), find_specific_last_op(start, end, NEQ, success, &found));
	if (found == true)
	{
		// printf("Dom_op level 6: index: %d, oprand: %c\n", result, tokens[result].type);
		return result;
	}

	// level 7: < <= > >=
	result = max(find_specific_last_op(start, end, LEQ, success, &found), find_specific_last_op(start, end, '<', success, &found));
	result = max(result, find_specific_last_op(start, end, GEQ, success, &found));
	result = max(result, find_specific_last_op(start, end, '>', success, &found));
	if (found == true)
	{
		// printf("Dom_op level 7: index: %d, oprand: %c\n", result, tokens[result].type);
		return result;
	}

	// level 8: << >>
	result = max(find_specific_last_op(start, end, SHL, success, &found), find_specific_last_op(start, end, SHR, success, &found));
	if (found == true)
	{
		// printf("Dom_op level 8: index: %d, oprand: %c\n", result, tokens[result].type);
		return result;
	}

	// level 9: + -
	result = max(find_specific_last_op(start, end, '+', success, &found), find_specific_last_op(start, end, '-', success, &found));
	if (found == true)
	{
		// printf("Dom_op level 9: index: %d, oprand: %c\n", result, tokens[result].type);
		return result;
	}

	// level 10: * / %
	result = max(find_specific_last_op(start, end, '*', success, &found), find_specific_last_op(start, end, '/', success, &found));
	result = max(result, find_specific_last_op(start, end, '%', success, &found));
	if (found == true)
	{
		// printf("Dom_op level 10: index: %d, oprand: %c\n", result, tokens[result].type);
		return result;
	}

	// level 11: ~ ! *(DREF)
	result = max(find_specific_last_op(start, end, '~', success, &found), find_specific_last_op(start, end, '!', success, &found));
	result = max(result, find_specific_last_op(start, end, DREF, success, &found));
	if (found == true)
	{
		// printf("Dom_op level 11: index: %d, oprand: %c\n", result, tokens[result].type);
		return result;
	}

	printf("ERROR: no valid oprand found!\n");
	*success = false;
	return 0;
}

static void strupr(char str[])
{
	for (int i = 0; i < strlen(str); i++)
	{
		str[i] = toupper(str[i]);
	}
}

static inline uint32_t get_reg32(uint32_t index)
{
	if (strncmp("$eax", tokens[index].str, 4) == 0)
	{
		return cpu.eax;
	}
	else if (strncmp("$ecx", tokens[index].str, 4) == 0)
	{
		return cpu.ecx;
	}
	else if (strncmp("$edx", tokens[index].str, 4) == 0)
	{
		return cpu.edx;
	}
	else if (strncmp("$ebx", tokens[index].str, 4) == 0)
	{
		return cpu.ebx;
	}
	else if (strncmp("$esp", tokens[index].str, 4) == 0)
	{
		return cpu.esp;
	}
	else if (strncmp("$ebp", tokens[index].str, 4) == 0)
	{
		return cpu.ebp;
	}
	else if (strncmp("$esi", tokens[index].str, 4) == 0)
	{
		return cpu.esi;
	}
	else if (strncmp("$edi", tokens[index].str, 4) == 0)
	{
		return cpu.edi;
	}
	else if (strncmp("$eip", tokens[index].str, 4) == 0)
	{
		return cpu.eip;
	}
	else
	{
		printf("Internal ERROR: no register matched!\n");
		assert(0);
	}
}

static inline uint32_t get_reg16(uint32_t index)
{
	if (strncmp("$ax", tokens[index].str, 4) == 0)
	{
		return cpu.eax & 0x0000ffff;
	}
	else if (strncmp("$cx", tokens[index].str, 4) == 0)
	{
		return cpu.ecx & 0x0000ffff;
	}
	else if (strncmp("$dx", tokens[index].str, 4) == 0)
	{
		return cpu.edx & 0x0000ffff;
	}
	else if (strncmp("$bx", tokens[index].str, 4) == 0)
	{
		return cpu.ebx & 0x0000ffff;
	}
	else if (strncmp("$sp", tokens[index].str, 4) == 0)
	{
		return cpu.esp & 0x0000ffff;
	}
	else if (strncmp("$bp", tokens[index].str, 4) == 0)
	{
		return cpu.ebp & 0x0000ffff;
	}
	else if (strncmp("$si", tokens[index].str, 4) == 0)
	{
		return cpu.esi & 0x0000ffff;
	}
	else if (strncmp("$di", tokens[index].str, 4) == 0)
	{
		return cpu.edi & 0x0000ffff;
	}
	else
	{
		printf("Internal ERROR: no register matched!\n");
		assert(0);
	}
}

static inline uint32_t get_reg8(uint32_t index)
{
	if (strncmp("$ah", tokens[index].str, 4) == 0)
	{
		return (cpu.eax & 0x0000ff00) >> 8;
	}
	else if (strncmp("$ch", tokens[index].str, 4) == 0)
	{
		return (cpu.ecx & 0x0000ff00) >> 8;
	}
	else if (strncmp("$dh", tokens[index].str, 4) == 0)
	{
		return (cpu.edx & 0x0000ff00) >> 8;
	}
	else if (strncmp("$bh", tokens[index].str, 4) == 0)
	{
		return (cpu.ebx & 0x0000ff00) >> 8;
	}
	else if (strncmp("$al", tokens[index].str, 4) == 0)
	{
		return cpu.eax & 0x000000ff;
	}
	else if (strncmp("$cl", tokens[index].str, 4) == 0)
	{
		return cpu.ecx & 0x000000ff;
	}
	else if (strncmp("$dl", tokens[index].str, 4) == 0)
	{
		return cpu.edx & 0x000000ff;
	}
	else if (strncmp("$bl", tokens[index].str, 4) == 0)
	{
		return cpu.ebx & 0x000000ff;
	}
	else
	{
		printf("Internal ERROR: no register matched!\n");
		assert(0);
	}
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
		char temp_str[MAX_TOKEN_STR_LEN];
		char *temp; // for passing into strtoul()
		if (tokens[start].type == NUM)
		{
			return atoi(tokens[start].str);
		}
		else if (tokens[start].type == NUM16_1)
		{
			strupr(tokens[start].str);
			return strtoul(tokens[start].str, &temp, 16);
		}
		else if (tokens[start].type == NUM16_2)
		{
			strupr(tokens[start].str);
			temp_str[strlen(temp_str) - 1] = 0;
			return strtoul(tokens[start].str, &temp, 16);
		}
		else if (tokens[start].type == NUM8_1)
		{
			strupr(tokens[start].str);
			return strtoul(tokens[start].str, &temp, 8);
		}
		else if (tokens[start].type == NUM8_2)
		{
			strupr(tokens[start].str);
			temp_str[strlen(temp_str) - 1] = 0;
			return strtoul(tokens[start].str, &temp, 8);
		}
		else if (tokens[start].type == NUM2_1)
		{
			strupr(tokens[start].str);
			return strtoul(tokens[start].str, &temp, 2);
		}
		else if (tokens[start].type == NUM2_2)
		{
			strupr(tokens[start].str);
			temp_str[strlen(temp_str) - 1] = 0;
			return strtoul(tokens[start].str, &temp, 2);
		}
		else if (tokens[start].type == REG32)
		{
			return get_reg32(start);
		}
		else if (tokens[start].type == REG16)
		{
			return get_reg16(start);
		}
		else if (tokens[start].type == REG8)
		{
			return get_reg8(start);
		}
		else if (tokens[start].type == SYMB)
		{
			uint32_t result = look_up_symtab(tokens[start].str, success);
			if (*success == false)
			{
				printf("Symbol ERROR: unmatch with symbol table!\n");
				return false;
			}
			return result;
		}
		else
		{
			printf("Syntax ERROR: wrong single token type!\n");
			*success = false;
			return 0;
		}
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
		uint32_t value1 = 0;
		uint32_t value2 = eval_tokens(dom_op_index + 1, end, success);

		if (*success == false)
		{
			return 0;
		}

		switch (tokens[dom_op_index].type)
		{
		case '+':
			value1 = eval_tokens(start, dom_op_index - 1, success);
			return value1 + value2;
		case '-':
			value1 = eval_tokens(start, dom_op_index - 1, success);
			return value1 - value2;
		case '*':
			value1 = eval_tokens(start, dom_op_index - 1, success);
			return value1 * value2;
		case '/':
			value1 = eval_tokens(start, dom_op_index - 1, success);
			return value1 / value2;
		case EQ:
			value1 = eval_tokens(start, dom_op_index - 1, success);
			return value1 == value2;
		case NEQ:
			value1 = eval_tokens(start, dom_op_index - 1, success);
			return value1 != value2;
		case LEQ:
			value1 = eval_tokens(start, dom_op_index - 1, success);
			return value1 <= value2;
		case GEQ:
			value1 = eval_tokens(start, dom_op_index - 1, success);
			return value1 >= value2;
		case AND:
			value1 = eval_tokens(start, dom_op_index - 1, success);
			return value1 && value2;
		case OR:
			value1 = eval_tokens(start, dom_op_index - 1, success);
			return value1 || value2;
		case SHL:
			value1 = eval_tokens(start, dom_op_index - 1, success);
			return value1 << value2;
		case SHR:
			value1 = eval_tokens(start, dom_op_index - 1, success);
			return value1 >> value2;
		case '&':
			value1 = eval_tokens(start, dom_op_index - 1, success);
			return value1 & value2;
		case '|':
			value1 = eval_tokens(start, dom_op_index - 1, success);
			return value1 | value2;
		case '^':
			value1 = eval_tokens(start, dom_op_index - 1, success);
			return value1 ^ value2;
		case '!':
			return !value2;
		case '~':
			return ~value2;
		case '>':
			value1 = eval_tokens(start, dom_op_index - 1, success);
			return value1 > value2;
		case '<':
			value1 = eval_tokens(start, dom_op_index - 1, success);
			return value1 < value2;
		case '%':
			value1 = eval_tokens(start, dom_op_index - 1, success);
			return value1 % value2;
		case DREF:
			if (value1 + 4 >= MEM_SIZE_B)
			{
				printf("ERROR: Memory read overflow!\n");
				*success = false;
				return 0;
			}
			return hw_mem_read(value1, 4);
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
