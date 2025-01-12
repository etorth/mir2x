// from: https://gist.github.com/Lee-swifter/d9cd651b093f0d32b65a2bce47b0ad91
#include <iostream>
#include "tinyxml2.h"

using namespace std;
using namespace tinyxml2;

void xmlTran(tinyxml2::XMLNode* node) {
    if(node == nullptr)
        return;

    if(node->ToDeclaration()) {
        auto declaration = dynamic_cast<tinyxml2::XMLDeclaration*>(node);
        cout << "XML 声明，value=" << declaration->Value() << endl;
    }
    if(node->ToElement()) {
        auto element = dynamic_cast<tinyxml2::XMLElement*>(node);
        cout << "XML 元素，name=" << element->Name() << ", value=" << element->Value() << endl;
        const tinyxml2::XMLAttribute* attribute = element->FirstAttribute();
        while (attribute != nullptr) {
            cout << "\t属性 " << attribute->Name() << "=" << attribute->Value() << endl;
            attribute = attribute->Next();
        }
    }
    if(node->ToText()) {
        auto text = dynamic_cast<tinyxml2::XMLText*>(node);
        cout << "XML 文本：" << text->Value() << endl;
    }
    if(node->ToComment()) {
        auto comment = dynamic_cast<tinyxml2::XMLComment*>(node);
        cout << "XML 注释：" << comment->Value() << endl;
    }
    if(node->ToUnknown()) {
        auto unknown = dynamic_cast<tinyxml2::XMLUnknown*>(node);
        cout << "XML 未知：" << unknown->Value() << endl;
    }
    if(node->ToDocument()) {
        auto document = dynamic_cast<tinyxml2::XMLDocument*>(node);
        cout << "XML 文档：" << document->ErrorName() << endl;
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
            std::cout << "Failed to read XML: " << xmlDoc.ErrorStr() << endl;
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
