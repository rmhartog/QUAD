/*****
 *
 * ElfSymbolResolver.h
 *
 * Author: Reinier M. Hartog, Vlad-Mihai Sima
 * Date:   19 November 2012
 *
 * An implementation of SymbolResolver based on ELF-information.
 * This implementation is based on the code by Vlad-Mihai Sima.
 *
 *****/

#ifndef ELFSYMBOLRESOLVER_H
#define ELFSYMBOLRESOLVER_H

#include <vector>
using namespace std;

#include "SymbolResolver.h"
#include "Symbols.h"

class ElfFunctionSymbol : public FunctionSymbol
{
private:
     char 	symbol_name[128];
     void	*symbol_address;
     size_t	symbol_size;
public:
		ElfFunctionSymbol(const ElfFunctionSymbol&);
		ElfFunctionSymbol(const char *name, void* addr, size_t size);
     virtual	~ElfFunctionSymbol();

     unsigned int getAddressRange(void **low, void **high)	const;

     virtual unsigned int getName(char *buffer, size_t size)	const;
     virtual unsigned int getReturnType(const Type **type)	const;
};

class ElfVariableSymbol : public VariableSymbol
{
private:
     char 	symbol_name[128];
     void	*symbol_address;
     size_t	symbol_size;
public:
		ElfVariableSymbol(const ElfVariableSymbol&);
		ElfVariableSymbol(const char *name, void* addr, size_t size);
     virtual	~ElfVariableSymbol();

     virtual VariableSymbol *clone();
     virtual const VariableSymbol *clone()			const;

     unsigned int getAddressRange(void **low, void **high)	const;

     virtual unsigned int getName(char *buffer, size_t size)	const;
     virtual unsigned int getType(const Type **type)		const;
};

class ElfSymbolResolver
{
private:
     vector<ElfFunctionSymbol*> functions;
     vector<ElfVariableSymbol*> variables;

     unsigned int addFunction(ElfFunctionSymbol *fs);
     unsigned int addVariable(ElfVariableSymbol *vs);

    		 ElfSymbolResolver();
public:
     virtual 	~ElfSymbolResolver();

     static unsigned int createElfSymbolResolver(ElfSymbolResolver **resolver, const char *filename);

     virtual unsigned int resolveFunction(const ExecutionContext& context, void *addr, const FunctionSymbol **function)			const;
     virtual unsigned int resolveVariable(const ExecutionContext& context, void *addr, size_t size, const VariableSymbol **variable)	const;
};

#endif // ELFSYMBOLRESOLVER_H
