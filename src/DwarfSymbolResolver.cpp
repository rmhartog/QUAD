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
#include "DwarfSymbols.h"
#include "DwarfIndexer.h"
	
#include <string.h>
#include <iostream>
using namespace std;

void internal_dwarf_handler(Dwarf_Error err, Dwarf_Ptr arg) {
}

DwarfSymbolResolver::DwarfSymbolResolver() : current_function(0) {
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

map<unsigned long long, list<VarEntry> > DwarfSymbolResolver::createRelevanceMap(const FunctionEntry &fe) {
	list<VarEntry>				variables;
	list<VarEntry>::iterator		vit;

	cerr << "Rel for " << fe.name << endl;

	map<unsigned long long, list<VarEntry> >	relevantVariables;
	if (indexer != 0) {
		variables = indexer->getVariables(fe);

		for (vit = variables.begin(); vit != variables.end(); vit++) {
			cerr << "   " << vit->name << endl;

			DwarfScriptList::iterator slit;

			for (slit = vit->location.begin(); slit != vit->location.end(); slit++) {
				cerr << "      " << slit->lowpc << " - " << slit->hipc << endl;
				if (slit->lowpc == 0 && (slit->hipc == 0 || slit->hipc == (unsigned long long) -1)) {
					relevantVariables[(unsigned long long) -1].push_back(*vit);
				} else {
					for (unsigned long long addr = slit->lowpc; addr < slit->hipc; addr++) {
						cerr << "         " << addr << endl;
						relevantVariables[addr].push_back(*vit);
					}
				}
			}
		}
	}
	return relevantVariables;
}

void DwarfSymbolResolver::createRelevanceMaps() {
	list<FunctionEntry>		functions;
	list<FunctionEntry>::iterator	fit;

	if (indexer != 0) {
		functions = indexer->getFunctions();

		for (fit = functions.begin(); fit != functions.end(); fit++) {
			relevance[fit->unique_id] = createRelevanceMap(*fit);
		}
	}
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
	dwarf_resolver->createRelevanceMaps();
     
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

const class FunctionSymbol *DwarfSymbolResolver::toSymbol(const struct FunctionEntry &fe) const {
	std::map<std::string, FunctionSymbol*>::iterator fit;
	FunctionSymbol *symbol = 0;
	
	fit = functionSymbols.find(fe.unique_id);
	if (fit == functionSymbols.end()) {
		symbol = DwarfFunctionSymbol::fromEntry(fe);
		functionSymbols[fe.unique_id] = symbol;
	} else {
		symbol = fit->second;
	}
	return symbol;
}

const class VariableSymbol *DwarfSymbolResolver::toSymbol(const struct VarEntry &ve) const {
	std::map<std::string, VariableSymbol*>::iterator vit;
	VariableSymbol *symbol = 0;
	
	vit = variableSymbols.find(ve.name + "_" + ve.function_id);
	if (vit == variableSymbols.end()) {
		symbol = DwarfVariableSymbol::fromEntry(ve);
		variableSymbols[ve.name + "_" + ve.function_id] = symbol;
	} else {
		symbol = vit->second;
	}
	return symbol;
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
			TypeEntry type = vit->type;
			void* vaddr;
			size_t vsize = type.size; 
			if (DwarfMachine::evaluateLocation(context, vit->location, &vaddr, 0) == 0) {
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
	void*				ip;	
	list<VarEntry> 			variables;
	list<VarEntry>::iterator	vit;

	if (context.getInstructionPointer(&ip) != 0) {
		return 2;
	}

	map<string, map<unsigned long long, list<VarEntry> > >::const_iterator rel_it;
	rel_it = relevance.find(fe.unique_id);
	if (rel_it == relevance.end()) {
		cerr << "No relevance map for " << fe.name << endl;
		return 3;
	}

	map<unsigned long long, list<VarEntry> >::const_iterator lrel_it, lrel_it_all;
	lrel_it = rel_it->second.find((unsigned long long) ip);
	lrel_it_all = rel_it->second.find((unsigned long long) -1);

	if (lrel_it != rel_it->second.end()) {
		variables.insert(variables.begin(), lrel_it->second.begin(), lrel_it->second.end());
	}
	if (lrel_it_all != rel_it->second.end()) {
		variables.insert(variables.begin(), lrel_it_all->second.begin(), lrel_it_all->second.end());
	}
	//variables = DwarfIndexer::getVariables(fe);
	for (vit = variables.begin(); vit != variables.end(); vit++) {		
		void* vaddr;
		unsigned int result = DwarfMachine::evaluateLocation(context, vit->location, &vaddr, 0, &fe);
		// variable is in memory
		if (result == 0) {
			TypeEntry type = vit->type;
			size_t vsize = type.size;
			if (addr >= vaddr && addr < vaddr + vsize) {
				*ve = *vit;
				return 0;
			}
		}
		// if variable is not in memory and not in a register, it is an error.
		else if (result != 1) {
			cerr << "Evaluate failed for " << vit->name << " in " << fe.name << endl; 
		}
	}

	cerr << "Lookup local. (" << fe.name << ", " << addr << ") - FAILED" << endl;
	return 1;
}

void DwarfSymbolResolver::setCurrentFunction(const struct FunctionEntry &fe) {
	delete current_function;

	current_function = new FunctionEntry;
	*current_function = fe;
}

const struct FunctionEntry* DwarfSymbolResolver::getCurrentFunction() const {
	return current_function;
}

// TODO warning, this is very inefficient.
void DwarfSymbolResolver::storeLocalVariables(const ExecutionContext &context, const struct FunctionEntry &fe) {
	list<VarEntry> 			variables;
	list<VarEntry>::iterator	vit;

	variables = DwarfIndexer::getVariables(fe);
	for (vit = variables.begin(); vit != variables.end(); vit++) {
		TypeEntry type = vit->type;

		void* vaddr;
		size_t vsize = type.size; 
		if (DwarfMachine::evaluateLocation(context, vit->location, &vaddr, 0, &fe) == 0) {
			for (unsigned int b = 0; b < vsize; b++) {
				void *byte_addr = (void*) &((char*) vaddr)[b];
				cache[byte_addr].push(*vit);
			}
		}
	}
}

void DwarfSymbolResolver::removeLocalVariables(const struct FunctionEntry &fe) {
	map<void*, stack<VarEntry> >::iterator	cache_iterator;
	
	for (cache_iterator = cache.begin(); cache_iterator != cache.end(); cache_iterator++) {
		if (!cache_iterator->second.empty()) {
			VarEntry ve = cache_iterator->second.top();
			if (ve.function_id == fe.unique_id) {
				cache_iterator->second.pop();
			}
		}
	}
}

unsigned int DwarfSymbolResolver::enterFunction(const ExecutionContext &context, void *addr) {
	FunctionEntry	fe;

	if (findFunction(addr, &fe) != 0) {
		return 3;
	}

	cerr << "Entering function " << fe.name << endl;

	const struct FunctionEntry *previous_fe = getCurrentFunction();
	if (previous_fe != 0) {
		storeLocalVariables(context, *previous_fe);
	}
	setCurrentFunction(fe);

	return 1;
}

unsigned int DwarfSymbolResolver::leaveFunction(const ExecutionContext &context, void *addr, void *ret_addr) {
	FunctionEntry	fe;
	FunctionEntry	rfe;

	if (findFunction(addr, &fe) != 0) {
		return 3;
	}

	if (findFunction(ret_addr, &rfe) != 0) {
		return 4;
	}

	cerr << "Leaving function " << fe.name << " to " << rfe.name << endl;
	removeLocalVariables(rfe);
	setCurrentFunction(rfe);

	return 1;
}

unsigned int DwarfSymbolResolver::resolveFunction(const ExecutionContext& context, void *addr, const FunctionSymbol **function) const {
	FunctionEntry fe;	

	if (findFunction(addr, &fe) == 0) {
		*function = toSymbol(fe);
		return 0;
	}

	return 1;
}

unsigned int DwarfSymbolResolver::resolveVariable(const ExecutionContext& context, void *addr, size_t size, const VariableSymbol **variable) const {
	void 		*ip;
	VarEntry 	ve;
	FunctionEntry	fe;

	if (findGlobalVariable(context, addr, size, &ve) == 0) {
		*variable = toSymbol(ve);
		return 0;
	}

	if (context.getInstructionPointer(&ip) != 0) {
		return 2;
	}

	if (findFunction(ip, &fe) == 0) {
		if (findLocalVariable(context, addr, size, fe, &ve) == 0) {
			*variable = toSymbol(ve);
			return 0;
		}
	}
	
	map<void*, stack<VarEntry> >::const_iterator	cache_iterator;

	cache_iterator = cache.find(addr);
	if (cache_iterator != cache.end()) {
		if (!cache_iterator->second.empty()) {
			ve = cache_iterator->second.top();
			*variable = toSymbol(ve);
			return 0;
		}
	}

	return 1;
}
