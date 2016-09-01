////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  Embedded Machine Learning Library (EMLL)
//  File:     ISerializable_test.cpp (utilities)
//  Authors:  Chuck Jacobs
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ISerializable_test.h"

// utilities
#include "UniqueId.h"
#include "ISerializable.h"
#include "Serializer.h"
#include "JsonSerializer.h"
#include "XMLSerializer.h"

// model
#include "Model.h"
#include "InputNode.h"
#include "OutputNode.h"

// nodes
#include "BinaryOperationNode.h"
#include "ConstantNode.h"

// testing
#include "testing.h"

// stl
#include <iostream>
#include <vector>
#include <sstream>

struct TestStruct : public utilities::ISerializable
{
    int a=0;
    float b=0.0f;
    double c=0.0;
    TestStruct() = default;
    TestStruct(int a, float b, double c) : a(a), b(b), c(c) {}
    static std::string GetTypeName() { return "TestStruct"; }
    virtual std::string GetRuntimeTypeName() const override { return GetTypeName(); }

    virtual void Serialize(utilities::Serializer& serializer) const override
    {
        serializer.Serialize("a", a);
        serializer.Serialize("b", b);
        serializer.Serialize("c", c);
    }

    virtual void Deserialize(utilities::Deserializer& serializer) override
    {
        // what about _type?
        serializer.Deserialize("a", a);
        serializer.Deserialize("b", b);
        serializer.Deserialize("c", c);
    }
};

template <typename SerializerType>
void TestSerializer()
{
    bool boolVal = true;
    int intVal = 1;
    float floatVal = 2.5;
    double doubleVal = 3.14;
    TestStruct testStruct{ 1, 2.2f, 3.3 };

    utilities::UniqueId id;

    model::Model g;
    auto in = g.AddNode<model::InputNode<double>>(3);
    auto constNode = g.AddNode<nodes::ConstantNode<double>>(std::vector<double>{ 1.0, 2.0, 3.0 });
    auto binaryOpNode = g.AddNode<nodes::BinaryOperationNode<double>>(in->output, constNode->output, nodes::BinaryOperationNode<double>::OperationType::add);
    auto out = g.AddNode<model::OutputNode<double>>(in->output);


    std::stringstream strstream;
    SerializerType serializer(strstream);
    serializer.Serialize(boolVal);

    serializer.Serialize(intVal);

    serializer.Serialize(floatVal);

    serializer.Serialize(doubleVal);

    serializer.Serialize(testStruct);

    serializer.Serialize(id);

    serializer.Serialize(*in);

    serializer.Serialize(*out);

    serializer.Serialize(*constNode);

    serializer.Serialize(*binaryOpNode);

    serializer.Serialize(g);

    // simple stuff
    serializer.Serialize(5);

    serializer.Serialize(3.1415);

    std::vector<int> intArray{ 1, 2, 3 };
    serializer.Serialize("intArray", intArray);

    std::vector<bool> boolArray{ true, false, true };
    serializer.Serialize("boolArray", boolArray);

    std::vector<TestStruct> structArray;
    structArray.emplace_back(1, 2.0f, 3.0);
    structArray.emplace_back(4, 5.0f, 6.0);
    structArray.emplace_back(7, 8.0f, 9.0);
    serializer.Serialize("structArray", structArray);
}

template <typename SerializerType, typename DeserializerType>
void TestDeserializer()
{
    utilities::SerializationContext context;

    {
        std::stringstream strstream;
        {
            SerializerType serializer(strstream);
            serializer.Serialize("true", true);
        }

        DeserializerType deserializer(strstream, context);
        bool val = false;
        deserializer.Deserialize("true", val);
        testing::ProcessTest("Deserialize bool check", val == true);
    }

    {
        std::stringstream strstream;
        {
            SerializerType serializer(strstream);
            serializer.Serialize("pi", 3.14159);
        }

        DeserializerType deserializer(strstream, context);
        double val = 0;
        deserializer.Deserialize("pi", val);
        testing::ProcessTest("Deserialize float check", val == 3.14159);
    }

    {
        std::stringstream strstream;
        {
            SerializerType serializer(strstream);
            serializer.Serialize("pie", std::string{ "cherry pie" });
        }

        DeserializerType deserializer(strstream, context);
        std::string val;
        deserializer.Deserialize("pie", val);
        testing::ProcessTest("Deserialize string check", val == "cherry pie");
    }

    {
        std::stringstream strstream;
        {
            SerializerType serializer(strstream);
            std::vector<int> arr{ 1,2,3 };
            serializer.Serialize("arr", arr);
        }

        DeserializerType deserializer(strstream, context);
        std::vector<int> val;
        deserializer.Deserialize("arr", val);
        testing::ProcessTest("Deserialize vector<int> check", val[0] == 1 && val[1] == 2 && val[2] == 3);
    }

    {
        std::stringstream strstream;
        {
            SerializerType serializer(strstream);
            TestStruct testStruct{ 1, 2.2f, 3.3 };
            serializer.Serialize("s", testStruct);
        }

        DeserializerType deserializer(strstream, context);
        TestStruct val;
        deserializer.Deserialize("s", val);
        testing::ProcessTest("Deserialize ISerializable check",  val.a == 1 && val.b == 2.2f && val.c == 3.3);        
    }

    {
        model::Model g;
        utilities::SerializationContext context;
        model::ModelSerializationContext modelContext(context, &g);
        modelContext.GetTypeFactory().AddType<model::Node, model::InputNode<double>>();
        modelContext.GetTypeFactory().AddType<model::Node, model::OutputNode<double>>();
        modelContext.GetTypeFactory().AddType<model::Node, nodes::ConstantNode<double>>();
        modelContext.GetTypeFactory().AddType<model::Node, nodes::BinaryOperationNode<double>>();
        modelContext.GetTypeFactory().AddType<nodes::ConstantNode<double>, nodes::ConstantNode<double>>();

        std::stringstream strstream;
        auto constVector = std::vector<double>{ 1.0, 2.0, 3.0 };

        {
            SerializerType serializer(strstream);
            auto in = g.AddNode<model::InputNode<double>>(3);
            auto constNode = g.AddNode<nodes::ConstantNode<double>>(constVector);
            auto binaryOpNode = g.AddNode<nodes::BinaryOperationNode<double>>(in->output, constNode->output, nodes::BinaryOperationNode<double>::OperationType::add);
            auto out = g.AddNode<model::OutputNode<double>>(in->output);

            serializer.Serialize("node1", *constNode);
            serializer.Serialize("node2", *in);
            serializer.Serialize("node3", constNode);
            serializer.Serialize("node4", constNode);
            serializer.Serialize("node5", binaryOpNode);
        }

        DeserializerType deserializer(strstream, context);
        deserializer.PushContext(modelContext);
        nodes::ConstantNode<double> newConstNode;
        model::InputNode<double> newIn;
        nodes::BinaryOperationNode<double> newBinaryOpNode;
        std::unique_ptr<nodes::ConstantNode<double>> newConstNodePtr = nullptr;
        std::unique_ptr<model::Node> newNodePtr = nullptr;
        std::unique_ptr<nodes::BinaryOperationNode<double>> newBinaryOpNodePtr = nullptr;
        deserializer.Deserialize("node1", newConstNode);
        deserializer.Deserialize("node2", newIn);
        deserializer.Deserialize("node3", newConstNodePtr);
        deserializer.Deserialize("node4", newNodePtr);
        deserializer.Deserialize("node5", newBinaryOpNode);
        deserializer.PopContext();
        testing::ProcessTest("Deserialize nodes check",  testing::IsEqual(constVector, newConstNode.GetValues()));
        testing::ProcessTest("Deserialize nodes check",  testing::IsEqual(constVector, newConstNodePtr->GetValues()));
    }

    {
        // arrays of stuff
        std::stringstream strstream;
        auto doubleVector = std::vector<double>{ 1.0, 2.0, 3.0 };
        std::vector<TestStruct> structVector;
        structVector.push_back(TestStruct{ 1, 2.2f, 3.3 });
        structVector.push_back(TestStruct{ 4, 5.5f, 6.6 });

        {
            SerializerType serializer(strstream);
            serializer.Serialize("vec1", doubleVector);
            serializer.Serialize("vec2", structVector);
        }

        DeserializerType deserializer(strstream, context);
        std::vector<double> newDoubleVector;
        std::vector<TestStruct> newStructVector;
        deserializer.Deserialize("vec1", newDoubleVector);
        deserializer.Deserialize("vec2", newStructVector);

        testing::ProcessTest("Deserialize array check",  testing::IsEqual(doubleVector, newDoubleVector));
        testing::ProcessTest("Deserialize array check",  testing::IsEqual(structVector[0].a, newStructVector[0].a));
        testing::ProcessTest("Deserialize array check",  testing::IsEqual(structVector[0].b, newStructVector[0].b));
        testing::ProcessTest("Deserialize array check",  testing::IsEqual(structVector[0].c, newStructVector[0].c));
        testing::ProcessTest("Deserialize array check",  testing::IsEqual(structVector[1].a, newStructVector[1].a));
        testing::ProcessTest("Deserialize array check",  testing::IsEqual(structVector[1].b, newStructVector[1].b));
        testing::ProcessTest("Deserialize array check",  testing::IsEqual(structVector[1].c, newStructVector[1].c));
    }

    {
        model::Model g;
        utilities::SerializationContext context;
        model::ModelSerializationContext modelContext(context, &g);
        modelContext.GetTypeFactory().AddType<model::Node, model::InputNode<double>>();
        modelContext.GetTypeFactory().AddType<model::Node, model::OutputNode<double>>();
        modelContext.GetTypeFactory().AddType<model::Node, nodes::ConstantNode<double>>();
        modelContext.GetTypeFactory().AddType<model::Node, nodes::BinaryOperationNode<double>>();
        modelContext.GetTypeFactory().AddType<nodes::ConstantNode<double>, nodes::ConstantNode<double>>();
        auto in = g.AddNode<model::InputNode<double>>(3);
        auto doubleVector = std::vector<double>{ 1.0, 2.0, 3.0 };
        auto constNode = g.AddNode<nodes::ConstantNode<double>>(doubleVector);
        auto binaryOpNode = g.AddNode<nodes::BinaryOperationNode<double>>(in->output, constNode->output, nodes::BinaryOperationNode<double>::OperationType::add);
        auto out = g.AddNode<model::OutputNode<double>>(in->output);

        std::stringstream strstream;
        {
            SerializerType serializer(strstream);

            serializer.Serialize(g);
            // std::cout << "Graph output:" << std::endl;
            // std::cout << strstream.str() << std::endl;
        }

        DeserializerType deserializer(strstream, context);
        deserializer.PushContext(modelContext);
        model::Model newGraph;
        deserializer.Deserialize(newGraph);

        std::stringstream strstream2;
        SerializerType serializer2(strstream2);

        serializer2.Serialize(newGraph);
        // std::cout << "New graph output:" << std::endl;
        // std::cout << strstream2.str() << std::endl;
    }

    {
        auto stringVal = std::string{ "Hi there! Here's a tab character: \t, as well as some 'quoted' text." };
        std::stringstream strstream;
        {
            SerializerType serializer(strstream);
            serializer.Serialize("str", stringVal);
        }
        
        DeserializerType deserializer(strstream, context);
        std::string val;
        deserializer.Deserialize("str", val);
        testing::ProcessTest("Deserialize string check", val == stringVal);    
    }
}

void TestJsonSerializer()
{
    TestSerializer<utilities::JsonSerializer>();
}

void TestJsonDeserializer()
{
    TestDeserializer<utilities::JsonSerializer, utilities::JsonDeserializer>();
}

void TestXmlSerializer()
{
    TestSerializer<utilities::SimpleXmlSerializer>();
}

void TestXmlDeserializer()
{
    TestDeserializer<utilities::SimpleXmlSerializer, utilities::SimpleXmlDeserializer>();
}
