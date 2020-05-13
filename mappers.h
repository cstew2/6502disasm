#ifndef __MAPPERS_H__
#define __MAPPERS_H__

struct mapper_def {
	uint8_t mapper;
	uint8_t submapper;

	uint64_t prg_rom_size;
	uint64_t chr_rom_size;	
};

const struct mapper_def mapper_table[] = {
	{0, 0, 16384, 8192},
};

#endif
