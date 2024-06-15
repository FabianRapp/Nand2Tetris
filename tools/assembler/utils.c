#include "assembler.h"
#include "simple_assembler.h"

// algigns the given buffer to read_len while leaving 12 bytes (longest
// possible instruction infront) for negative indexing
char	*align_buffer(char *base)
{
	const int	longest_instruction_len = 12;
	uintptr_t	address = (uintptr_t)base;
	size_t offset = READ_LEN - (address % READ_LEN);
	if (offset < longest_instruction_len)
		offset += READ_LEN;
	char	*aligned_ptr = base + offset;
	assume (((uintptr_t)aligned_ptr) % READ_LEN == 0);
	return (aligned_ptr);
}

// for parsing benefits '(' is not not valid and thus deleted
// returns true for every valid char in hack-asm that is syntax relevant
bool	relevant_char(char c)
{
	if (c == ' ' || !isascii(c) || c == ')' || c == '/' || c == 0x0d)
		return (false);
	return (true);
}

// makes each line a string which represents an instruction
//	(stays in same buffer, tho location might change duo to the following)
// these contents are filtered out:
// - comments
// - whitespace
// - non ascii chars and simple spaces(this reults in "a b" == "ab")
// - empty lines or lines only containing the points above
// if the last instruction is inclomplte it is not terminated
size_t	filter_charset(volatile char *buffer, int read_bytes)
{
	size_t	byte_count = 0;
	static bool	empty_line = true;
	static bool	is_comment = false;

	assume(read_bytes > 0 && read_bytes <= READ_CHUNK_SIZE);
	for (int i = 0; i < read_bytes; i++)
	{
		while (i < read_bytes && buffer[i] != '\n' && (is_comment ||
			!relevant_char(buffer[i])))
		{
			if (buffer[i] == '/')
				is_comment = true;
			i++;
		}
		if (i == read_bytes)
			return (byte_count);
		if (buffer[i] == '\n')
		{
			if (!empty_line)
				buffer[byte_count++] = 0;
			is_comment = false;
			empty_line = true;
		}
		else
		{
			buffer[byte_count++] = buffer[i];
			empty_line = false;
		}
	}
	return (byte_count);
}
