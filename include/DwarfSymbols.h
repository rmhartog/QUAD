/*****
 *
 * DwarfSymbols.h
 *
 * Author: Reinier M. Hartog
 * Date:   6 August 2013
 *
 *****/

#ifndef DWARFSYMBOLS_H
#define DWARFSYMBOLS_H

#include "Symbols.h"
#include "DwarfIndexer.h"

class DwarfType : public Type
{
private:
     TypeEntry	entry;
     
     DwarfType(const TypeEntry &te);
public:
     ~DwarfType();

     static DwarfType *fromEntry(const TypeEntry &te);

     virtual unsigned int getName(char *buffer, size_t size)		const;
     virtual unsigned int getStorageType(enum EncodingType *type)	const;
     virtual unsigned int getSize(size_t *size)				const;
};

class DwarfFunctionSymbol : public FunctionSymbol
{
private:
     FunctionEntry	entry;
     
     DwarfFunctionSymbol(const FunctionEntry &fe);
public:
     ~DwarfFunctionSymbol();

     static DwarfFunctionSymbol *fromEntry(const FunctionEntry &fe);

     virtual unsigned int getName(char *buffer, size_t size)		const;
     // virtual unsigned int getSourceLocation(...) 			const = 0;

     //virtual unsigned int getReturnType(const Type **type)		const;
     //virtual unsigned int getAddress(const void **addr)		const = 0;
};

class DwarfVariableSymbol : public VariableSymbol
{
private:
     VarEntry	entry;
     
     DwarfVariableSymbol(const VarEntry &ve);
public:
     ~DwarfVariableSymbol();

     virtual VariableSymbol* clone();
     virtual const VariableSymbol *clone()				const;

     static DwarfVariableSymbol *fromEntry(const VarEntry &ve);

     virtual unsigned int getName(char *buffer, size_t size)		const;
};

#endif // DWARFSYMBOLS_H
