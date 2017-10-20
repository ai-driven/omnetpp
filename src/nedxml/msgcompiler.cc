//==========================================================================
//  MSGCOMPILER.CC - part of
//
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 2002-2017 Andras Varga
  Copyright (C) 2006-2017 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

#include "common/stringutil.h"
#include "common/stlutil.h"
#include "common/fileutil.h"
#include "msgcompiler.h"
#include "nedparser.h"
#include "nedexception.h"
#include "nedutil.h"

#include "omnetpp/simkerneldefs.h"

using namespace omnetpp::common;

namespace omnetpp {
namespace nedxml {

static bool isQualified(const std::string& qname)
{
    return qname.find("::") != qname.npos;
}

static std::string canonicalizeQName(const std::string& namespac, const std::string& name)
{
    std::string qname;
    if (name.find("::") != name.npos)
        qname = name.substr(0, 2) == "::" ? name.substr(2) : name;  // remove leading "::", because names in @classes don't have it either
    else if (!namespac.empty() && !name.empty())
        qname = namespac + "::" + name;
    else
        qname = name;
    return qname;
}

inline std::string ptr2str(const char *ptr)
{
    return ptr ? ptr : "";
}

// Note: based on omnetpp 5.x checkandcast.h (merged check_and_cast and check_and_cast_failure)
template<class P, class T>
P check_and_cast(T *p)
{
    assert(p);
    P ret = dynamic_cast<P>(p);
    assert(ret);
    return ret;
}

MsgCompiler::MsgCompiler(NEDErrorStore *e, const MsgCompilerOptions& options) : analyzer(typeTable, e)
{
    opts = options;
    errors = e;
}

MsgCompiler::~MsgCompiler()
{
}

void MsgCompiler::generate(MsgFileElement *fileElement, const char *hFile, const char *ccFile)
{
    codegen.openFiles(hFile, ccFile);
    process(fileElement, true);
    codegen.closeFiles();

    if (errors->containsError())
        codegen.deleteFiles();
}

void MsgCompiler::processImport(NEDElement *child, const std::string& currentDir)
{
    if (!currentNamespace.empty()) {
        errors->addError(child, "imports are not allowed within a namespace\n");
        return;
    }

    std::string importName = child->getAttribute("import-spec");
    std::string fileName = resolveImport(importName, currentDir);
    if (fileName == "") {
        errors->addError(child, "cannot resolve import '%s'\n", importName.c_str());
        return;
    }

    NEDParser parser(errors);
    NEDElement *tree = parser.parseMSGFile(fileName.c_str());
    if (errors->containsError()) {
        delete tree;
        return;
    }

    typeTable.importedNedFiles.push_back(tree); // keep AST until we're done because ClassInfo/FieldInfo refer to it...

    // extract declarations
    MsgFileElement *fileElement = check_and_cast<MsgFileElement*>(tree);
    process(fileElement, false);
    currentNamespace = "";
}

std::string MsgCompiler::resolveImport(const std::string& importName, const std::string& currentDir)
{
    std::string msgFile = opp_replacesubstring(importName, ".", "/", true) + ".msg";
    std::string candidate = concatDirAndFile(currentDir.c_str(), msgFile.c_str());
    if (fileExists(candidate.c_str()))
        return candidate;
    for (auto dir : opts.importPath) {
        std::string candidate = concatDirAndFile(dir.c_str(), msgFile.c_str());
        if (fileExists(candidate.c_str()))
            return candidate;
    }
    return "";
}

void MsgCompiler::process(MsgFileElement *fileElement, bool generateCode)
{
    std::string currentDir = directoryOf(fileElement->getFilename());

    if (generateCode) {
        NamespaceElement *firstNS = fileElement->getFirstNamespaceChild();
        std::string firstNSName = firstNS ? ptr2str(firstNS->getAttribute("name")) : "";
        codegen.generateProlog(fileElement->getFilename(), firstNSName, opts.exportDef);
    }

    /*
       <!ELEMENT msg-file (comment*, (namespace|property-decl|property|cplusplus|import|
                           struct-decl|class-decl|message-decl|packet-decl|enum-decl|
                           struct|class|message|packet|enum)*)>
     */
    for (NEDElement *child = fileElement->getFirstChild(); child; child = child->getNextSibling()) {
        switch (child->getTagCode()) {
            case NED_NAMESPACE:
                // open namespace(s)
                if (generateCode && !currentNamespace.empty())
                    codegen.generateNamespaceEnd(currentNamespace);
                currentNamespace = ptr2str(child->getAttribute("name"));
                validateNamespaceName(currentNamespace, child);
                if (generateCode)
                    codegen.generateNamespaceBegin(currentNamespace);
                break;

            case NED_CPLUSPLUS: {
                // print C++ block
                if (generateCode) {
                    std::string body = ptr2str(child->getAttribute("body"));
                    std::string target = ptr2str(child->getAttribute("target"));
                    if (target == "" || target == "h" || target == "cc")
                        codegen.generateCplusplusBlock(target, body);
                    else
                        errors->addError(child, "unrecognized target '%s' for cplusplus block", target.c_str());
                }
                break;
            }

            case NED_IMPORT: {
                std::string importName = child->getAttribute("import-spec");
                if (!common::contains(importsSeen, importName)) {
                    importsSeen.insert(importName);
                    processImport(child, currentDir);
                    if (generateCode)
                        codegen.generateImport(importName);
                }
                break;
            }

            case NED_STRUCT_DECL:
            case NED_CLASS_DECL:
            case NED_MESSAGE_DECL:
            case NED_PACKET_DECL:
                analyzer.extractClassDecl(child, currentNamespace);
                break;

            case NED_ENUM_DECL: {
                //TODO analyzer.extractEnumDecl()
                // forward declaration -- add to table
                std::string name = ptr2str(child->getAttribute("name"));
                if (contains(analyzer.RESERVED_WORDS, name))
                    errors->addError(child, "Enum name is reserved word: '%s'", name.c_str());
                std::string qname = canonicalizeQName(currentNamespace, name);
                typeTable.declaredEnums.insert(qname);
                break;
            }

            case NED_ENUM: {
                EnumInfo enumInfo = analyzer.extractEnumInfo(check_and_cast<EnumElement *>(child), currentNamespace);
                typeTable.declaredEnums.insert(enumInfo.enumQName);
                typeTable.definedEnums[enumInfo.enumQName] = enumInfo; //TODO wrap
                if (generateCode) {
                    if (isQualified(enumInfo.enumName))
                        errors->addError(enumInfo.nedElement, "type name may not be qualified: '%s'\n", enumInfo.enumName.c_str());
                    codegen.generateEnum(enumInfo);
                }
                break;
            }

            case NED_STRUCT: {
                ClassInfo classInfo = analyzer.extractClassInfo(child, currentNamespace);
                analyzer.analyze(classInfo, currentNamespace, opts);
                typeTable.addDeclaredClass(classInfo.msgname, classInfo.classtype, child); //TODO also included in analyze()
                typeTable.definedClasses[classInfo.msgqname] = classInfo; //TODO wrap
                if (generateCode) {
                    if (classInfo.generate_class) {
                        if (isQualified(classInfo.msgclass))
                            errors->addError(classInfo.nedElement, "type name may only be qualified when generating descriptor for an existing class: '%s'\n", classInfo.msgclass.c_str());
                        codegen.generateStruct(classInfo, opts.exportDef);
                    }
                    if (classInfo.generate_descriptor)
                        codegen.generateDescriptorClass(classInfo);
                }
                break;
            }

            case NED_CLASS:
            case NED_MESSAGE:
            case NED_PACKET: {
                ClassInfo classInfo = analyzer.extractClassInfo(child, currentNamespace);
                analyzer.analyze(classInfo, currentNamespace, opts);
                typeTable.addDeclaredClass(classInfo.msgqname, classInfo.classtype, child); //TODO also included in analyze()
                typeTable.definedClasses[classInfo.msgqname] = classInfo; //TODO wrap
                if (generateCode) {
                    if (classInfo.generate_class) {
                        if (isQualified(classInfo.msgclass))
                            errors->addError(classInfo.nedElement, "type name may only be qualified when generating descriptor for an existing class: '%s'\n", classInfo.msgclass.c_str());
                        codegen.generateClass(classInfo, opts.exportDef);
                    }
                    if (classInfo.generate_descriptor)
                        codegen.generateDescriptorClass(classInfo);
                }
                break;
            }
        }
    }

    if (generateCode) {
        if (!currentNamespace.empty())
            codegen.generateNamespaceEnd(currentNamespace);
        codegen.generateEpilog();
    }
}

void MsgCompiler::validateNamespaceName(const std::string& namespaceName, NEDElement *element)
{
    if (namespaceName.empty())
        errors->addError(element, "namespace name is empty\n");
    for (auto token : opp_split(namespaceName, "::"))
        if (contains(MsgAnalyzer::RESERVED_WORDS, token))
            errors->addError(element, "namespace name '%s' is a reserved word\n", token.c_str());
}

}  // namespace nedxml
}  // namespace omnetpp