/*****
 *
 * Symbols.h
 *
 * Author: Reinier M. Hartog
 * Date:   19 November 2012
 *
 * This header dictates the basic interface for a class that represents a symbol within the target application.
 *
 *****/

#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "ExecutionContext.h"

#include <unistd.h>	

// the type of location a variable is stored in,
// this can be either in a register or in memory.
// if it is in a register, it is currently of no interest to us.
// NOTE: DWARF has support for 'pieces' of variables in different places,
//       this can currently not be encoded.
enum LocationType
{
     LT_Register,
     LT_Memory
};

// this structure provides information about the location of a variable.
struct Location {
     enum LocationType	location_type;
     union {
     	char* 		addr;
	enum eRegister 	reg;
     };
};

// this enumeration describes the way the value of this type is encoded in memory.
enum EncodingType {
	ET_Unknown,
	ET_Address,
	ET_Boolean,
	ET_Float,
	ET_Signed,
	ET_Signed_Char,
	ET_Unsigned,
	ET_Unsigned_Char
};

const size_t	UnknownSize = -1;

// this class will provide type information about variables and function return types.
class Type {
public:
     virtual ~Type() {}

     virtual unsigned int getName(char *buffer, size_t size)		const = 0;
     virtual unsigned int getStorageType(enum EncodingType *type)	const = 0;
     virtual unsigned int getSize(size_t *size)				const = 0;
};

class FunctionSymbol {
public:
     virtual ~FunctionSymbol() {}

     virtual unsigned int getName(char *buffer, size_t size)		const = 0;
     // virtual unsigned int getSourceLocation(...) 			const = 0;

     //virtual unsigned int getReturnType(const Type **type)		const = 0;
     //virtual unsigned int getAddress(const void **addr)			const = 0;
};

class VariableSymbol {
public:
     virtual ~VariableSymbol() {}

     virtual VariableSymbol* clone()					= 0;
     virtual const VariableSymbol *clone()				const = 0;

     virtual unsigned int getName(char *buffer, size_t size)		const = 0;
     // virtual unsigned int getSourceLocation(...) 			const = 0;

     //virtual unsigned int getType(const Type **type)			const = 0;
     //virtual unsigned int getLocation(const Location *loc)		const = 0;
};

#endif // SYMBOLS_H
