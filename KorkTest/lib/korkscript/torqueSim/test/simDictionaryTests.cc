//-----------------------------------------------------------------------------
// Copyright (c) 2026 korkscript contributors.
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "sim/simBase.h"
#include "sim/simDictionary.h"

#include <catch2/catch.hpp>

#include <atomic>
#include <string>
#include <utility>
#include <vector>

namespace
{
   static std::atomic<U32> gNextSimDictionaryTestId{200000};

   std::string uniqueName(const char* prefix)
   {
      return std::string(prefix) + "_" + std::to_string(gNextSimDictionaryTestId.fetch_add(1));
   }

   template <class T, class... Args>
   static T* createUnregisteredObjectWithId(const std::string& name, U32 id, Args&&... args)
   {
      T* obj = new T(std::forward<Args>(args)...);
      obj->setId(id);
      obj->assignName(name.c_str());
      return obj;
   }

   class DictionaryTestObject : public SimObject
   {
      typedef SimObject Parent;

   public:
      DictionaryTestObject() = default;

      DECLARE_CONOBJECT(DictionaryTestObject);
   };
}

IMPLEMENT_CONOBJECT(DictionaryTestObject);

TEST_CASE("SimNameDictionary inserts finds and removes named objects", "[SimDictionary][Name]") {
   SimNameDictionary dict;
   // Use unregistered objects here so the local dictionary does not disturb the live global Sim dictionaries.
   DictionaryTestObject* first = createUnregisteredObjectWithId<DictionaryTestObject>(uniqueName("namedA"), 210001);
   DictionaryTestObject* second = createUnregisteredObjectWithId<DictionaryTestObject>(uniqueName("namedB"), 210002);

   // Empty lookups should fail cleanly.
   REQUIRE(dict.find(nullptr) == nullptr);
   REQUIRE(dict.find(StringTable->insert("missing")) == nullptr);

   dict.insert(first);
   dict.insert(second);
   REQUIRE(dict.find(first->getName()) == first);
   REQUIRE(dict.find(second->getName()) == second);

   // Renaming changes the lookup key, so the old name stops resolving once the dictionary is rebuilt.
   const StringTableEntry oldFirstName = first->getName();
   dict.remove(first);
   first->assignName(uniqueName("namedA_renamed").c_str());
   dict.insert(first);
   REQUIRE(dict.find(oldFirstName) == nullptr);
   REQUIRE(dict.find(first->getName()) == first);

   dict.remove(second);
   REQUIRE(dict.find(second->getName()) == nullptr);

   delete first;
   delete second;
}

TEST_CASE("SimManagerNameDictionary tracks manager lookups independently", "[SimDictionary][ManagerName]") {
   SimManagerNameDictionary dict;
   // The manager-name dictionary uses the same linkage field as the engine's global name table.
   // Keep these objects unregistered so we only exercise the local container.
   DictionaryTestObject* first = createUnregisteredObjectWithId<DictionaryTestObject>(uniqueName("managerA"), 220001);
   DictionaryTestObject* second = createUnregisteredObjectWithId<DictionaryTestObject>(uniqueName("managerB"), 220002);

   REQUIRE(dict.find(nullptr) == nullptr);
   REQUIRE(dict.find(StringTable->insert("missing")) == nullptr);

   dict.insert(first);
   dict.insert(second);
   REQUIRE(dict.find(first->getName()) == first);
   REQUIRE(dict.find(second->getName()) == second);

   dict.remove(first);
   REQUIRE(dict.find(first->getName()) == nullptr);
   REQUIRE(dict.find(second->getName()) == second);

   dict.remove(second);
   REQUIRE(dict.find(second->getName()) == nullptr);

   delete first;
   delete second;
}

TEST_CASE("SimIdDictionary inserts finds and removes by id", "[SimDictionary][Id]") {
   SimIdDictionary dict;
   // The id dictionary reuses the same linkage field as the global Sim id table, so these must stay local-only too.
   DictionaryTestObject* first = createUnregisteredObjectWithId<DictionaryTestObject>(uniqueName("idA"), 230001);
   DictionaryTestObject* second = createUnregisteredObjectWithId<DictionaryTestObject>(uniqueName("idB"), 230002);
   DictionaryTestObject* colliding = createUnregisteredObjectWithId<DictionaryTestObject>(uniqueName("idCollision"), 230001 + 4096);

   // The table is masked by 4095, so these ids intentionally collide into the same bucket.
   dict.insert(first);
   dict.insert(second);
   dict.insert(colliding);

   REQUIRE(dict.find(first->getId()) == first);
   REQUIRE(dict.find(second->getId()) == second);
   REQUIRE(dict.find(colliding->getId()) == colliding);
   REQUIRE(dict.find(999999) == nullptr);

   dict.remove(second);
   REQUIRE(dict.find(second->getId()) == nullptr);
   REQUIRE(dict.find(first->getId()) == first);
   REQUIRE(dict.find(colliding->getId()) == colliding);

   dict.remove(first);
   REQUIRE(dict.find(first->getId()) == nullptr);
   REQUIRE(dict.find(colliding->getId()) == colliding);

   dict.remove(colliding);
   REQUIRE(dict.find(colliding->getId()) == nullptr);

   delete first;
   delete second;
   delete colliding;
}

TEST_CASE("SimNameDictionary and SimManagerNameDictionary resize and rehash cleanly", "[SimDictionary][Name][ManagerName]") {
   SimNameDictionary nameDict;
   SimManagerNameDictionary managerDict;
   std::vector<DictionaryTestObject*> objects;
   objects.reserve(40);

   // Insert more than the default bucket count so both dictionaries must rebuild their hash tables.
   for (U32 i = 0; i < 40; ++i)
   {
      auto* obj = createUnregisteredObjectWithId<DictionaryTestObject>(uniqueName("rehash"), 250000 + i);
      objects.push_back(obj);
      nameDict.insert(obj);
      managerDict.insert(obj);
   }

   for (DictionaryTestObject* obj : objects)
   {
      REQUIRE(nameDict.find(obj->getName()) == obj);
      REQUIRE(managerDict.find(obj->getName()) == obj);
   }

   for (DictionaryTestObject* obj : objects)
   {
      nameDict.remove(obj);
      managerDict.remove(obj);
   }

   for (DictionaryTestObject* obj : objects)
   {
      REQUIRE(nameDict.find(obj->getName()) == nullptr);
      REQUIRE(managerDict.find(obj->getName()) == nullptr);
      delete obj;
   }
}

TEST_CASE("SimDictionary handles null-name objects as no-ops", "[SimDictionary]") {
   SimNameDictionary nameDict;
   SimManagerNameDictionary managerDict;
   DictionaryTestObject* obj = new DictionaryTestObject();
   REQUIRE(obj->registerObject(240001));

   // Id-only registration leaves the name dictionaries with nothing to index.
   REQUIRE(obj->getName() == nullptr);

   nameDict.insert(obj);
   managerDict.insert(obj);
   REQUIRE(nameDict.find(nullptr) == nullptr);
   REQUIRE(managerDict.find(nullptr) == nullptr);

   obj->deleteObject();
}
