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

#include <map>
#include <string>
#include <stack>
#include <list>

class DwarfSymbolResolver : public SymbolResolver
{
private:
     Dwarf_Debug					dwarf_handle;
     Dwarf_Error					dwarf_error;
     class DwarfIndexer*				indexer;
     struct FunctionEntry*				current_function;
     std::map<void*, std::stack<struct VarEntry> >	cache;

     mutable std::map<std::string, class FunctionSymbol*>	functionSymbols;
     mutable std::map<std::string, class VariableSymbol*>	variableSymbols;
     std::map<std::string, std::map<unsigned long long, std::list<VarEntry> > >	relevance;

     DwarfSymbolResolver();
     ~DwarfSymbolResolver();

     Dwarf_Debug*	getDwarfHandle();
     Dwarf_Error*	getDwarfError();
     void		createIndexer();
     std::map<unsigned long long, std::list<VarEntry> >	createRelevanceMap(const struct FunctionEntry &);
     void		createRelevanceMaps();
public:
     static unsigned int createDwarfSymbolResolver(Elf *, DwarfSymbolResolver **);
     static unsigned int destroyDwarfSymbolResolver(DwarfSymbolResolver **);

private:
     const class FunctionSymbol *toSymbol(const struct FunctionEntry &fe) const;
     const class VariableSymbol *toSymbol(const struct VarEntry &ve) const;

     unsigned int findFunction(void *addr, struct FunctionEntry *fe)													const;
     unsigned int findGlobalVariable(const class ExecutionContext &context, void *addr, size_t size, struct VarEntry *ve)						const; 
     unsigned int findLocalVariable(const class ExecutionContext &context, void *addr, size_t size, const struct FunctionEntry &fe, struct VarEntry *ve)		const;

     void setCurrentFunction(const struct FunctionEntry &fe);
     const struct FunctionEntry* getCurrentFunction()															const;

     void storeLocalVariables(const class ExecutionContext &context, const struct FunctionEntry &fe);
     void removeLocalVariables(const struct FunctionEntry &fe);
public:
     virtual unsigned int enterFunction(const class ExecutionContext &context, void *addr);
     virtual unsigned int leaveFunction(const class ExecutionContext &context, void *addr, void *ret_addr);
     // this method resolves a function from an address within the target,
     // given an ExecutionContext of the target. On success, a pointer to the resolved function will be stored in *function.
     // returns zero on success, non-zero on failure.
     // NOTE: if the context is modified while this method is executing, the result will be undefined.
     virtual unsigned int resolveFunction(const class ExecutionContext &context, void *addr, const class FunctionSymbol **function)					const;
     // this method resolves a variable from an address-size pair within the target,
     // given an ExecutionContext of the target. On success, a pointer to the resolved variable will be stored in *variable.
     // returns zero on success, non-zero on failure.
     // NOTE: if the context is modified while this method is executing, the result will be undefined.
     virtual unsigned int resolveVariable(const class ExecutionContext &context, void *addr, size_t size, const class VariableSymbol **variable)			const;
};

#endif // DWARFSYMBOLRESOLVER_H
