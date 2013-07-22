/*****
 *
 * SymbolResolver.h
 *
 * Author: Reinier M. Hartog
 * Date:   19 November 2012
 *
 * This header dictates the basic interface for a class that resolves addresses to symbols.
 *
 *****/

#ifndef SYMBOLRESOLVER_H
#define SYMBOLRESOLVER_H

#include "ExecutionContext.h"
#include "Symbols.h"

class SymbolResolver
{
public:
     virtual unsigned int enterFunction(void *addr)												= 0;
     virtual unsigned int leaveFunction(void *addr, void *ret_addr)										= 0;
     // this method resolves a function from an address within the target,
     // given an ExecutionContext of the target. On success, a pointer to the resolved function will be stored in *function.
     // returns zero on success, non-zero on failure.
     // NOTE: if the context is modified while this method is executing, the result will be undefined.
     virtual unsigned int resolveFunction(const ExecutionContext& context, void *addr, const FunctionSymbol **function)			const	= 0;
     // this method resolves a variable from an address-size pair within the target,
     // given an ExecutionContext of the target. On success, a pointer to the resolved variable will be stored in *variable.
     // returns zero on success, non-zero on failure.
     // NOTE: if the context is modified while this method is executing, the result will be undefined.
     virtual unsigned int resolveVariable(const ExecutionContext& context, void *addr, size_t size, const VariableSymbol **variable)	const	= 0;
};

#endif // SYMBOLRESOLVER_H
