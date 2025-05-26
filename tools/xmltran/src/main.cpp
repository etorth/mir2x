// from: https://gist.github.com/Lee-swifter/d9cd651b093f0d32b65a2bce47b0ad91
#include <iostream>
#include "tinyxml2.h"

static void xmlTran(tinyxml2::XMLNode* node)
{
    if(node == nullptr){
        return;
    }

    if(node->ToDeclaration()){
        auto declaration = dynamic_cast<tinyxml2::XMLDeclaration *>(node);
        std::cout << "XMLDeclaration value: " << declaration->Value() << std::endl;
    }

    if(node->ToElement()){
        auto element = dynamic_cast<tinyxml2::XMLElement *>(node);
        std::cout << "XMLElement name: " << element->Name() << ", value: " << element->Value() << std::endl;
        const tinyxml2::XMLAttribute* attribute = element->FirstAttribute();
        while (attribute != nullptr) {
            std::cout << "\tXMLAttribute " << attribute->Name() << "=" << attribute->Value() << std::endl;
            attribute = attribute->Next();
        }
    }

    if(node->ToText()){
        auto text = dynamic_cast<tinyxml2::XMLText *>(node);
        std::cout << "XMLText: " << text->Value() << std::endl;
    }

    if(node->ToComment()) {
        auto comment = dynamic_cast<tinyxml2::XMLComment *>(node);
        std::cout << "XMLComment: " << comment->Value() << std::endl;
    }

    if(node->ToUnknown()) {
        auto unknown = dynamic_cast<tinyxml2::XMLUnknown *>(node);
        std::cout << "XMLUnknown: " << unknown->Value() << std::endl;
    }

    if(node->ToDocument()) {
        auto document = dynamic_cast<tinyxml2::XMLDocument *>(node);
        std::cout << "XMLDocument: " << document->ErrorName() << std::endl;
    }

    if(node->NoChildren()) {
        return;
    }

    tinyxml2::XMLNode* child = node->FirstChild();
    while(child != nullptr) {
        xmlTran(child);
        child = child->NextSibling();
    }
}

int main(int argc, char *argv[])
{
    if(argc == 2){
        tinyxml2::XMLDocument xmlDoc;
        tinyxml2::XMLError error = xmlDoc.LoadFile(argv[1]);
        if(error != tinyxml2::XML_SUCCESS) {
            std::cout << "Failed to read XML: " << xmlDoc.ErrorStr() << std::endl;
            return EXIT_FAILURE;
        }

        xmlTran(&xmlDoc);
        return EXIT_SUCCESS;
    }
    else{
        std::cout << "Usage: xmltran <input.xml>" << std::endl;
        return EXIT_FAILURE;
    }
}
