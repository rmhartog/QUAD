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

struct TypeEntry {
     std::string	name;
};

struct VarEntry {
     std::string	name;
     Dwarf_Off		type;
     DwarfScriptList	location;
};

struct DwarfIndex {
     std::map<Dwarf_Off, struct TypeEntry*>&		types;
     std::map<Dwarf_Off, struct VarEntry*>&		variables;
     std::map<Dwarf_Off, struct FunctionEntry*>&	functions;
};

struct FunctionEntry {
     std::string 				name;
     Dwarf_Off					return_type;
     Dwarf_Addr					lopc;
     Dwarf_Addr					hipc;

     std::map<Dwarf_Off, struct VarEntry*>	variables;
};

class DwarfIndexer
{
private:
     std::map<Dwarf_Off, struct TypeEntry*>	types;
     std::map<Dwarf_Off, struct VarEntry*>	global_variables;
     std::map<Dwarf_Off, struct FunctionEntry*>	functions;

     static Dwarf_Half		getDwarfTag(Dwarf_Die, Dwarf_Error);
     static std::string		getDwarfName(Dwarf_Die, Dwarf_Debug, Dwarf_Error);
     static Dwarf_Off		getDwarfOffset(Dwarf_Die, Dwarf_Debug, Dwarf_Error);
     static Dwarf_Off		getDwarfRefOffset(Dwarf_Die, Dwarf_Half, Dwarf_Debug, Dwarf_Error);
     static unsigned int	getDwarfPC(Dwarf_Die, Dwarf_Addr*, Dwarf_Addr*, Dwarf_Error);
     static unsigned int 	getDwarfScriptList(Dwarf_Die, Dwarf_Half, DwarfScriptList &, Dwarf_Debug, Dwarf_Error);
     static unsigned int	getChildren(Dwarf_Die, std::list<Dwarf_Die>&, Dwarf_Debug, Dwarf_Error);
public:
     DwarfIndexer();
     ~DwarfIndexer();

     unsigned int accept(Dwarf_Debug, Dwarf_Error);
     unsigned int accept(DwarfIndex&, Dwarf_Die, Dwarf_Debug, Dwarf_Error);
     unsigned int acceptChildren(DwarfIndex&, Dwarf_Die, Dwarf_Debug, Dwarf_Error);
     unsigned int visitCU(DwarfIndex&, Dwarf_Die, Dwarf_Debug, Dwarf_Error);
     unsigned int visitBaseType(DwarfIndex&, Dwarf_Die, Dwarf_Debug, Dwarf_Error);
     unsigned int visitVariable(DwarfIndex&, Dwarf_Die, Dwarf_Debug, Dwarf_Error);
     unsigned int visitSubProgram(DwarfIndex&, Dwarf_Die, Dwarf_Debug, Dwarf_Error);

     std::list<VarEntry>	getVariables()				const;
     static std::list<VarEntry>	getVariables(const FunctionEntry&);
     std::list<FunctionEntry>	getFunctions()				const;
};

#endif // DWARFINDEXER_H
