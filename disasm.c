#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "disasm.h"
#include "6502.h"
#include "mappers.h"

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
			input_file = malloc(sizeof(char) * strlen(argv[++i])+1);
			strcpy(input_file, argv[i]);
		}
		else if(!strncmp(argv[i], "-o", 2)) {
			output_file = malloc(sizeof(char) * strlen(argv[++i])+1);
			strcpy(output_file, argv[i]);
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
	if(fread(file, 1, *size, f) != *size) {
		printf("could not read in %s", filename);
	}
	file[*size] = '\0';

	fclose(f);

	return file;
}

void hex_dump(uint8_t *binary, size_t size, char *output_file)
{
	char *hex = calloc(size*20, sizeof(char));
	
	for(size_t i=0; i < size; i++) {
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
			char operand[3];
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
	char *disassembly = calloc(size*10, sizeof(char));

	//first 16 bits is header
	//magic number
	char magic[5];
	memcpy(magic, binary, 4);
	magic[4] = '\0';
	
	if(magic[0] != 'N' ||
	   magic[1] != 'E' ||
	   magic[2] != 'S' ||
	   magic[3] != 0x1A) {
		printf("Input is not a .nes file!\n");
		return;
	}

	bool ines2 = false;
	if((binary[7] & 0x0C) == 0x8) {
		ines2 = true;
	}

	bool trainer = false;
	if((binary[6] & 0x04) == 0x04) {
		trainer = true;
	}
	
	//sizes
	uint16_t prg_rom;
	if((binary[9] & 0x0F) == 0x0F) {
		prg_rom = (int)pow(2, binary[4] & 0xFC) * (((binary[4] & 0x03) * 2) + 1);
	}
	else {
		prg_rom = binary[4] | ((binary[9] & 0x0F) << 8);
	}
	uint16_t chr_rom;
	if((binary[9] & 0xF0) == 0xF0) {
		chr_rom = (int)pow(2, binary[5] & 0xFC) * (((binary[5] & 0x03) * 2) + 1);
	}
	else {
	        chr_rom = binary[5] | ((binary[9] & 0xF0) << 4);
	}
	
	uint32_t prg_ram = ((binary[10] & 0x0F) == 0) ? 0 : (64 << (binary[10] & 0x0F));
	uint32_t prg_nvram = ((binary[10] & 0xF0) == 0) ? 0 : (64 << (binary[10] & 0xF0));

	uint32_t chr_ram = ((binary[11] & 0x0F) == 0) ? 0 : (64 << (binary[11] & 0x0F));
	uint32_t chr_nvram = ((binary[11] & 0xF0) == 0) ? 0 : (64 << (binary[11] & 0xF0));
	
	//mapper
	uint16_t mapper = ((binary[6] & 0xF0) >> 4) | (binary[6] & 0xF0) | ((binary[8] & 0xF0) << 8);
	uint8_t submapper = (binary[6] & 0xF0) >> 4;

	//nes 2.0 header
	const char *ines_version =  ines2 ? "iNES 2.0 header" : "iNES 1.0 header";
	
	char header[512];
	sprintf(header,
		";%s\n"
		";%s\n"
		";prg rom pages: %u\n"
		";chr rom pages: %u\n"
		";prg ram size: %u\n"
		";prg nvram size: %u\n"
		";chr ram size: %u\n"
		";chr nvram size: %u\n"
		";mapper number: %u\n"
		";submapper number: %u\n",
		magic,
		ines_version,
		prg_rom,
		chr_rom,
		prg_ram,
		prg_nvram,
		chr_ram,
		chr_nvram,
		mapper,
		submapper);
	strcat(disassembly, header);

	//PRG_ROM
	size_t prg_start = 15 + (trainer ? 512 : 0);
	uint64_t prg_page_size = mapper_table[mapper].prg_rom_size;
	for(size_t i=prg_start; i <  prg_page_size * prg_rom; i++) {
		struct opcode_def opcode = asm_opcodes[binary[i]];
		strcat(disassembly, opcode.mnemonic);
		strcat(disassembly, "\t");
		if(opcode.size > 1) {
			strcat(disassembly, "$");
		}
		
		for(int j=0; j < opcode.size-1; j++) {
			char operand[3];
			sprintf(operand, "%X", binary[++i]);	
			strcat(disassembly, operand);
		}
		
		strcat(disassembly, "\n");
	}

	//PRG
	size_t chr_start = 15 + (trainer ? 512 : 0) +  prg_page_size * prg_rom;
	uint64_t chr_page_size = mapper_table[mapper].chr_rom_size;

	
	//write disassembled source output file
	FILE *out = fopen(output_file, "w");
	fwrite(disassembly, sizeof(uint8_t), strlen(disassembly), out);
	free(disassembly);
	fclose(out);
}
