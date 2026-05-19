//-----------------------------------------------------------------------------
// Copyright (c) 2026 korkscript contributors.
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "embed/internalApi.h"
#include "sim/dynamicTypes.h"

#include <catch2/catch.hpp>

#include <utility>

extern KorkApi::Vm* sVM;

using Catch::Approx;

namespace
{
   class TestDynamicType : public ConsoleBaseType
   {
      typedef ConsoleBaseType Parent;

   public:
      explicit TestDynamicType(const char* typeName, const char* className, U32 size, U32 valueSize)
         : ConsoleBaseType(size, valueSize, &mTypeIdStorage, typeName),
           mClassName(className)
      {
      }

      bool getData(KorkApi::Vm* vmPtr, KorkApi::TypeStorageInterface* inputStorage, KorkApi::TypeStorageInterface* outputStorage, void* fieldUserPtr, BitSet32 flag, U32 requestedType) override
      {
         (void)vmPtr;
         (void)inputStorage;
         (void)outputStorage;
         (void)fieldUserPtr;
         (void)flag;
         (void)requestedType;
         return true;
      }

      const char* getTypeClassName() override
      {
         return mClassName;
      }

      void exportToVm(KorkApi::Vm* vm) override
      {
         exportTypeToVm(this, vm);
      }

      const char* prepData(KorkApi::Vm* vmPtr, const char* data, char* buffer, U32 bufferLen) override
      {
         (void)vmPtr;
         (void)buffer;
         (void)bufferLen;
         return data;
      }

      S32 mTypeIdStorage = 0;
      const char* mClassName;
   };

   TestDynamicType* getAlphaType()
   {
      static TestDynamicType* sAlpha = new TestDynamicType("DynamicAlpha", "DynamicAlphaClass", sizeof(S32), sizeof(S32));
      return sAlpha;
   }

   TestDynamicType* getBetaType()
   {
      static TestDynamicType* sBeta = new TestDynamicType("DynamicBeta", "DynamicBetaClass", sizeof(F32), sizeof(F32));
      return sBeta;
   }

   void ensureDynamicTypesInitialized()
   {
      getAlphaType();
      getBetaType();
      ConsoleBaseType::initialize();
   }
}

TEST_CASE("ConsoleBaseType list construction and lookup", "[dynamicTypes][ConsoleBaseType]") {
   ensureDynamicTypesInitialized();

   TestDynamicType* alpha = getAlphaType();
   TestDynamicType* beta = getBetaType();

   REQUIRE(ConsoleBaseType::getListHead() == beta);
   REQUIRE(beta->getListNext() == alpha);
   REQUIRE(alpha->getTypeID() > 0);
   REQUIRE(beta->getTypeID() > alpha->getTypeID());
   REQUIRE(ConsoleBaseType::getType(alpha->getTypeID()) == alpha);
   REQUIRE(ConsoleBaseType::getType(beta->getTypeID()) == beta);

   REQUIRE(std::string(alpha->getTypeName()) == "DynamicAlpha");
   REQUIRE(alpha->getTypeClassName() == std::string("DynamicAlphaClass"));
   REQUIRE(alpha->getFieldSize() == sizeof(S32));
   REQUIRE(alpha->getValueSize() == sizeof(S32));
   alpha->setInspectorFieldType(nullptr);
   REQUIRE(alpha->getInspectorFieldType() == nullptr);

   alpha->setInspectorFieldType("DynamicInspector");
   REQUIRE(std::string(alpha->getInspectorFieldType()) == "DynamicInspector");
   REQUIRE(alpha->getTypePrefix() == StringTable->EmptyString);
   REQUIRE_FALSE(alpha->isDatablock());
}

TEST_CASE("ConsoleBaseType VM registration exports type metadata", "[dynamicTypes][ConsoleBaseType]") {
   ensureDynamicTypesInitialized();

   TestDynamicType* alpha = getAlphaType();
   TestDynamicType* beta = getBetaType();
   REQUIRE(sVM != nullptr);

   alpha->setInspectorFieldType("DynamicInspector");
   const size_t beforeCount = sVM->mInternal->mTypes.size();
   alpha->exportToVm(sVM);
   REQUIRE(sVM->mInternal->mTypes.size() == beforeCount + 1);

   const KorkApi::TypeInfo& info = sVM->mInternal->mTypes.back();
   REQUIRE(info.name == StringTable->insert("DynamicAlpha"));
   REQUIRE(info.inspectorFieldType == StringTable->insert("DynamicInspector"));
   REQUIRE(info.userPtr == alpha);
   REQUIRE(info.fieldSize == alpha->getFieldSize());
   REQUIRE(info.valueSize == alpha->getValueSize());
   REQUIRE(info.iFuncs.CastValueFn != nullptr);
   REQUIRE(info.iFuncs.GetTypeClassNameFn != nullptr);
   REQUIRE(info.iFuncs.PrepDataFn != nullptr);
   REQUIRE(info.iFuncs.PerformOpFn != nullptr);
   REQUIRE(std::string(info.iFuncs.GetTypeClassNameFn(info.userPtr)) == "DynamicAlphaClass");

   const size_t beforeDirectRegisterCount = sVM->mInternal->mTypes.size();
   beta->registerTypeWithVm(sVM);
   REQUIRE(sVM->mInternal->mTypes.size() == beforeDirectRegisterCount + 1);

   const KorkApi::TypeInfo& betaInfo = sVM->mInternal->mTypes.back();
   REQUIRE(betaInfo.name == StringTable->insert("DynamicBeta"));
   REQUIRE(betaInfo.userPtr == beta);
   REQUIRE(betaInfo.fieldSize == beta->getFieldSize());
   REQUIRE(betaInfo.valueSize == beta->getValueSize());
}

TEST_CASE("ConsoleBaseType operation helpers", "[dynamicTypes][ConsoleBaseType]") {
   ensureDynamicTypesInitialized();

   TestDynamicType* alpha = getAlphaType();
   REQUIRE(sVM != nullptr);

   const KorkApi::ConsoleValue lhsNum = KorkApi::ConsoleValue::makeNumber(2.0);
   const KorkApi::ConsoleValue rhsNum = KorkApi::ConsoleValue::makeNumber(3.0);
   const KorkApi::ConsoleValue zeroNum = KorkApi::ConsoleValue::makeNumber(0.0);
   const KorkApi::ConsoleValue oneNum = KorkApi::ConsoleValue::makeNumber(1.0);

   REQUIRE(alpha->performOp(sVM, Compiler::OP_ADD, lhsNum, rhsNum).getFloat() == Approx(2.0));

   REQUIRE(alpha->performOpNumeric(sVM, Compiler::OP_ADD, lhsNum, rhsNum).getFloat() == Approx(5.0));
   REQUIRE(alpha->performOpNumeric(sVM, Compiler::OP_SUB, lhsNum, rhsNum).getFloat() == Approx(-1.0));
   REQUIRE(alpha->performOpNumeric(sVM, Compiler::OP_MUL, lhsNum, rhsNum).getFloat() == Approx(6.0));
   REQUIRE(alpha->performOpNumeric(sVM, Compiler::OP_DIV, lhsNum, zeroNum).getFloat() == Approx(0.0));
   REQUIRE(alpha->performOpNumeric(sVM, Compiler::OP_MOD, lhsNum, zeroNum).getFloat() == Approx(0.0));
   REQUIRE(alpha->performOpNumeric(sVM, Compiler::OP_CMPEQ, lhsNum, lhsNum).getFloat() == Approx(1.0));
   REQUIRE(alpha->performOpNumeric(sVM, Compiler::OP_CMPNE, lhsNum, rhsNum).getFloat() == Approx(1.0));
   REQUIRE(alpha->performOpNumeric(sVM, Compiler::OP_BITAND, lhsNum, rhsNum).getFloat() == Approx(2.0));
   REQUIRE(alpha->performOpNumeric(sVM, Compiler::OP_BITOR, lhsNum, rhsNum).getFloat() == Approx(3.0));
   REQUIRE(alpha->performOpNumeric(sVM, Compiler::OP_AND, oneNum, zeroNum).getFloat() == Approx(0.0));
   REQUIRE(alpha->performOpNumeric(sVM, Compiler::OP_OR, oneNum, zeroNum).getFloat() == Approx(1.0));
   REQUIRE(alpha->performOpNumeric(sVM, Compiler::OP_NEG, lhsNum, rhsNum).getFloat() == Approx(-2.0));
   REQUIRE(alpha->performOpNumeric(sVM, Compiler::OP_NOT, lhsNum, rhsNum).getFloat() == Approx(0.0));
   REQUIRE(alpha->performOpNumeric(sVM, Compiler::OP_SHL, lhsNum, oneNum).getFloat() == Approx(4.0));
   REQUIRE(alpha->performOpNumeric(sVM, Compiler::OP_SHR, KorkApi::ConsoleValue::makeNumber(8.0), oneNum).getFloat() == Approx(4.0));
   REQUIRE(alpha->performOpNumeric(sVM, Compiler::OP_INVALID, lhsNum, rhsNum).getFloat() == Approx(2.0));

   const KorkApi::ConsoleValue lhsInt = KorkApi::ConsoleValue::makeUnsigned(2);
   const KorkApi::ConsoleValue rhsInt = KorkApi::ConsoleValue::makeUnsigned(3);
   const KorkApi::ConsoleValue zeroInt = KorkApi::ConsoleValue::makeUnsigned(0);
   const KorkApi::ConsoleValue oneInt = KorkApi::ConsoleValue::makeUnsigned(1);

   REQUIRE(alpha->performOpUnsigned(sVM, Compiler::OP_ADD, lhsInt, rhsInt).getInt() == 5);
   REQUIRE(alpha->performOpUnsigned(sVM, Compiler::OP_SUB, lhsInt, rhsInt).getInt() == static_cast<U64>(-1));
   REQUIRE(alpha->performOpUnsigned(sVM, Compiler::OP_MUL, lhsInt, rhsInt).getInt() == 6);
   REQUIRE(alpha->performOpUnsigned(sVM, Compiler::OP_DIV, lhsInt, zeroInt).getInt() == 0);
   REQUIRE(alpha->performOpUnsigned(sVM, Compiler::OP_MOD, lhsInt, zeroInt).getInt() == 0);
   REQUIRE(alpha->performOpUnsigned(sVM, Compiler::OP_CMPEQ, lhsInt, lhsInt).getInt() == 1);
   REQUIRE(alpha->performOpUnsigned(sVM, Compiler::OP_CMPNE, lhsInt, rhsInt).getInt() == 1);
   REQUIRE(alpha->performOpUnsigned(sVM, Compiler::OP_BITAND, lhsInt, rhsInt).getInt() == 2);
   REQUIRE(alpha->performOpUnsigned(sVM, Compiler::OP_BITOR, lhsInt, rhsInt).getInt() == 3);
   REQUIRE(alpha->performOpUnsigned(sVM, Compiler::OP_AND, oneInt, zeroInt).getInt() == 0);
   REQUIRE(alpha->performOpUnsigned(sVM, Compiler::OP_OR, oneInt, zeroInt).getInt() == 1);
   REQUIRE(alpha->performOpUnsigned(sVM, Compiler::OP_NEG, lhsInt, rhsInt).getInt() == static_cast<U64>(-2));
   REQUIRE(alpha->performOpUnsigned(sVM, Compiler::OP_NOT, lhsInt, rhsInt).getInt() == 0);
   REQUIRE(alpha->performOpUnsigned(sVM, Compiler::OP_SHL, lhsInt, oneInt).getInt() == 4);
   REQUIRE(alpha->performOpUnsigned(sVM, Compiler::OP_SHR, KorkApi::ConsoleValue::makeUnsigned(8), oneInt).getInt() == 4);
   REQUIRE(alpha->performOpUnsigned(sVM, Compiler::OP_INVALID, lhsInt, rhsInt).getInt() == 2);

   const char* prepInput = "dynamic-types-prep";
   char prepBuffer[64] = {};
   REQUIRE(alpha->prepData(sVM, prepInput, prepBuffer, sizeof(prepBuffer)) == prepInput);
   REQUIRE(std::string(alpha->getTypeClassName()) == "DynamicAlphaClass");
}
