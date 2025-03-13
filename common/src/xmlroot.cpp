#include <cstdlib>
#include <cstring>
#include <string>
#include <stdexcept>
#include "strf.hpp"
#include "xmlroot.hpp"
#include "fflerror.hpp"

#define _xmlroot_to_type(nodePath, func) \
do{ \
    fflassert(str_haschar(nodePath)); \
    if(const auto nodePtr = getXMLNode(nodePath)){ \
        const auto textPtr = nodePtr->GetText(); \
        fflassert(str_haschar(textPtr)); \
        return func(textPtr); \
    } \
    return {}; \
}while(0)

std::optional<int> XMLRoot::to_d(const char *nodePath) const
{
    _xmlroot_to_type(nodePath, std::stoi);
}

std::optional<float> XMLRoot::to_f(const char *nodePath) const
{
    _xmlroot_to_type(nodePath, std::stof);
}

std::optional<bool> XMLRoot::to_bool(const char *nodePath) const
{
    _xmlroot_to_type(nodePath, to_bool);
}

const tinyxml2::XMLElement *XMLRoot::getXMLNode(const char *nodePath) const
{
    // well-defined path should be:
    // "root/basic/fullscreen/a/b/c/d/e"
    //
    // but also support:
    // "basic/fullscreen/a/b/c/d/e"
    // "////root/basic/fullscreen/a/b/c/d/e"
    // "   /root/basic/fullscreen/a/b/c/d/e"

    fflassert(str_haschar(nodePath));
    const char *startCPtr = nodePath;

    while(startCPtr[0] == '/'){
        startCPtr++;
    }

    if(startCPtr[0] == '\0'){
        return nullptr;
    }

    // skip root in front if possible
    // we don't need to check std::strlen(startCPtr) here
    if(true
            && (startCPtr[0] == 'r' || startCPtr[0] == 'R')
            && (startCPtr[1] == 'o' || startCPtr[1] == 'O')
            && (startCPtr[2] == 'o' || startCPtr[2] == 'O')
            && (startCPtr[3] == 't' || startCPtr[3] == 'T')){

        startCPtr += 4;
        while(startCPtr[0] == '/'){
            startCPtr++;
        }

        if(startCPtr[0] == '\0'){
            return nullptr;
        }
    }

    const char *endCPtr = nullptr;
    const auto *nodePtr = m_xmlDoc.RootElement();

    while(true){
        // startCPtr must be valid here
        // 1. not nullptr
        // 2. startCPtr[0] != '\0'
        endCPtr = std::strchr(startCPtr, '/');
        if(endCPtr == nullptr){
            return nodePtr->FirstChildElement(startCPtr);
        }
        else{
            nodePtr = nodePtr->FirstChildElement(std::string(startCPtr, endCPtr - startCPtr).c_str());

            // detected wrong path when parsing
            if(nodePtr == nullptr){
                return nullptr;
            }

            // get rid of all '//////'
            // this also takes care of "root/a/b/" which ends with '/'
            //
            startCPtr = endCPtr;
            while(*startCPtr == '/'){
                startCPtr++;
            }

            if(startCPtr[0] == '\0'){
                return nodePtr;
            }
        }
    }
    throw fflreach();
}

bool XMLRoot::load(const char *fileName)
{
    fflassert(str_haschar(fileName));
    return m_xmlDoc.LoadFile(fileName) == tinyxml2::XML_SUCCESS;
}
