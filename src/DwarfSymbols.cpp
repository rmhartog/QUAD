/*****
 *
 * DwarfSymbols.cpp
 *
 * Author: Reinier M. Hartog
 *
 *****/

#include "DwarfSymbols.h"
#include <cstring>

DwarfFunctionSymbol::DwarfFunctionSymbol(const FunctionEntry &fe) : entry(fe) {
}

DwarfFunctionSymbol::~DwarfFunctionSymbol() {
}

DwarfFunctionSymbol *DwarfFunctionSymbol::fromEntry(const FunctionEntry &fe) {
	return new DwarfFunctionSymbol(fe);
}

unsigned int DwarfFunctionSymbol::getName(char *buffer, size_t size) const {
	size_t copy = entry.name.size() + 1;
	if (size != 0) {
		if (size < copy) {
			copy = size;
		}

		memcpy(buffer, entry.name.c_str(), copy - 1);
		buffer[copy - 1] = '\0';
	}
	return copy;
}

DwarfVariableSymbol::DwarfVariableSymbol(const VarEntry &ve) : entry(ve) {
}

DwarfVariableSymbol::~DwarfVariableSymbol() {
}

DwarfVariableSymbol *DwarfVariableSymbol::fromEntry(const VarEntry &ve) {
	return new DwarfVariableSymbol(ve);
}

unsigned int DwarfVariableSymbol::getName(char *buffer, size_t size) const {
	size_t copy = entry.name.size() + 1;
	if (size != 0) {
		if (size < copy) {
			copy = size;
		}

		memcpy(buffer, entry.name.c_str(), copy - 1);
		buffer[copy - 1] = '\0';
	}
	return copy;
}

