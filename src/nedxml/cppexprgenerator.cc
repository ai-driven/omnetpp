//==========================================================================
//   CPPEXPRGENERATOR.CC
//            part of OMNeT++
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 2002-2004 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/


#include <stdio.h>
#include <assert.h>
#include "nedcompiler.h"
#include "cppexprgenerator.h"
#include "nederror.h"


inline bool strnotnull(const char *s)
{
    return s && s[0];
}

// static struct { char *fname; int args; } known_funcs[] =
// {
//    /* <math.h> */
//    {"fabs", 1},    {"fmod", 2},
//    {"acos", 1},    {"asin", 1},    {"atan", 1},   {"atan2", 1},
//    {"sin", 1},     {"cos", 1},     {"tan", 1},    {"hypot", 2},
//    {"ceil", 1},    {"floor", 1},
//    {"exp", 1},     {"pow", 2},     {"sqrt", 1},
//    {"log",  1},    {"log10", 1},
//
//    /* OMNeT++ */
//    {"uniform", 2},      {"intuniform", 2},       {"exponential", 1},
//    {"normal", 2},       {"truncnormal", 2},
//    {"genk_uniform", 3}, {"genk_intuniform",  3}, {"genk_exponential", 2},
//    {"genk_normal", 3},  {"genk_truncnormal", 3},
//    {"min", 2},          {"max", 2},
//
//    /* OMNeT++, to support expressions */
//    {"bool_and",2}, {"bool_or",2}, {"bool_xor",2}, {"bool_not",1},
//    {"bin_and",2},  {"bin_or",2},  {"bin_xor",2},  {"bin_compl",1},
//    {"shift_left",2}, {"shift_right",2},
//    {NULL,0}
// };

static ParamNode *getParameterDecl(NEDElement *moduletypedecl, const char *paramName)
{
    if (!moduletypedecl)
        return NULL;
    ParamsNode *paramsdecl = (ParamsNode *)moduletypedecl->getFirstChildWithTag(NED_PARAMS);
    if (!paramsdecl)
        return NULL;
    return (ParamNode *) paramsdecl->getFirstChildWithAttribute(NED_PARAM, "name", paramName);
}

static bool isParameterConst(NEDElement *moduletypedecl, const char *paramName)
{
    ParamNode *param = getParameterDecl(moduletypedecl, paramName);
    if (!param)
        return false;
    return strstr(param->getDataType(), "const")!=NULL;
}


int CppExpressionGenerator::count = 0;

CppExpressionGenerator::CppExpressionGenerator(ostream& _out, NEDSymbolTable *_symboltable) :
  out(_out), symboltable(_symboltable)
{
}

void CppExpressionGenerator::collectExpressions(NEDElement *node)
{
    exprMap.clear();
    doCollectExpressions(node, NULL);
}

void CppExpressionGenerator::doCollectExpressions(NEDElement *node, NEDElement *currentSubmodTypeDecl)
{
    for (NEDElement *child=node->getFirstChild(); child; child = child->getNextSibling())
    {
        if (child->getTagCode()==NED_EXPRESSION)
        {
            // stop recursion at expressions: embedded expressions are merged into their parents
            if (needsExpressionClass((ExpressionNode *)child, currentSubmodTypeDecl))
                collectExpressionInfo((ExpressionNode *)child, currentSubmodTypeDecl);
        }
        else if (child->getTagCode()==NED_SUBMODULE)
        {
            // get submodule's type decl from symbol table -- we need it for expression generation
            const char *typeName = ((SubmoduleNode *)child)->getTypeName();
            NEDElement *thisSubmodTypeDecl = symboltable->getModuleDeclaration(typeName);
            if (!thisSubmodTypeDecl)
                INTERNAL_ERROR1(node,"doCollectExpressions(): module type not found: %s", typeName);

            doCollectExpressions(child, thisSubmodTypeDecl);
        }
        else
        {
            doCollectExpressions(child, currentSubmodTypeDecl);
        }
    }
}

bool CppExpressionGenerator::needsExpressionClass(ExpressionNode *expr, NEDElement *currentSubmodTypeDecl)
{
    // if simple expr, no expression class necessary
    NEDElement *node = expr->getFirstChild();
    if (!node)  {INTERNAL_ERROR0(expr, "needsExpressionClass(): empty expression");return false;}
    int tag = node->getTagCode();

    // only non-const parameter assignments and channel attrs need expression classes (?)
    int parenttag = expr->getParent()->getTagCode();
    if (parenttag!=NED_SUBSTPARAM && parenttag!=NED_CHANNEL_ATTR && parenttag!=NED_CONN_ATTR)
        return false;

    // also: if the parameter is non-volatile (that is, const), we don't need expr class
    if (parenttag==NED_SUBSTPARAM && isParameterConst(currentSubmodTypeDecl, ((SubstparamNode *)expr->getParent())->getName()))
        return false;

    // identifiers and constants never need expression classes
    if (tag==NED_IDENT || tag==NED_CONST)
        return false;

    // also, non-ref bare parameter references never need expression classes
    if (tag==NED_PARAM_REF && !((ParamRefNode *)node)->getIsRef())
        return false;

    // special functions (INPUT, INDEX, SIZEOF) may also go without expression classes
    if (tag==NED_FUNCTION)
    {
        const char *funcname = ((FunctionNode *)node)->getName();
        if (!strcmp(funcname,"index"))
            return false;
        if (!strcmp(funcname,"sizeof"))
            return false;
        if (!strcmp(funcname,"input"))
        {
            NEDElement *op1 = node->getFirstChild();
            NEDElement *op2 = op1 ? op1->getNextSibling() : NULL;
            if ((!op1 || op1->getTagCode()==NED_CONST) && (!op2 || op2->getTagCode()==NED_CONST))
                return false;
        }
        if (!strcmp(funcname,"xmldoc"))
            return false;
    }
    return true;
}

void CppExpressionGenerator::collectExpressionInfo(ExpressionNode *expr, NEDElement *currentSubmodTypeDecl)
{
    char buf[32];
    sprintf(buf,"Expr%d",count++);
    NEDElement *ctx = expr;
    while (ctx->getTagCode()!=NED_CHANNEL &&
           ctx->getTagCode()!=NED_SIMPLE_MODULE &&
           ctx->getTagCode()!=NED_COMPOUND_MODULE &&
           ctx->getTagCode()!=NED_NETWORK &&
           ctx->getParent())
    {
        ctx = ctx->getParent();
    }

    ExpressionInfo info;
    info.expr = expr;
    info.name = buf;
    info.ctxtype = ctx ? ctx->getTagCode() : 0;
    info.submoduleTypeDecl = currentSubmodTypeDecl;
    doExtractArgs(info, expr);
    exprMap[expr] = info;
}

void CppExpressionGenerator::doExtractArgs(ExpressionInfo& info, NEDElement *node)
{
    for (NEDElement *child=node->getFirstChild(); child; child = child->getNextSibling())
    {
        bool isctorarg = false, iscachedvar = false;
        int tag = child->getTagCode();
        if (tag==NED_IDENT)
            isctorarg = true;
        else if (tag==NED_FUNCTION && !strcmp(((FunctionNode *)child)->getName(),"index"))
             isctorarg = true;
        else if (tag==NED_FUNCTION && !strcmp(((FunctionNode *)child)->getName(),"sizeof"))
             isctorarg = true;
        else if (tag==NED_FUNCTION)
             iscachedvar = true;

        if (isctorarg)
            info.ctorargs.push_back(child);
        if (iscachedvar)
            info.cachedvars.push_back(child);
        doExtractArgs(info, child);
    }
}

void CppExpressionGenerator::generateExpressionClass(ExpressionInfo& info)
{
    const char *classname = info.name.c_str();
    NEDElementVector::iterator i;

    // generate expression class
    const char *srcloc = info.expr->getSourceLocation();
    out << "// evaluator for expression at " << (srcloc ? srcloc : "(no source info)") << "\n";
    out << "class " << classname << " : public cDoubleExpression\n";
    out << "{\n";
    out << "  private:\n";
    if (info.ctxtype==NED_SIMPLE_MODULE || info.ctxtype==NED_COMPOUND_MODULE)
        out << "    cModule *mod;\n";
    else if (info.ctxtype==NED_CHANNEL)
        out << "    // cChannel *channel;\n";
    // variables to hold arguments (external references) for the expression
    for (i=info.ctorargs.begin(); i!=info.ctorargs.end(); ++i)
        out << "    " << getTypeForArg(*i) << " " << getNameForArg(*i) << (*i)->getId() << ";\n";
    for (i=info.cachedvars.begin(); i!=info.cachedvars.end(); ++i)
        out << "    " << getTypeForArg(*i) << " " << getNameForArg(*i) << (*i)->getId() << ";\n";
    out << "  public:\n";

    // constructor:
    out << "    " << classname << "(";
    if (info.ctxtype==NED_SIMPLE_MODULE || info.ctxtype==NED_COMPOUND_MODULE)
        out << "cModule *mod";
    else
        out << "void *";
    for (i=info.ctorargs.begin(); i!=info.ctorargs.end(); ++i)
        out << ", " << getTypeForArg(*i) << " " << getNameForArg(*i) << (*i)->getId();
    out << ")  {\n";
    if (info.ctxtype==NED_SIMPLE_MODULE || info.ctxtype==NED_COMPOUND_MODULE)
        out << "        this->mod=mod;\n";
    for (i=info.ctorargs.begin(); i!=info.ctorargs.end(); ++i)
        out << "        this->" << getNameForArg(*i) << (*i)->getId() << "=" << getNameForArg(*i) << (*i)->getId() << ";\n";
    for (i=info.cachedvars.begin(); i!=info.cachedvars.end(); ++i)
    {
        out << "        this->" << getNameForArg(*i) << (*i)->getId() << "=";
        doValueForCachedVar(*i);
        out << ";\n";
    }
    out << "    }\n";

    // dup()
    out << "    cExpression *dup()  {return new " << classname << "(";
    if (info.ctxtype==NED_SIMPLE_MODULE || info.ctxtype==NED_COMPOUND_MODULE)
        out << "mod";
    else
        out << "NULL";
    for (i=info.ctorargs.begin(); i!=info.ctorargs.end(); ++i)
        out << ", " << getNameForArg(*i) << (*i)->getId();
    out << ");}\n";

    // expression method:
    out << "    double evaluate()  {return ";
    generateChildren(info.expr, "               ", MODE_EXPRESSION_CLASS);
    out << ";}\n";
    out << "};\n\n";
}

const char *CppExpressionGenerator::getTypeForArg(NEDElement *node)
{
    if (node->getTagCode()==NED_IDENT)
        return "long";
    else if (node->getTagCode()==NED_FUNCTION && !strcmp(((FunctionNode *)node)->getName(),"index"))
        return "long";
    else if (node->getTagCode()==NED_FUNCTION && !strcmp(((FunctionNode *)node)->getName(),"sizeof"))
        return "long"; // FIXME cModule* or cGate* ?
    else if (node->getTagCode()==NED_FUNCTION)
    {
        const char *types[] = {"MathFuncNoArg", "MathFunc1Arg", "MathFunc2Args",
                               "MathFunc3Args", "MathFunc4Args"};
        int argcount = node->getNumChildren();
        return types[argcount];
    }
    else
        {INTERNAL_ERROR1(node, "getTypeForArg(): unexpected tag '%s'", node->getTagName());return NULL;}
}

const char *CppExpressionGenerator::getNameForArg(NEDElement *node)
{
    if (node->getTagCode()==NED_IDENT)
        return ((IdentNode *)node)->getName();
    else if (node->getTagCode()==NED_FUNCTION)
        return ((FunctionNode *)node)->getName(); //FIXME!!!
    else
        {INTERNAL_ERROR1(node, "getNameForArg(): unexpected tag '%s'", node->getTagName());return NULL;}
}

void CppExpressionGenerator::doValueForArg(NEDElement *node)
{
    if (node->getTagCode()==NED_IDENT)
        out << ((IdentNode *)node)->getName() << "_var";
    else if (node->getTagCode()==NED_FUNCTION && !strcmp(((FunctionNode *)node)->getName(),"index"))
        out << "submodindex";
    else if (node->getTagCode()==NED_FUNCTION && !strcmp(((FunctionNode *)node)->getName(),"sizeof"))
        out << "sizeof(...)"; //FIXME
    else
        {INTERNAL_ERROR1(node, "doValueForArg(): unexpected tag '%s'", node->getTagName());}
}

void CppExpressionGenerator::doValueForCachedVar(NEDElement *node)
{
    if (node->getTagCode()==NED_FUNCTION)
    {
        const char *funcname = ((FunctionNode *)node)->getName();
        int argcount = ((FunctionNode *)node)->getNumChildren();
        const char *methods[] = {"mathFuncNoArg()", "mathFunc1Arg()", "mathFunc2Args()",
                                 "mathFunc3Args()", "mathFunc4Args()"};
        out << "_getFunction(\"" << funcname << "\"," << argcount << ")->" << methods[argcount];
    }
    else
        {INTERNAL_ERROR1(node, "doValueForArg(): unexpected tag '%s'", node->getTagName());}
}

void CppExpressionGenerator::generateExpressionClasses()
{
   if (!exprMap.empty())
   {
       out << "namespace {  // unnamed namespace to avoid name clashes\n\n";
       for (NEDExpressionMap::iterator i=exprMap.begin(); i!=exprMap.end(); ++i)
           generateExpressionClass((*i).second);
       out << "}  // end unnamed namespace\n\n";
   }
}

void CppExpressionGenerator::generateExpressionUsage(ExpressionNode *expr, const char *indent)
{
    // find name in hash table
    NEDExpressionMap::iterator e = exprMap.find(expr);
    if (e==exprMap.end())
    {
        // inline expression
        generateChildren(expr, indent, MODE_INLINE_EXPRESSION);
    }
    else
    {
        // use generated expression class
        ExpressionInfo& info = (*e).second;
        const char *classname = info.name.c_str();
        out << "tmpval.setDoubleValue(new " << classname << "(";
        if (info.ctxtype==NED_SIMPLE_MODULE || info.ctxtype==NED_COMPOUND_MODULE)
            out << "mod";
        else
            out << "NULL";
        for (NEDElementVector::iterator i=info.ctorargs.begin(); i!=info.ctorargs.end(); ++i)
        {
            out << ", ";
            doValueForArg((*i));
        }
        out << "))";
    }
}

void CppExpressionGenerator::generateChildren(NEDElement *node, const char *indent, int mode)
{
    for (NEDElement *child=node->getFirstChild(); child; child = child->getNextSibling())
    {
        generateItem(child, indent, mode);
    }
}

void CppExpressionGenerator::generateItem(NEDElement *node, const char *indent, int mode)
{
    int tagcode = node->getTagCode();
    switch (tagcode)
    {
        case NED_OPERATOR:
            doOperator((OperatorNode *)node, indent, mode); break;
        case NED_FUNCTION:
            doFunction((FunctionNode *)node, indent, mode); break;
        case NED_PARAM_REF:
            doParamref((ParamRefNode *)node, indent, mode); break;
        case NED_IDENT:
            doIdent((IdentNode *)node, indent, mode); break;
        case NED_CONST:
            doConst((ConstNode *)node, indent, mode); break;
        case NED_EXPRESSION:
            doExpression((ExpressionNode *)node, indent, mode); break;
        default:
            INTERNAL_ERROR1(node,"generateItem(): unexpected tag %s", node->getTagName());
    }
}

void CppExpressionGenerator::doOperator(OperatorNode *node, const char *indent, int mode)
{
    NEDElement *op1 = node->getFirstChild();
    NEDElement *op2 = op1 ? op1->getNextSibling() : NULL;
    NEDElement *op3 = op2 ? op2->getNextSibling() : NULL;

    if (!op2)
    {
        // unary.
        const char *name = node->getName();
        bool ulongcast = !strcmp(name,"~");
        out << name << (ulongcast ? "(unsigned long)(" : "(double)(");
        generateItem(op1,indent,mode);
        out << ")";
    }
    else if (!op3)
    {
        // binary.
        const char *name = node->getName();
        if (!strcmp(name,"^"))
        {
            out << "pow((double)";
            generateItem(op1,indent,mode);
            out << ",(double)";
            generateItem(op2,indent,mode);
            out << ")";
        }
        else
        {
            // determine name of C/C++ operator
            const char *clangoperator = name;
            if (!strcmp(name,"#"))
                clangoperator = "^";
            if (!strcmp(name,"##"))
                clangoperator = "!=";  // use "!=" on bools for logical xor

            // we may need to cast operands to bool or unsigned long
            // with %, both operands has to be long (C++ rule)
            bool boolcast = !strcmp(name,"&&") || !strcmp(name,"||") || !strcmp(name,"##");
            bool ulongcast = !strcmp(name,"&") || !strcmp(name,"|") || !strcmp(name,"#") ||
                             !strcmp(name,"<<") || !strcmp(name,">>") || !strcmp(name,"~");
            bool longcast = !strcmp(name,"%");

            // always put parens to force NED precedence (might be different from C++'s)
            out << (boolcast ? "(bool)(" : ulongcast ? "(unsigned long)(" : longcast ? "(long)(" : "(double)(");
            generateItem(op1,indent,mode);
            out << ")";
            //out << (boolcast || ulongcast || longcast ? ")" : "");
            out << clangoperator;
            out << (boolcast ? "(bool)(" : ulongcast ? "(unsigned long)(" : longcast ? "(long)(" : "(double)(");
            generateItem(op2,indent,mode);
            out << ")";
            //out << (boolcast || ulongcast || longcast ? ")" : "");
        }
    }
    else
    {
        // tertiary can only be "?:"
        out << "(bool)(";
        generateItem(op1,indent,mode);
        out << ") ? (double)(";
        generateItem(op2,indent,mode);
        out << ") : (double)(";
        generateItem(op3,indent,mode);
        out << ")";
    }
}

void CppExpressionGenerator::doFunction(FunctionNode *node, const char *indent, int mode)
{
    // get function name, arg count, args
    const char *funcname = node->getName();
    int argcount = node->getNumChildren();

    // operators should be handled specially
    if (!strcmp(funcname,"index"))
    {
        // validation code should ensure this only occurs within a submodule vector
        if (mode==MODE_INLINE_EXPRESSION)
            out << "submodindex";
        else
            out << "index" << node->getId();
        return;
    }
    else if (!strcmp(funcname,"sizeof"))
    {
        IdentNode *op1 = node->getFirstIdentChild();
        assert(op1);
        const char *name = op1->getName();

        // we must decide if "name" is a gate or a submodule
        bool isgate = false;
        bool issubmod = false;
        NEDElement *mod1 = node;
        while (mod1->getTagCode()!=NED_COMPOUND_MODULE && mod1->getParent())
            mod1 = mod1->getParent();
        if (!mod1)
            INTERNAL_ERROR0(node, "sizeof can only be within a compound module declaration");
        CompoundModuleNode *mod = (CompoundModuleNode *)mod1;
        GatesNode *gates = mod->getFirstGatesChild();
        if (gates && gates->getFirstChildWithAttribute(NED_GATE,"name",name))
            isgate = true;

        if (!isgate)
        {
            SubmodulesNode *submods = mod->getFirstSubmodulesChild();
            if (submods && submods->getFirstChildWithAttribute(NED_SUBMODULE,"name",name))
                issubmod = true;
        }

        // generate code
        if (isgate)
        {
            out << "mod->gateSize(\"" << name << "\")";
        }
        else if (issubmod)
        {
            out << name << "_size";
        }
        else
        {
            INTERNAL_ERROR2(node, "%s is neither a module gate not a submodule in sizeof(%s)", name, name);
        }
        return;
    }
    else if (!strcmp(funcname,"input"))
    {
        NEDElement *op1 = node->getFirstChild();
        NEDElement *op2 = op1 ? op1->getNextSibling() : NULL;

        if (mode==MODE_INLINE_EXPRESSION)
        {
            out << "(tmpval=";
            if (op1) {
                generateItem(op1,indent,mode);
            } else {
                out << "0L";
            }
            if (op2) {
                out << ", tmpval.setPrompt(";
                generateItem(op2,indent,mode);
                out << ")";
            }
            out << ", tmpval.setInput(true), tmpval)";
        }
        else
        {
            // FIXME: NOT GOOD THIS WAY! if inside expression, input cpar must become member of the expr class...
            out << "(p=new cPar(), (*p)=";
            if (op1) {
                generateItem(op1,indent,mode);
            }
            if (op2) {
                out << ", p->setPrompt(";
                generateItem(op2,indent,mode);
                out << ")";
            }
            out << ", p->setInput(true), (*p))";
        }
        return;
    }
    else if (!strcmp(funcname,"xmldoc"))
    {
        if (mode!=MODE_INLINE_EXPRESSION) {INTERNAL_ERROR0(node, "doFunction(): xmldoc() must be generated inline");return;}

        NEDElement *op1 = node->getFirstChild();
        NEDElement *op2 = op1 ? op1->getNextSibling() : NULL;

        out << "_getXMLDocument(";
        if (op1) {
            generateItem(op1,indent,mode);
        }
        if (op2) {
            out << ", ";
            generateItem(op2,indent,mode);
        }
        out << ")";
        return;
    }

    // normal function: emit function call code
    if (mode==MODE_EXPRESSION_CLASS)
    {
        out << getNameForArg(node) << node->getId();
    }
    else // MODE_INLINE_EXPRESSION
    {
        const char *methods[] = {"mathFuncNoArg()", "mathFunc1Arg()", "mathFunc2Args()",
                                 "mathFunc3Args()", "mathFunc4Args()"};
        out << "_getFunction(\"" << funcname << "\"," << argcount << ")->" << methods[argcount];
    }

    // arglist is the same for both modes
    out << "(";
    for (NEDElement *child=node->getFirstChild(); child; child = child->getNextSibling())
    {
        if (child != node->getFirstChild())
            out << ", ";
        generateItem(child,indent,mode);
    }
    out << ")";
}

void CppExpressionGenerator::doParamref(ParamRefNode *node, const char *indent, int mode)
{
    if (mode==MODE_EXPRESSION_CLASS)
    {
        if (node->getIsAncestor())
        {
            out << "mod->ancestorPar(\"" << node->getParamName() << "\")";
        }
        else
        {
            // arg holds pointer of current module
            out << "mod";
            if (strnotnull(node->getModule()))
            {
                out << "->submodule(\"" << node->getModule() << "\")";
                ExpressionNode *modindex = (ExpressionNode *) node->getFirstChildWithAttribute(NED_EXPRESSION,"target","vector-size");
                if (modindex)
                {
                    out << "[_checkModuleIndex((int)(";
                    generateExpressionUsage(modindex,indent);
                    // FIXME modname_size will be undefined here........
                    out << ")," << node->getModule() << "_size,\"" << node->getModule() << "\")]";
                }
            }
            out << "->par(\"" << node->getParamName() << "\")";
            // FIXME this will always be by reference?
        }
    }
    else // MODE_INLINE_EXPRESSION
    {
        if (node->getIsAncestor())
        {
            out << "mod->ancestorPar(\"" << node->getParamName() << "\")";
        }
        else
        {
            // FIXME handle node->getIsRef(), if ever needed with inline expressions(?)
            if (strnotnull(node->getModule()))
            {
                out << node->getModule() << "_p";
                ExpressionNode *modindex = (ExpressionNode *) node->getFirstChildWithAttribute(NED_EXPRESSION,"target","vector-size");
                if (modindex)
                {
                    out << "[_checkModuleIndex((int)(";
                    generateExpressionUsage(modindex,indent);
                    out << ")," << node->getModule() << "_size,\"" << node->getModule() << "\")]";
                }
                out << "->";
            } else {
                out << "mod->";
            }
            out << "par(\"" << node->getParamName() << "\")";
        }
    }
}

void CppExpressionGenerator::doIdent(IdentNode *node, const char *indent, int mode)
{
    if (mode==MODE_EXPRESSION_CLASS)
    {
        out << getNameForArg(node) << "_var";
    }
    else // MODE_INLINE_EXPRESSION
    {
        out << node->getName() << "_var";
    }
}

void CppExpressionGenerator::doConst(ConstNode *node, const char *indent, int mode)
{
    bool isstring = (node->getType()==NED_CONST_STRING);

    if (isstring) out << "\"";
    out << node->getValue();
    if (isstring) out << "\"";
}

void CppExpressionGenerator::doExpression(ExpressionNode *node, const char *indent, int mode)
{
    generateItem(node->getFirstChild(), indent, mode);
}

