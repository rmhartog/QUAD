/*****
 *
 * DwarfIndexer.h
 *
 * Author: Reinier M. Hartog
 *
 *****/

#ifndef DWARFINDEXER_H
#define DWARFINDEXER_H

#include "libdwarf.h"
#include <list>
#include <map>
#include <string>

#include "DwarfMachine.h"

enum TypeType {
     TT_Base,
     TT_Typedef,
     TT_Array,
     TT_BoundedArray,
     TT_Pointer,
     TT_Const,
     TT_Struct
};

struct TypeEntry {
     enum TypeType	type;
     std::string	name;
     Dwarf_Off		basetype_off;
     TypeEntry*		basetype_type;
     unsigned long long	size;
     unsigned long long upper_bound;
};

struct VarEntry {
     std::string	name;
     std::string	function_id;
     Dwarf_Off		type_off;
     TypeEntry		type;
     DwarfScriptList	location;
};

struct DwarfIndex {
     std::map<Dwarf_Off, struct TypeEntry*>&		types;
     std::map<Dwarf_Off, struct VarEntry*>&		variables;
     std::map<Dwarf_Off, struct FunctionEntry*>&	functions;
};

struct FunctionEntry {
     std::string 				name;
     std::string				unique_id;
     Dwarf_Off					return_type;
     Dwarf_Addr					lopc;
     Dwarf_Addr					hipc;
     DwarfScriptList				frame_base;

     std::map<Dwarf_Off, struct VarEntry*>	variables;
};

class DwarfIndexer
{
private:
     std::map<Dwarf_Off, struct TypeEntry*>	types;
     std::map<Dwarf_Off, struct VarEntry*>	global_variables;
     std::map<Dwarf_Off, struct FunctionEntry*>	functions;
     Dwarf_Addr					CU_lopc, CU_hipc;
     std::string				current_function_id;
     TypeEntry*					current_basetype;

     static Dwarf_Half		getDwarfTag(Dwarf_Die, Dwarf_Error);
     static std::string		getDwarfName(Dwarf_Die, Dwarf_Debug, Dwarf_Error);
     static Dwarf_Off		getDwarfOffset(Dwarf_Die, Dwarf_Debug, Dwarf_Error);
     static Dwarf_Unsigned	getDwarfUnsigned(Dwarf_Die, Dwarf_Half, Dwarf_Debug, Dwarf_Error);
     static Dwarf_Off		getDwarfRefOffset(Dwarf_Die, Dwarf_Half, Dwarf_Debug, Dwarf_Error);
     static unsigned int	getDwarfPC(Dwarf_Die, Dwarf_Addr*, Dwarf_Addr*, Dwarf_Error);
     static unsigned int 	getDwarfScriptList(Dwarf_Die, Dwarf_Half, DwarfScriptList &, Dwarf_Debug, Dwarf_Error, unsigned long long);
     static unsigned int	getChildren(Dwarf_Die, std::list<Dwarf_Die>&, Dwarf_Debug, Dwarf_Error);

     void			fixIndirectType(struct TypeEntry&);
     void			fixIndirectTypes();
     void			fixIndirectVariableTypes();
     void			fixIndirectLocalVariableTypes(const struct FunctionEntry&);
     void			fixIndirectVariableType(struct VarEntry&);
public:
     DwarfIndexer();
     ~DwarfIndexer();

     unsigned int accept(Dwarf_Debug, Dwarf_Error);
     unsigned int accept(DwarfIndex&, Dwarf_Die, Dwarf_Debug, Dwarf_Error);
     unsigned int acceptChildren(DwarfIndex&, Dwarf_Die, Dwarf_Debug, Dwarf_Error);
     unsigned int visitCU(DwarfIndex&, Dwarf_Die, Dwarf_Debug, Dwarf_Error);
     unsigned int visitBaseType(DwarfIndex&, Dwarf_Die, Dwarf_Debug, Dwarf_Error);
     unsigned int visitTypedefType(DwarfIndex&, Dwarf_Die, Dwarf_Debug, Dwarf_Error);
     unsigned int visitArrayType(DwarfIndex&, Dwarf_Die, Dwarf_Debug, Dwarf_Error);
     unsigned int visitSubrangeType(DwarfIndex&, Dwarf_Die, Dwarf_Debug, Dwarf_Error);
     unsigned int visitPointerType(DwarfIndex&, Dwarf_Die, Dwarf_Debug, Dwarf_Error);
     unsigned int visitConstType(DwarfIndex&, Dwarf_Die, Dwarf_Debug, Dwarf_Error);
     unsigned int visitStructureType(DwarfIndex&, Dwarf_Die, Dwarf_Debug, Dwarf_Error);
     unsigned int visitVariable(DwarfIndex&, Dwarf_Die, Dwarf_Debug, Dwarf_Error);
     unsigned int visitSubProgram(DwarfIndex&, Dwarf_Die, Dwarf_Debug, Dwarf_Error);

     unsigned int		getType(Dwarf_Off, TypeEntry*)		const;
     std::list<VarEntry>	getVariables()				const;
     static std::list<VarEntry>	getVariables(const FunctionEntry&);
     std::list<FunctionEntry>	getFunctions()				const;
};

#endif // DWARFINDEXER_H
