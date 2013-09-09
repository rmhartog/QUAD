/*****
 *
 * ElfSymbolResolver.cpp
 *
 * Author: Reinier M. Hartog, Vlad-Mihai Sima
 * Date:   19 November 2012
 *
 * An implementation of SymbolResolver based on ELF-information.
 * This implementation is based on the code by Vlad-Mihai Sima.
 *
 *****/

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdio.h>

#include "ElfSymbolResolver.h"
#include "gelf.h"

ElfFunctionSymbol::ElfFunctionSymbol(const ElfFunctionSymbol& other) {
	void *low, *high;
	other.getName(symbol_name, 128);
	other.getAddressRange(&low, &high);
	symbol_address = low;
	symbol_size = ((char*) high - (char*) low);
}

ElfFunctionSymbol::ElfFunctionSymbol(const char *name, void *addr, size_t size) {
	strncpy(symbol_name, name, 128);
	symbol_address = addr;
	symbol_size = size;
}

ElfFunctionSymbol::~ElfFunctionSymbol() {
}

unsigned int ElfFunctionSymbol::getAddressRange(void **low, void **high) const {
	if (low != NULL) {
		*low = symbol_address;
	}
	if (high != NULL) {
		*high = ((char*) symbol_address) + symbol_size;
	}
	return 0;
}

unsigned int ElfFunctionSymbol::getName(char *buffer, size_t size) const {
	size_t len = strlen(symbol_name);
	if (buffer == NULL) {
		return len;
	} else {
		size_t min = len < (size - 1) ? len : (size - 1);

		strncpy(buffer, symbol_name, min);
		buffer[min] = '\0';

		return min;
	}
}

unsigned int ElfFunctionSymbol::getReturnType(const Type **type) const {
	return 1;
}

ElfVariableSymbol::ElfVariableSymbol(const ElfVariableSymbol& other) {
	void *low, *high;
	other.getName(symbol_name, 128);
	other.getAddressRange(&low, &high);
	symbol_address = low;
	symbol_size = ((char*) high - (char*) low);
}

ElfVariableSymbol::ElfVariableSymbol(const char *name, void *addr, size_t size) {
	strncpy(symbol_name, name, 128);
	symbol_address = addr;
	symbol_size = size;
}

ElfVariableSymbol::~ElfVariableSymbol() {
}

VariableSymbol *ElfVariableSymbol::clone() {
	return new ElfVariableSymbol(*this);
}

const VariableSymbol *ElfVariableSymbol::clone() const {
	return new ElfVariableSymbol(*this);
}

unsigned int ElfVariableSymbol::getAddressRange(void **low, void **high) const {
	if (low != NULL) {
		*low = symbol_address;
	}
	if (high != NULL) {
		*high = ((char*) symbol_address) + symbol_size;
	}
	return 0;
}

unsigned int ElfVariableSymbol::getName(char *buffer, size_t size) const {
	size_t len = strlen(symbol_name);
	if (buffer == NULL) {
		return len;
	} else {
		size_t min = len < (size - 1) ? len : (size - 1);

		strncpy(buffer, symbol_name, min);
		buffer[min] = '\0';

		return min;
	}
}

unsigned int ElfVariableSymbol::getType(const Type **type) const {
	return 1;
}

ElfSymbolResolver::ElfSymbolResolver() {
}

ElfSymbolResolver::~ElfSymbolResolver() {
	while (!functions.empty()) {
		delete functions.back();
		functions.pop_back();
	}

	while (!variables.empty()) {
		delete variables.back();
		variables.pop_back();
	}
}

unsigned int ElfSymbolResolver::createElfSymbolResolver(ElfSymbolResolver **resolver, const char *filename) {
	int			elf_fd;
	Elf 			*elf_handle;
	Elf_Scn			*elf_scn 	= NULL;
	GElf_Shdr		elf_shdr;
	Elf_Data		*elf_data	= NULL;
	GElf_Sym 		elf_symbol;
	ElfSymbolResolver 	*instance;
	int			symbol_count	= 0;
	int			i 		= 0;
	
	if (elf_version(EV_CURRENT) == EV_NONE) {
		return 1; // failed to initialize libelf
	}

	elf_fd = open(filename, O_RDONLY);
	if (elf_fd == -1) {
		return 2; // failed to open file
	}

	elf_handle = elf_begin(elf_fd, ELF_C_READ, NULL);
	if (elf_handle == 0) {
		close(elf_fd);
		return 3; // failed to elf_begin
	}

	instance = new ElfSymbolResolver();
	if (instance == 0) {
		elf_end(elf_handle);
		close(elf_fd);
		return 4; // failed to allocate instance
	}

	while ((elf_scn = elf_nextscn(elf_handle, elf_scn)) != NULL) {
		if (gelf_getshdr(elf_scn, &elf_shdr) != &elf_shdr) {
			delete instance;
			elf_end(elf_handle);
			close(elf_fd);
			return 5; // gelf_getshdr failed
		}

		if (elf_shdr.sh_type == SHT_SYMTAB) {
			elf_data = elf_getdata(elf_scn, elf_data);
			if (elf_data != NULL) {
				symbol_count = elf_shdr.sh_size / elf_shdr.sh_entsize;

				for (i = 0; i < symbol_count; i++) {
					if (gelf_getsym(elf_data, i, &elf_symbol) == 0) {
						delete instance;
						elf_end(elf_handle);
						close(elf_fd);
						return 6; // gelf_getsym failed
					}

					if (ELF32_ST_BIND(elf_symbol.st_info) == STB_GLOBAL && elf_symbol.st_size > 0) {
						if (ELF32_ST_TYPE(elf_symbol.st_info) == STT_OBJECT) {
							const char *variable_name = elf_strptr(elf_handle, elf_shdr.sh_link, elf_symbol.st_name);
							if (variable_name != 0) {
								printf("variable: %s (%lu)\n", variable_name, (unsigned long int) elf_symbol.st_size);
							}

							ElfVariableSymbol* v = new ElfVariableSymbol(variable_name, (void *) elf_symbol.st_value, (size_t) elf_symbol.st_size);
							instance->addVariable(v);
						} else if (ELF32_ST_TYPE(elf_symbol.st_info) == STT_FUNC) {
							const char *function_name = elf_strptr(elf_handle, elf_shdr.sh_link, elf_symbol.st_name);
							if (function_name != 0) {
								printf("function: %s (%#x - %#x)\n", function_name, (unsigned int) elf_symbol.st_value, (unsigned int) elf_symbol.st_size);
							}

							ElfFunctionSymbol* f = new ElfFunctionSymbol(function_name, (void *) elf_symbol.st_value, (size_t) elf_symbol.st_size);
							instance->addFunction(f);
						}
					}
				}
			}
		}
	}

	if (resolver != 0) {
		*resolver = instance;
	} else {
		delete instance;
	}

	elf_end(elf_handle);
	close(elf_fd);

	return 0;
}

unsigned int ElfSymbolResolver::addFunction(ElfFunctionSymbol *fs) {
	functions.push_back(fs);
	return 0;
}

unsigned int ElfSymbolResolver::addVariable(ElfVariableSymbol *vs) {
	variables.push_back(vs);
	return 0;
}

unsigned int ElfSymbolResolver::resolveFunction(const ExecutionContext& context, void *addr, const FunctionSymbol **result) const {
	vector<ElfFunctionSymbol*>::const_iterator it;
	void *low, *high;

	for (it = functions.begin(); it != functions.end(); it++) {
		ElfFunctionSymbol* function = *it;	

		if (function->getAddressRange(&low, &high) == 0) {
			if (low <= addr && addr < high) {
				if (result != NULL) {
					*result = new ElfFunctionSymbol(*function);
					return 0;
				}
			}
		}
	}
	
	return 1;
}

unsigned int ElfSymbolResolver::resolveVariable(const ExecutionContext& context, void *addr, size_t size, const VariableSymbol **result) const {
	vector<ElfVariableSymbol*>::const_iterator it;
	void *low, *high;

	for (it = variables.begin(); it != variables.end(); it++) {
		ElfVariableSymbol* variable = *it;	

		if (variable->getAddressRange(&low, &high) == 0) {
			if (low <= addr && ((char *) addr) + size <= high) {
				if (result != NULL) {
					*result = new ElfVariableSymbol(*variable);
					return 0;
				}
			}
		}
	}
	
	return 1;
}
