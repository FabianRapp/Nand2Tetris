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
// returns true for every valid char is the hack-asm
// language
bool	valid_char(char c)
{
	assume(c != '/');
	if (c == ' ' || !isascii(c) || c == ')')
	//if ((c != 'D' && c != 'M' && c != 'A' && c != '0' && c != '1' && c != '-'
	//	&& c != '&' && c != '|' && c != '!' && c != ';' && c != '\n'
	//	&& c != '=' && c != '+' && c != 'G' && c != 'T' && c != 'E'
	//	&& c != 'Q' && c != 'L' && c != 'N' && c != '('
	//	&& c != '@' && c != 'P' && c != '_' && c != 'J' && !isascii(c))
	//	|| c == ')')
	{
		return (false);
	}
	return (true);
}

size_t	filter_charset(volatile char *buffer, int read_bytes)
{
	size_t	byte_count = 0;
	static bool	empty_line = true;
	static bool	is_comment = false;
	assume(read_bytes > 0 && read_bytes <= READ_CHUNK_SIZE);
	for (int i = 0; i < read_bytes; i++)
	{
		if (buffer[i] == '/' || is_comment)
			is_comment = true;
		while (i < read_bytes && buffer[i] != '\n' && (is_comment ||
			buffer[i] == ' ' || buffer[i] == 0x0d))
		{
			i++;
			if (buffer[i] == '/' || is_comment)
				is_comment = true;
		}
		assume(i<=read_bytes);
		if (i == read_bytes)
			return (byte_count);
		if (!valid_char(buffer[i]))
			continue ;
		if (buffer[i] == '\n' && !empty_line)
		{
			buffer[byte_count++] = 0;
			//buffer[byte_count++] = '\n';
			is_comment = false;
			empty_line = true;
		}
		else if(buffer[i] == '\n')
			is_comment = false;
		else
		{
			buffer[byte_count++] = buffer[i];
			empty_line = false;
		}
	}
	return (byte_count);
}
