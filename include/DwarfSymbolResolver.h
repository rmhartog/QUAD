/*****
 *
 * DwarfSymbolResolver.h
 *
 * Author: Reinier M. Hartog
 * Date:   19 November 2012
 *
 * An implementation of SymbolResolver based on DWARF-information.
 *
 *****/

#ifndef DWARFSYMBOLRESOLVER_H
#define DWARFSYMBOLRESOLVER_H

#include "libdwarf.h"
#include "SymbolResolver.h"
#include "ExecutionContext.h"
#include "Symbols.h"

class DwarfSymbolResolver : public SymbolResolver
{
private:
     Dwarf_Debug		dwarf_handle;
     Dwarf_Error		dwarf_error;
     class DwarfIndexer*	indexer;
     struct FunctionEntry*	current_function;
     class SymbolCache*		cache;

     DwarfSymbolResolver();
     ~DwarfSymbolResolver();

     Dwarf_Debug*	getDwarfHandle();
     Dwarf_Error*	getDwarfError();
     void		createIndexer();
public:
     static unsigned int createDwarfSymbolResolver(Elf *, DwarfSymbolResolver **);
     static unsigned int destroyDwarfSymbolResolver(DwarfSymbolResolver **);

private:
     unsigned int findFunction(void *addr, struct FunctionEntry *fe)												const;
     unsigned int findGlobalVariable(const ExecutionContext &context, void *addr, size_t size, struct VarEntry *ve)						const; 
     unsigned int findLocalVariable(const ExecutionContext &context, void *addr, size_t size, const struct FunctionEntry &fe, struct VarEntry *ve)		const;
public:
     virtual unsigned int enterFunction(void *addr);
     virtual unsigned int leaveFunction(void *addr, void *ret_addr);
     // this method resolves a function from an address within the target,
     // given an ExecutionContext of the target. On success, a pointer to the resolved function will be stored in *function.
     // returns zero on success, non-zero on failure.
     // NOTE: if the context is modified while this method is executing, the result will be undefined.
     virtual unsigned int resolveFunction(const ExecutionContext &context, void *addr, const FunctionSymbol **function)			const;
     // this method resolves a variable from an address-size pair within the target,
     // given an ExecutionContext of the target. On success, a pointer to the resolved variable will be stored in *variable.
     // returns zero on success, non-zero on failure.
     // NOTE: if the context is modified while this method is executing, the result will be undefined.
     virtual unsigned int resolveVariable(const ExecutionContext &context, void *addr, size_t size, const VariableSymbol **variable)	const;
};

#endif // DWARFSYMBOLRESOLVER_H
