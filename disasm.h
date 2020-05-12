#ifndef __DISASM_H__
#define __DISASM_H__

uint8_t *read_file(char *filename, size_t *size);

void hex_dump(uint8_t *binary, size_t size, char *output_file);
void convert(uint8_t *binary, size_t size, char *output_file);
void nes_convert(uint8_t *binary, size_t size, char *output_file);

#endif
