/*****
 *
 * DwarfSymbolResolver.cpp
 *
 * Author: Reinier M. Hartog
 * Date:   19 November 2012
 *
 * An implementation of SymbolResolver based on DWARF-information.
 *
 *****/

#include "DwarfSymbolResolver.h"
	
#include <string.h>

void internal_dwarf_handler(Dwarf_Error err, Dwarf_Ptr arg) {
}

DwarfSymbolResolver::DwarfSymbolResolver() {
}

DwarfSymbolResolver::~DwarfSymbolResolver() {
	if (indexer != 0) {
		delete indexer;
	}
}

Dwarf_Debug* DwarfSymbolResolver::getDwarfHandle() {
	return &dwarf_handle;
}

Dwarf_Error* DwarfSymbolResolver::getDwarfError() {
	return &dwarf_error;
}

void DwarfSymbolResolver::createIndexer() {
	indexer = new DwarfIndexer();
	indexer->accept(*getDwarfHandle(), *getDwarfError());
}

unsigned int DwarfSymbolResolver::createDwarfSymbolResolver(Elf *elf_handle, DwarfSymbolResolver **resolver) {
	DwarfSymbolResolver *dwarf_resolver = new DwarfSymbolResolver();
	if (dwarf_resolver == 0) {
		return 1;
	}

	if (dwarf_elf_init(elf_handle, DW_DLC_READ, &internal_dwarf_handler, 0, dwarf_resolver->getDwarfHandle(), dwarf_resolver->getDwarfError()) != DW_DLV_OK) {
		delete dwarf_resolver;
		return 2;
	}

	if (resolver == 0) {
		delete dwarf_resolver;
		return 3;
	}

	dwarf_resolver->createIndexer();
     
	*resolver = dwarf_resolver;
	return 0;
}

unsigned int DwarfSymbolResolver::destroyDwarfSymbolResolver(DwarfSymbolResolver **resolver) {
	if (resolver == 0) {
		return 1;
	}

	if (*resolver == 0) {
		return 2;
	}

	dwarf_finish(*(*resolver)->getDwarfHandle(), (*resolver)->getDwarfError());

	delete *resolver;
	*resolver = 0;
	return 0;
}

#include <iostream>
using namespace std;

unsigned int DwarfSymbolResolver::resolveFunction(const ExecutionContext& context, void *addr, const FunctionSymbol **function) const {
	list<FunctionEntry>		functions;
	list<FunctionEntry>::iterator	fit;

	cerr << "F:" << addr << endl;
	if (indexer != 0) {
		functions = indexer->getFunctions();

		for (fit = functions.begin(); fit != functions.end(); fit++) {
			
		}
	}
	return 1;
}

unsigned int DwarfSymbolResolver::resolveVariable(const ExecutionContext& context, void *addr, size_t size, const VariableSymbol **variable) const {
	// *variable = new DwarfVariable("testvariable");
	// cerr << "resolveVariable: " << addr << " (" << size << ")" << endl;

	list<VarEntry> 			variables;
	list<VarEntry>::iterator	vit;
	void*				ip;

	if (indexer != 0) {
		variables = indexer->getVariables();
		
		for (vit = variables.begin(); vit != variables.end(); vit++) {
			//cerr << vit->name << endl;		
			void* vaddr;
			size_t vsize = 4;
			if (DwarfMachine::evaluateLocation(context, vit->location, &vaddr) == 0) {
				if (addr >= vaddr && addr < vaddr + vsize) {
					cerr << vit->name << endl;
				}
			}
		}

			
	}
	return 1;
}
