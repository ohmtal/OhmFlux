//-----------------------------------------------------------------------------
// Copyright (c) 2026 korkscript contributors.
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "console/consoleObject.h"
#include "console/consoleTypes.h"
#include "core/stringTable.h"
#include "embed/internalApi.h"
#include "sim/simBase.h"

#include <catch2/catch.hpp>

#include <atomic>
#include <string>
#include <vector>

extern KorkApi::Vm* sVM;

namespace
{
   static std::atomic<U32> gNextConsoleObjectTestId{500000};

   std::string uniqueName(const char* prefix)
   {
      return std::string(prefix) + "_" + std::to_string(gNextConsoleObjectTestId.fetch_add(1));
   }

   struct CapturedFieldInfo
   {
      std::string name;
      std::string typeName;
      std::string groupName;
      std::string docs;
      U16 typeId = 0;
      U32 offset = 0;
      S32 elementCount = 0;
      bool isScriptField = false;
   };

   bool captureClassField(void* userPtr, KorkApi::ClassId ownerClassId, const KorkApi::ClassFieldEnumerationInfo* info)
   {
      (void)ownerClassId;
      auto* fields = static_cast<std::vector<CapturedFieldInfo>*>(userPtr);
      fields->push_back({
         info->fieldName ? info->fieldName : "",
         info->typeName ? info->typeName : "",
         info->groupName ? info->groupName : "",
         info->fieldDocs ? info->fieldDocs : "",
         info->typeId,
         info->offset,
         info->elementCount,
         info->isScriptField
      });
      return true;
   }

   const CapturedFieldInfo* findField(const std::vector<CapturedFieldInfo>& fields, const char* name)
   {
      const std::string needle = name ? name : "";
      for (const CapturedFieldInfo& field : fields)
      {
         if (field.name == needle)
            return &field;
      }
      return nullptr;
   }

   class ConsoleObjectProbeBase : public SimObject
   {
      typedef SimObject Parent;

   public:
      ConsoleObjectProbeBase() = default;

      S32 mBaseValue = 17;
      bool mBaseProtected = false;

      static void initPersistFields()
      {
         Parent::initPersistFields();
         addGroup("ProbeGroup", "Base console object probe fields.");
         addField("baseValue", TypeS32, Offset(mBaseValue, ConsoleObjectProbeBase), "Base integer value.");
         addProtectedField("baseProtected", TypeBool, Offset(mBaseProtected, ConsoleObjectProbeBase), nullptr, "Base protected flag.");
         endGroup("ProbeGroup");
      }

      DECLARE_CONOBJECT(ConsoleObjectProbeBase);
   };

   class ConsoleObjectProbeChild : public ConsoleObjectProbeBase
   {
      typedef ConsoleObjectProbeBase Parent;

   public:
      ConsoleObjectProbeChild() = default;

      S32 mChildValue = 29;

      static void initPersistFields()
      {
         Parent::initPersistFields();
         addGroup("ChildGroup", "Child console object probe fields.");
         addField("childValue", TypeS32, Offset(mChildValue, ConsoleObjectProbeChild), "Child integer value.");
         endGroup("ChildGroup");
      }

      DECLARE_CONOBJECT(ConsoleObjectProbeChild);
   };
}

IMPLEMENT_CONOBJECT(ConsoleObjectProbeBase);
IMPLEMENT_CONOBJECT(ConsoleObjectProbeChild);

TEST_CASE("ConsoleObject classes register with the VM", "[ConsoleObject]") {
   REQUIRE(sVM != nullptr);

   const KorkApi::ClassId simObjectId = sVM->getClassId("SimObject");
   const KorkApi::ClassId baseId = sVM->getClassId("ConsoleObjectProbeBase");
   const KorkApi::ClassId childId = sVM->getClassId("ConsoleObjectProbeChild");

   REQUIRE(simObjectId >= 0);
   REQUIRE(baseId >= 0);
   REQUIRE(childId >= 0);
   REQUIRE(std::string(sVM->getClassName(baseId)) == "ConsoleObjectProbeBase");
   REQUIRE(std::string(sVM->getClassName(childId)) == "ConsoleObjectProbeChild");

   const KorkApi::ClassInfo* baseInfo = sVM->mInternal->getClassInfoByName(StringTable->insert("ConsoleObjectProbeBase"));
   const KorkApi::ClassInfo* childInfo = sVM->mInternal->getClassInfoByName(StringTable->insert("ConsoleObjectProbeChild"));
   REQUIRE(baseInfo != nullptr);
   REQUIRE(childInfo != nullptr);

   // The embed API should mirror the registered class rep data exactly.
   REQUIRE(baseInfo->name == StringTable->insert("ConsoleObjectProbeBase"));
   REQUIRE(childInfo->name == StringTable->insert("ConsoleObjectProbeChild"));
   REQUIRE(baseInfo->userPtr == ConsoleObjectProbeBase::getStaticClassRep());
   REQUIRE(childInfo->userPtr == ConsoleObjectProbeChild::getStaticClassRep());
   REQUIRE(baseInfo->parentKlassId == simObjectId);
   REQUIRE(childInfo->parentKlassId == baseId);
   REQUIRE(baseInfo->nativeRootClassId == baseId);
   REQUIRE(childInfo->nativeRootClassId == childId);
   REQUIRE(baseInfo->scriptClass == nullptr);
   REQUIRE(childInfo->scriptClass == nullptr);
   REQUIRE(baseInfo->fields != nullptr);
   REQUIRE(childInfo->fields != nullptr);

   auto* baseObject = dynamic_cast<ConsoleObjectProbeBase*>(ConsoleObject::create("ConsoleObjectProbeBase"));
   auto* childObject = dynamic_cast<ConsoleObjectProbeChild*>(ConsoleObject::create("ConsoleObjectProbeChild"));
   REQUIRE(baseObject != nullptr);
   REQUIRE(childObject != nullptr);
   REQUIRE(baseInfo->numFields == baseObject->getFieldList().size());
   REQUIRE(childInfo->numFields == childObject->getFieldList().size());
   delete baseObject;
   delete childObject;

   REQUIRE(baseInfo->iCreate.CreateClassFn != nullptr);
   REQUIRE(baseInfo->iCreate.DestroyClassFn != nullptr);
   REQUIRE(baseInfo->iCreate.ProcessArgsFn != nullptr);
   REQUIRE(baseInfo->iCreate.AddObjectFn != nullptr);
   REQUIRE(baseInfo->iCreate.RemoveObjectFn != nullptr);
   REQUIRE(baseInfo->iCreate.GetIdFn != nullptr);
   REQUIRE(baseInfo->iCreate.GetNameFn != nullptr);
   REQUIRE(baseInfo->iCustomFields.IterateFields != nullptr);
   REQUIRE(baseInfo->iCustomFields.GetFieldByIterator != nullptr);
   REQUIRE(baseInfo->iCustomFields.GetFieldByName != nullptr);
   REQUIRE(baseInfo->iCustomFields.SetCustomFieldByName != nullptr);
   REQUIRE(baseInfo->iCustomFields.SetCustomFieldType != nullptr);
   REQUIRE(baseInfo->iEnum.GetSize != nullptr);
   REQUIRE(baseInfo->iEnum.GetObjectAtIndex != nullptr);
   REQUIRE(baseInfo->iSignals.TriggerSignal != nullptr);
}

TEST_CASE("ConsoleObject class metadata enumerates registered fields", "[ConsoleObject][Fields]") {
   REQUIRE(sVM != nullptr);

   const KorkApi::ClassId baseId = sVM->getClassId("ConsoleObjectProbeBase");
   const KorkApi::ClassId childId = sVM->getClassId("ConsoleObjectProbeChild");
   REQUIRE(baseId >= 0);
   REQUIRE(childId >= 0);

   std::vector<CapturedFieldInfo> baseFields;
   std::vector<CapturedFieldInfo> childFields;
   sVM->enumerateClassFields(baseId, 0, &baseFields, captureClassField);
   sVM->enumerateClassFields(childId, 0, &childFields, captureClassField);

   // The metadata should include the fields we registered in initPersistFields.
   const CapturedFieldInfo* baseValue = findField(baseFields, "baseValue");
   const CapturedFieldInfo* baseProtected = findField(baseFields, "baseProtected");
   const CapturedFieldInfo* childValue = findField(childFields, "childValue");
   REQUIRE(baseValue != nullptr);
   REQUIRE(baseProtected != nullptr);
   REQUIRE(childValue != nullptr);

   REQUIRE(baseValue->typeId == TypeS32);
   REQUIRE(baseValue->docs == "Base integer value.");
   REQUIRE(baseValue->isScriptField == false);

   REQUIRE(baseProtected->typeId == TypeBool);
   REQUIRE(baseProtected->docs == "Base protected flag.");
   REQUIRE(baseProtected->isScriptField == false);

   REQUIRE(childValue->typeId == TypeS32);
   REQUIRE(childValue->docs == "Child integer value.");
   REQUIRE(childValue->isScriptField == false);

   REQUIRE(findField(baseFields, "ProbeGroup_begingroup") == nullptr);
   REQUIRE(findField(baseFields, "ProbeGroup_endgroup") == nullptr);
   REQUIRE(findField(childFields, "ChildGroup_begingroup") == nullptr);
   REQUIRE(findField(childFields, "ChildGroup_endgroup") == nullptr);
}

TEST_CASE("ConsoleObject embed callbacks create, name, and field data round-trip", "[ConsoleObject][Embed]") {
   REQUIRE(sVM != nullptr);

   const KorkApi::ClassInfo* childInfo = sVM->mInternal->getClassInfoByName(StringTable->insert("ConsoleObjectProbeChild"));
   REQUIRE(childInfo != nullptr);

   // The registry should also be able to construct the class directly by name.
   auto* createdByName = dynamic_cast<ConsoleObjectProbeChild*>(ConsoleObject::create("ConsoleObjectProbeChild"));
   REQUIRE(createdByName != nullptr);
   REQUIRE(std::string(createdByName->getClassName()) == "ConsoleObjectProbeChild");
   REQUIRE(createdByName->findField(StringTable->insert("baseValue")) != nullptr);
   REQUIRE(createdByName->findField(StringTable->insert("childValue")) != nullptr);
   delete createdByName;

   // The create/destroy hooks should be able to construct and clean up an object directly.
   KorkApi::CreateClassReturn created = {};
   childInfo->iCreate.CreateClassFn(childInfo->userPtr, sVM, &created);
   REQUIRE(created.userPtr != nullptr);
   REQUIRE((created.initialFlags & KorkApi::ModStaticFields) != 0);
   REQUIRE((created.initialFlags & KorkApi::ModDynamicFields) != 0);
   REQUIRE(childInfo->iCreate.ProcessArgsFn(sVM, created.userPtr, nullptr, false, false, 0, nullptr));
   childInfo->iCreate.DestroyClassFn(childInfo->userPtr, sVM, created.userPtr);

   auto* object = new ConsoleObjectProbeChild;
   REQUIRE(object->registerObject(uniqueName("consoleObjectProbe").c_str()));
   object->setModDynamicFields(true);

   REQUIRE(childInfo->iCreate.GetIdFn(object->getVMObject()) == object->getId());
   REQUIRE(std::string(childInfo->iCreate.GetNameFn(object->getVMObject())) == object->getName());

   // Dynamic field access should flow through the embed metadata as well.
   const StringTableEntry dynField = StringTable->insert("embeddedDynamic");
   const KorkApi::ConsoleValue payload = KorkApi::ConsoleValue::makeString("probe-payload");
   REQUIRE(sVM->setObjectField(object->getVMObject(), dynField, payload, KorkApi::ConsoleValue()));
   const KorkApi::ConsoleValue readBack = sVM->getObjectField(object->getVMObject(), dynField, KorkApi::ConsoleValue());
   REQUIRE(std::string(sVM->valueAsString(readBack)) == "probe-payload");

   object->deleteObject();
}
