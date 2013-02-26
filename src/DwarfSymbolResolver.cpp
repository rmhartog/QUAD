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
#include <iostream>
using namespace std;

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

unsigned int DwarfSymbolResolver::findFunction(void *addr, FunctionEntry *fe) const {
	list<FunctionEntry>		functions;
	list<FunctionEntry>::iterator	fit;

	if (indexer != 0) {
		functions = indexer->getFunctions();

		for (fit = functions.begin(); fit != functions.end(); fit++) {
			if (fit->lopc <= (Dwarf_Addr) addr && (Dwarf_Addr) addr < fit->hipc) {
				*fe = *fit;
				return 0;
			}
		}
	}

	return 1;
}

unsigned int DwarfSymbolResolver::findGlobalVariable(const ExecutionContext &context, void *addr, size_t size, struct VarEntry *ve) const {
	list<VarEntry> 			variables;
	list<VarEntry>::iterator	vit;

	if (indexer != 0) {
		variables = indexer->getVariables();
		
		for (vit = variables.begin(); vit != variables.end(); vit++) {		
			TypeEntry type;
			if (indexer->getType(vit->type, &type) != 0) {
				continue;
			}

			void* vaddr;
			size_t vsize = type.size; 
			if (DwarfMachine::evaluateLocation(context, vit->location, &vaddr) == 0) {
				if (addr >= vaddr && addr < vaddr + vsize) {
					*ve = *vit;
					return 0;
				}
			}
		}			
	}
	return 1;
}

unsigned int DwarfSymbolResolver::findLocalVariable(const ExecutionContext &context, void *addr, size_t size, const struct FunctionEntry &fe, struct VarEntry *ve) const {
	list<VarEntry> 			variables;
	list<VarEntry>::iterator	vit;

	cerr << "Lookup local. (" << fe.name << ", " << addr << ")" << endl;

	variables = DwarfIndexer::getVariables(fe);
	for (vit = variables.begin(); vit != variables.end(); vit++) {		
		void* vaddr;
		size_t vsize = 4; // TODO: set the size correctly
		if (DwarfMachine::evaluateLocation(context, vit->location, &vaddr, &fe) == 0) {
			if (addr >= vaddr && addr < vaddr + vsize) {
				cerr << "Local: " << vit->name << endl;
				*ve = *vit;
				return 0;
			}
		}
	}

	return 1;
}

unsigned int DwarfSymbolResolver::resolveFunction(const ExecutionContext& context, void *addr, const FunctionSymbol **function) const {
	FunctionEntry fe;	

	findFunction(addr, &fe);

	return 1;
}

unsigned int DwarfSymbolResolver::resolveVariable(const ExecutionContext& context, void *addr, size_t size, const VariableSymbol **variable) const {
	void 		*ip;
	VarEntry 	ve;
	FunctionEntry	fe;
	
	if (findGlobalVariable(context, addr, size, &ve) == 0) {
		return 10;
	}

	if (context.getInstructionPointer(&ip) != 0) {
		return 2;
	}

	if (findFunction(ip, &fe) == 0) {
		if (findLocalVariable(context, addr, size, fe, &ve) == 0) {
			return 10;
		}
	}

	return 1;
}
