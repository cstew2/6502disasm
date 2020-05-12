#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "disasm.h"
#include "6502.h"

int main(int argc, char **argv)
{
	if(argc < 2) {
		printf("Not enough args\n");
		return 0;
	}

	char *input_file =  NULL;
	char *output_file =  NULL;
	bool hex_flag = false;
	bool nes_flag = false;
	
	for(int i=1; i < argc; i++) {
		if(!strncmp(argv[i], "--hex", 5)) {
			hex_flag = true;
		}
		if(!strncmp(argv[i], "--nes", 5)) {
			nes_flag = true;
		}
		else if(!strncmp(argv[i], "-f", 2)) {
			int len = strlen(argv[++i])+1;
			input_file = malloc(sizeof(char) * len);
			strncpy(input_file, argv[i], len);
		}
		else if(!strncmp(argv[i], "-o", 2)) {
			int len = strlen(argv[++i])+1;
			output_file = malloc(sizeof(char) * len);
			strncpy(output_file, argv[i], len);
		}
	}

	size_t size = 0;
	uint8_t *file = read_file(input_file, &size);

	if(file == NULL) {
		return -1;
	}
	
	if(hex_flag) {
		hex_dump(file, size, output_file);
	}
	else if(nes_flag) {
		nes_convert(file, size, output_file);
	}
	else {
		convert(file, size, output_file);
	}

	free(input_file);
	free(output_file);
       	free(file);
	
	return 0;
}

uint8_t *read_file(char *filename, size_t *size)
{
	FILE *f = NULL;
	f = fopen(filename, "rb");

	if(f == NULL) {
		return NULL;
	}
	
	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	fseek(f, 0, SEEK_SET);
	
        uint8_t *file = calloc(*size+1, sizeof(uint8_t));
	fread(file, 1, *size, f);
	file[*size] = '\0';

	fclose(f);

	return file;
}

void hex_dump(uint8_t *binary, size_t size, char *output_file)
{
	char *hex = calloc(size*20, sizeof(char));
	
	for(int i=0; i < size; i++) {
		char digit[6];
		sprintf(digit, "0x%X\t", binary[i]);
		strcat(hex, digit);
		if((i+1)%8 == 0) {
			strcat(hex, "\n");
		}
	}

	FILE *out = fopen(output_file, "w");
	fwrite(hex, sizeof(uint8_t), strlen(hex), out);
	free(hex);
	fclose(out);
}

void convert(uint8_t *binary, size_t size, char *output_file)
{
	char *disassembly = calloc(size*20, sizeof(char));

	for(size_t i=0; i < size; i++) {
		struct opcode_def opcode = asm_opcodes[binary[i]];
		strcat(disassembly, opcode.mnemonic);
		strcat(disassembly, "\t");
		
		for(int j=0; j < opcode.size-1; j++) {
			char operand[2];
			sprintf(operand, "%X", binary[++i]);
			
			strcat(disassembly, " ");
			strcat(disassembly, operand);
		}
		
		strcat(disassembly, "\n");
	}

	FILE *out = fopen(output_file, "w");
	fwrite(disassembly, sizeof(uint8_t), strlen(disassembly), out);
	free(disassembly);
	fclose(out);
}

void nes_convert(uint8_t *binary, size_t size, char *output_file)
{
	char *disassembly = calloc(size*20, sizeof(char));

	//first 16 bits is header
	char magic[7];
	sprintf(magic, ";%.*s\n", 4, binary);
	strcat(disassembly, magic);
       	
	//PRG_ROM
	for(size_t i=2; i < size; i++) {
		struct opcode_def opcode = asm_opcodes[binary[i]];
		strcat(disassembly, opcode.mnemonic);
		strcat(disassembly, "\t");
		if(opcode.size > 1) {
			strcat(disassembly, "$");
		}
		
		for(int j=0; j < opcode.size-1; j++) {
			char operand[2];
			sprintf(operand, "%X", binary[++i]);	
			strcat(disassembly, operand);
		}
		
		strcat(disassembly, "\n");
	}

	FILE *out = fopen(output_file, "w");
	fwrite(disassembly, sizeof(uint8_t), strlen(disassembly), out);
	free(disassembly);
	fclose(out);
}
