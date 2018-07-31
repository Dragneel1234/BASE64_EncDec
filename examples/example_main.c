#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "include\\base64_encdec.h"

#define APP_NAME "BASE64_ENC_DEC"

enum LOCAL_ERROR
{
	ISOKAY, EXTRA_PARAMETERS, STRING_FILE_MODE_BOTHTRUE, STRING_FILE_MODE_BOTHFALSE, STRING_NOTENOUGH_MEMORY,
	FILE_INPUT_NOT_OPEN, FILE_OUTPUT_NOT_OPEN, BUFFER_NOTENOUGH_MEMORY
};

void print_help()
{
	fprintf(stdout, "Usage: " APP_NAME " (-f/-s) [-d] input [output]\n"
					"\n"
					"Positional argument:\n"
					"  -f:\t\t file mode\n"
					"  -s:\t\t string mode\n"
					"  input:\t input string or file\n"
					"  output:\t output file(string will be written to console)\n"
					"\n"
					"Optional argument:\n"
					"  -d:\t\t decode from BASE64\n");
}

int8_t *file_read(FILE *pointer, size_t *read_bytes)
{
	int64_t file_size;
	_fseeki64(pointer, 0, SEEK_END);
	file_size = _ftelli64(pointer);
	_fseeki64(pointer, 0, SEEK_SET);

	if (file_size > SIZE_MAX)
	{
		file_size = SIZE_MAX;
	}

	int8_t *buffer = (int8_t *)malloc((size_t)file_size);
	fread((char *)buffer, sizeof(int8_t), (size_t)file_size, pointer);

	*read_bytes = file_size;
	return buffer;
}

int32_t encode(int8_t *input, size_t size, int8_t *output, size_t *capacity, bool is_string_mode)
{
	if (is_string_mode)
	{
		int32_t result = base64_encode(input, strlen((char *)input), output, capacity);
		return result;
	}
	else
	{
		int32_t result = base64_encode(input, size, output, capacity);
		return result;
	}

	return -1;
}

int32_t decode(int8_t *input, size_t size, int8_t *output, size_t *capacity, bool is_string_mode)
{
	if (is_string_mode)
	{
		int32_t result = base64_decode(input, strlen((char *)input), output, capacity);
		return result;
	}
	else
	{
		int32_t result = base64_decode(input, size, output, capacity);
		return result;
	}

	return -1;
}

int main(int argc, char const *argv[])
{
	bool file_mode = false;
	bool string_mode = false;
	bool is_decode = false;
	char *input = NULL;
	char *output = NULL;

	if (argc == 1)
	{
		print_help();
		return ISOKAY;
	}

	/* At zero, binary name is present */
	for (int i = 1; i < argc; ++i)
	{
		if (argv[i][0] == '-')
		{
			switch(argv[i][1])
			{
				case 'f':
					file_mode = true;
					break;
				case 's':
					string_mode = true;
					break;
				case 'd':
					is_decode = true;
					break;

				default:
					fprintf(stderr, "Extra parameter: %s\n", argv[i]);
					return EXTRA_PARAMETERS;
			}
		}
		else if (input == NULL)
		{
			input = (char *)argv[i];
		}
		else if (output == NULL)
		{
			output = (char *)argv[i];
		}
		else
		{
			fprintf(stderr, "Extra parameter: %s\n", argv[i]);
			return EXTRA_PARAMETERS;
		}
	}

	if (file_mode == true && string_mode == true)
	{
		fprintf(stderr, "Both string and file mode is set to true\n");
		return STRING_FILE_MODE_BOTHTRUE;
	}
	else if (file_mode == false && string_mode == false)
	{
		fprintf(stderr, "Both string and file mode is set to false\n");
		return STRING_FILE_MODE_BOTHFALSE;
	}

	if (string_mode == true)
	{
		size_t input_size = strlen(input);
		size_t output_capacity = (input_size * 4 / 3) + 1;
		output = (char *)malloc(output_capacity);
		if (output == NULL)
		{
			fprintf(stderr, "Cannot allocate memory for output string\n");
			return STRING_NOTENOUGH_MEMORY;
		}
		memset(output, 0, output_capacity);

		int32_t result = -1;
		if (is_decode == false)
		{
			result = encode((int8_t *)input, input_size, (int8_t *)output, &output_capacity, string_mode);
		}
		else
		{
			result = decode((int8_t *)input, input_size, (int8_t *)output, &output_capacity, string_mode);
		}
		
		printf("%s\n\nError code: %d\n", output, result);
		free(output);
	}
	else
	{
		FILE *inptr, *outptr; 
		fopen_s(&inptr, input, "rb");
		if (inptr == NULL)
		{
			fprintf(stderr, "Cannot open input file\n");
			return FILE_INPUT_NOT_OPEN;
		}

		fopen_s(&outptr, output, "wb");
		if (outptr == NULL)
		{
			fprintf(stderr, "Cannot create output file\n");
			return FILE_OUTPUT_NOT_OPEN;
		}

		size_t input_size, output_capacity;
		int8_t *in_buffer = file_read(inptr, &input_size);
		if (in_buffer == NULL)
		{
			fprintf(stderr, "Cannot allocate memory for input buffer\n");
			return BUFFER_NOTENOUGH_MEMORY;
		}

		output_capacity = (input_size * 4 / 3) + 1;
		int8_t *out_buffer = (int8_t *)malloc(output_capacity);
		if (out_buffer == NULL)
		{
			fprintf(stderr, "Cannot allocate memory for output buffer\n");
			return BUFFER_NOTENOUGH_MEMORY;
		}

		int32_t result;

		if (is_decode == false)
		{
			result = encode(in_buffer, input_size, out_buffer, &output_capacity, string_mode);
		}
		else
		{
			result = decode(in_buffer, input_size, out_buffer, &output_capacity, string_mode);
		}
		
		fwrite(out_buffer, sizeof(int8_t), output_capacity, outptr);

		free(out_buffer);
		free(in_buffer);

		fclose(outptr);
		fclose(inptr);

		printf("Error code: %d\n", result);
	}	
	

	return ISOKAY;
}